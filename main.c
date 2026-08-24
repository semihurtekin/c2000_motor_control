/**
 * @file    main.c
 * @brief   Continuous ADC DMA ping-pong buffer verification lab.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

#include "F2837xD_device.h"

#include "platform_clock.h"
#include "platform_interrupt.h"

#include "mcal_adc.h"
#include "mcal_cpu_int.h"
#include "mcal_dma.h"
#include "mcal_epwm.h"
#include "mcal_pie.h"
#include "mcal_timer.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define DMA_BUFFER_SIZE        (64U)
#define DMA_CH1_PIE_CHANNEL    (1U)

/*==============================================================================
 * Private Types
 *============================================================================*/

typedef enum
{
    DMA_LAB_BUFFER_PING = 0U,
    DMA_LAB_BUFFER_PONG = 1U
} DmaLabBufferType;

/*==============================================================================
 * Private Variables
 *============================================================================*/

#pragma DATA_SECTION(DmaPingBuffer, "ramgs0")
static volatile uint16_t DmaPingBuffer[DMA_BUFFER_SIZE];

#pragma DATA_SECTION(DmaPongBuffer, "ramgs0")
static volatile uint16_t DmaPongBuffer[DMA_BUFFER_SIZE];

static volatile uint16_t PingReady;
static volatile uint16_t PongReady;
static volatile uint16_t SoftwareBufferOverrun;
static volatile uint32_t DmaIrqCount;

static volatile DmaLabBufferType DmaExpectedBuffer;
static volatile uint16_t FirstTransferSeen;

static uint16_t AdcStartupDone;
static uint16_t DmaHwOverrun;

static uint16_t PingFirstSample;
static uint16_t PingLastSample;
static uint16_t PongFirstSample;
static uint16_t PongLastSample;

static uint32_t PingProcessCount;
static uint32_t PongProcessCount;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

__interrupt void DmaCh1Isr(void);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

int main(void)
{
    Mcal_EpwmTbConfigType epwmTbConfig;
    Mcal_EpwmAdcTrigConfigType adcTrigConfig;
    Mcal_AdcSocConfigType adcSocConfig;
    Mcal_AdcIntConfigType adcIntConfig;
    Mcal_DmaChannelConfigType dmaConfig;
    Mcal_TimerConfigType timerConfig;

    PingReady = 0U;
    PongReady = 0U;
    SoftwareBufferOverrun = 0U;
    DmaIrqCount = 0UL;

    DmaExpectedBuffer = DMA_LAB_BUFFER_PING;
    FirstTransferSeen = 0U;

    AdcStartupDone = 0U;
    DmaHwOverrun = 0U;

    PingFirstSample = 0U;
    PingLastSample = 0U;
    PongFirstSample = 0U;
    PongLastSample = 0U;

    PingProcessCount = 0UL;
    PongProcessCount = 0UL;

    (void)Platform_ClockInit();

    /*
     * Configure the CPU interrupt infrastructure before the DMA event source
     * is released.
     */
    (void)Mcal_CpuInt_Init();
    (void)Mcal_Pie_Init();

    /*
     * Platform vector-table boundary:
     * DMA Channel 1 -> DmaCh1Isr.
     */
    (void)Platform_IntSetDmaCh1(&DmaCh1Isr);

    /*
     * Keep the ePWM time-base stopped until ADC, DMA and CPU notification
     * paths are fully configured.
     */
    EALLOW;

    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1U;
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1U;
    CpuSysRegs.PCLKCR0.bit.DMA = 1U;

    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1U;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0U;

    EDIS;

    /*--------------------------------------------------------------------------
     * ADC configuration
     *------------------------------------------------------------------------*/

    (void)Mcal_Adc_Init(MCAL_ADC_A);

    adcSocConfig.adc = MCAL_ADC_A;
    adcSocConfig.soc = MCAL_ADC_SOC_0;
    adcSocConfig.channel = MCAL_ADC_CHANNEL_0;
    adcSocConfig.trigger = MCAL_ADC_TRIG_EPWM1_SOCA;
    adcSocConfig.acquisitionCycles = 20U;

    (void)Mcal_Adc_InitSoc(&adcSocConfig);

    /*
     * EOC0 generates ADCAINT1. The ADC interrupt output is used as the DMA
     * hardware trigger; the ADCA1 PIE interrupt itself is not enabled.
     */
    adcIntConfig.adc = MCAL_ADC_A;
    adcIntConfig.adcInt = MCAL_ADC_INT_1;
    adcIntConfig.sourceEoc = MCAL_ADC_SOC_0;

    (void)Mcal_Adc_EnableInterrupt(&adcIntConfig);

    /*--------------------------------------------------------------------------
     * ePWM sampling time-base
     *------------------------------------------------------------------------*/

    epwmTbConfig.module = MCAL_EPWM_1;
    epwmTbConfig.period = 5000U;
    epwmTbConfig.mode = MCAL_EPWM_COUNT_UP_DOWN;
    epwmTbConfig.clkDiv = MCAL_EPWM_CLKDIV_1;
    epwmTbConfig.hsClkDiv = MCAL_EPWM_HSCLKDIV_1;

    (void)Mcal_Epwm_InitTimeBase(&epwmTbConfig);

    adcTrigConfig.module = MCAL_EPWM_1;
    adcTrigConfig.soc = MCAL_EPWM_ADC_SOCA;
    adcTrigConfig.source = MCAL_EPWM_ADC_TRIG_ZERO;
    adcTrigConfig.eventPrescale = 1U;

    (void)Mcal_Epwm_InitAdcTrigger(&adcTrigConfig);

    /*--------------------------------------------------------------------------
     * DMA ping-pong configuration
     *------------------------------------------------------------------------*/

    (void)Mcal_Dma_Init();

    dmaConfig.channel = MCAL_DMA_CHANNEL_1;
    dmaConfig.trigger = MCAL_DMA_TRIG_ADCA1;

    dmaConfig.sourceAddress =
        &AdcaResultRegs.ADCRESULT0;

    /*
     * Ping is the destination of the first transfer. The DMA ISR changes the
     * destination shadow pointer to Pong for the second transfer, then
     * alternates Ping/Pong on every following transfer.
     */
    dmaConfig.destinationAddress =
        &DmaPingBuffer[0];

    dmaConfig.burstWords = 1U;
    dmaConfig.sourceBurstStep = 0;
    dmaConfig.destinationBurstStep = 0;

    dmaConfig.transferBursts = DMA_BUFFER_SIZE;
    dmaConfig.sourceTransferStep = 0;
    dmaConfig.destinationTransferStep = 1;

    dmaConfig.oneShot =
        MCAL_DMA_ONESHOT_DISABLE;

    dmaConfig.continuous =
        MCAL_DMA_CONT_ENABLE;

    (void)Mcal_Dma_InitChannel(&dmaConfig);

    /*
     * Beginning-of-transfer interrupt is intentional. At ISR entry the DMA
     * has already loaded the active address for the new transfer, therefore
     * software can safely program the shadow address for the following
     * transfer without modifying the buffer currently being filled.
     */
    (void)Mcal_Dma_EnableInterrupt(
        MCAL_DMA_CHANNEL_1,
        MCAL_DMA_INT_AT_BEGINNING);

    /*--------------------------------------------------------------------------
     * ADC power-up settling time
     *------------------------------------------------------------------------*/

    timerConfig.timer = MCAL_TIMER_0;
    timerConfig.period = 1UL;
    timerConfig.prescaler = 65535U;

    (void)Mcal_Timer_Init(&timerConfig);
    (void)Mcal_Timer_Start(MCAL_TIMER_0);

    while(AdcStartupDone == 0U)
    {
        (void)Mcal_Timer_IsElapsed(
            MCAL_TIMER_0,
            &AdcStartupDone);
    }

    (void)Mcal_Timer_Stop(MCAL_TIMER_0);
    (void)Mcal_Timer_ClearFlag(MCAL_TIMER_0);

    /*
     * DMA CH1 interrupt -> PIE Group 7 / Channel 1 -> CPU INT7.
     */
    (void)Mcal_Pie_Enable(
        MCAL_PIE_GROUP_7,
        DMA_CH1_PIE_CHANNEL);

    (void)Mcal_CpuInt_Enable(MCAL_CPU_INT_7);

    /*
     * Arm the consumer before releasing the sampling source.
     */
    (void)Mcal_Dma_Start(MCAL_DMA_CHANNEL_1);

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
    EDIS;

    Mcal_CpuInt_EnableGlobal();

    for(;;)
    {
        /*
         * This is deliberately tiny "processing" for the lab. A production
         * consumer must finish using a completed buffer before DMA returns to
         * that buffer one full ping-pong period later.
         */
        if(PingReady != 0U)
        {
            PingFirstSample = DmaPingBuffer[0];
            PingLastSample =
                DmaPingBuffer[DMA_BUFFER_SIZE - 1U];

            PingProcessCount++;
            PingReady = 0U;
        }
        else
        {
            /* Do nothing. */
        }

        if(PongReady != 0U)
        {
            PongFirstSample = DmaPongBuffer[0];
            PongLastSample =
                DmaPongBuffer[DMA_BUFFER_SIZE - 1U];

            PongProcessCount++;
            PongReady = 0U;
        }
        else
        {
            /* Do nothing. */
        }

        /*
         * Hardware trigger overrun is independent from the software buffer
         * ownership overrun detected in DmaCh1Isr().
         */
        (void)Mcal_Dma_IsOverrun(
            MCAL_DMA_CHANNEL_1,
            &DmaHwOverrun);
    }
}

/*==============================================================================
 * Interrupt Service Routines
 *============================================================================*/

__interrupt void DmaCh1Isr(void)
{
    DmaIrqCount++;

    /*
     * DmaExpectedBuffer identifies the buffer that has just become active at
     * this beginning-of-transfer interrupt.
     */
    if(DmaExpectedBuffer == DMA_LAB_BUFFER_PING)
    {
        /*
         * If Ping was still marked ready when DMA returned to it, the CPU did
         * not consume the previous Ping contents before they began to be
         * overwritten.
         */
        if(PingReady != 0U)
        {
            SoftwareBufferOverrun = 1U;
            PingReady = 0U;
        }
        else
        {
            /* Do nothing. */
        }

        /*
         * After the first transfer has started, a new Ping transfer implies
         * that the previous Pong transfer is now complete.
         */
        if(FirstTransferSeen != 0U)
        {
            PongReady = 1U;
        }
        else
        {
            FirstTransferSeen = 1U;
        }

        /*
         * Ping is active now. Program Pong as the destination of the next
         * transfer. The current active destination is not modified.
         */
        (void)Mcal_Dma_SetDstStartAddress(
            MCAL_DMA_CHANNEL_1,
            &DmaPongBuffer[0]);

        DmaExpectedBuffer = DMA_LAB_BUFFER_PONG;
    }
    else
    {
        /*
         * Pong has just become active, therefore the previous Ping transfer
         * is complete.
         */
        if(PongReady != 0U)
        {
            SoftwareBufferOverrun = 1U;
            PongReady = 0U;
        }
        else
        {
            /* Do nothing. */
        }

        PingReady = 1U;

        /*
         * Pong is active now. Program Ping for the following transfer.
         */
        (void)Mcal_Dma_SetDstStartAddress(
            MCAL_DMA_CHANNEL_1,
            &DmaPingBuffer[0]);

        DmaExpectedBuffer = DMA_LAB_BUFFER_PING;
    }

    /*
     * The DMA channel interrupt itself has no separate software-clear flag in
     * this configuration. Releasing PIE Group 7 allows the next DMA interrupt
     * to propagate to CPU1.
     */
    (void)Mcal_Pie_Ack(MCAL_PIE_GROUP_7);
}
