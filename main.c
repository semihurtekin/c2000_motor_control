/**
 * @file    main.c
 * @brief   ePWM-triggered ADCA1 interrupt verification lab.
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
#include "mcal_epwm.h"
#include "mcal_gpio.h"
#include "mcal_pie.h"
#include "mcal_timer.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define ADCA1_PIE_CHANNEL      (1U)
#define ADC_DEBUG_GPIO         (2U)

/*==============================================================================
 * Private Variables
 *============================================================================*/

static uint16_t AdcRaw0;
static uint16_t AdcRaw1;
static uint16_t AdcStartupDone;
static uint16_t AdcIntOverflow;
static uint32_t AdcIsrCount;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

__interrupt void ADCB_ISR(void);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

int main(void)
{
    Mcal_AdcSocConfigType adcSocConfig;
    Mcal_AdcIntConfigType adcIntConfig;
    Mcal_EpwmTbConfigType epwmTbConfig;
    Mcal_EpwmAdcTrigConfigType adcTrigConfig;
    Mcal_GpioConfigType gpioConfig;
    Mcal_TimerConfigType timerConfig;

    AdcRaw0 = 0U;
    AdcRaw1 = 0U;
    AdcStartupDone = 0U;
    AdcIntOverflow = 0U;
    AdcIsrCount = 0UL;

    (void)Platform_ClockInit();

    /*
     * Interrupt routing is configured while global CPU interrupts are closed.
     */
    (void)Mcal_CpuInt_Init();
    (void)Mcal_Pie_Init();

    // /*
    //  * Platform vector-table boundary:
    //  * ADCA1 -> Adca1Isr.
    //  *
    //  * Add Platform_IntSetAdca1() next to the existing
    //  * Platform_IntSetTimer0() implementation.
    //  */
    // (void)Platform_IntSetAdca1(&Adca1Isr);
    (void)Platform_IntSetAdcb1(&ADCB_ISR);

    EALLOW;

    CpuSysRegs.PCLKCR13.bit.ADC_A = 1U;
    CpuSysRegs.PCLKCR13.bit.ADC_B = 1U;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1U;

    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1U;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0U;

    EDIS;

    /*
     * GPIO0 is used only as an ISR timing marker for the logic analyzer.
     * It remains GPIO, not EPWM1A, in this lab.
     */
    gpioConfig.pin = ADC_DEBUG_GPIO;
    gpioConfig.dir = MCAL_GPIO_DIR_OUTPUT;
    gpioConfig.pull = MCAL_GPIO_PULL_DISABLE;
    gpioConfig.odr = MCAL_GPIO_ODR_DISABLE;
    gpioConfig.inv = MCAL_GPIO_INV_DISABLE;
    gpioConfig.qual = MCAL_GPIO_QUAL_SYNC;
    gpioConfig.owner = MCAL_GPIO_OWNER_CPU1;
    gpioConfig.initLevel = MCAL_GPIO_LEVEL_LOW;

    (void)Mcal_Gpio_InitPin(&gpioConfig);

    (void)Mcal_Adc_Init(MCAL_ADC_A);
    (void)Mcal_Adc_Init(MCAL_ADC_B);

    adcSocConfig.adc = MCAL_ADC_A;
    adcSocConfig.soc = MCAL_ADC_SOC_0;
    adcSocConfig.channel = MCAL_ADC_CHANNEL_0;
    adcSocConfig.trigger = MCAL_ADC_TRIG_EPWM1_SOCA;
    adcSocConfig.acquisitionCycles = 20U;

    (void)Mcal_Adc_InitSoc(&adcSocConfig);

    adcSocConfig.adc = MCAL_ADC_B;
    adcSocConfig.soc = MCAL_ADC_SOC_0;
    adcSocConfig.channel = MCAL_ADC_CHANNEL_3;
    
    (void)Mcal_Adc_InitSoc(&adcSocConfig);

    Mcal_Adc_SetSocPriority(MCAL_ADC_A, 2U);

    /*
     * EOC0 -> ADCA ADCINT1.
     *
     * ADC v0.2 enables continuous interrupt mode internally as the F2837xD
     * silicon-erratum workaround. Overflow is monitored in the ISR.
     */
    adcIntConfig.adc = MCAL_ADC_B;
    adcIntConfig.adcInt = MCAL_ADC_INT_1;
    adcIntConfig.sourceEoc = MCAL_ADC_SOC_0;

    (void)Mcal_Adc_EnableInterrupt(&adcIntConfig);

    /*
     * ePWM1 is used only as the ADC sampling time base in this lab.
     *
     * 100 MHz TBCLK / (2 * 5000) = 10 kHz.
     */
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

    /*
     * Wait >500 us after ADC power-up.
     */
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
     * ADCA1 -> PIE Group1 / Channel1 -> CPU INT1.
     */
    (void)Mcal_Pie_Enable(
        MCAL_PIE_GROUP_1,
        2U);

    (void)Mcal_CpuInt_Enable(MCAL_CPU_INT_1);

    /*
     * Open the ADC sampling source only after the full interrupt path
     * has been configured.
     */
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
    EDIS;

    Mcal_CpuInt_EnableGlobal();

    for(;;)
    {
        /*
         * Main loop intentionally idle.
         * AdcRaw and AdcIsrCount are updated by Adca1Isr().
         */
    }
}

/*==============================================================================
 * Interrupt Service Routines
 *============================================================================*/

__interrupt void ADCB_ISR(void)
{
    (void)Mcal_Gpio_Write(
        ADC_DEBUG_GPIO,
        MCAL_GPIO_LEVEL_HIGH);
        
    (void)Mcal_Adc_GetResult(
        MCAL_ADC_A,
        MCAL_ADC_SOC_0,
        &AdcRaw0);

    (void)Mcal_Adc_GetResult(
        MCAL_ADC_B,
        MCAL_ADC_SOC_0,
        &AdcRaw1);

    (void)Mcal_Adc_ClearIntFlag(
        MCAL_ADC_B,
        MCAL_ADC_INT_1);

    /*
     * F2837xD ADC interrupt erratum handling:
     *
     * An overflow means another ADC interrupt event arrived before software
     * completed servicing the previous event. After clearing the normal flag,
     * check overflow immediately. If it is set, clear the normal flag again
     * and then clear the overflow indication.
     */
    (void)Mcal_Adc_IsIntOverflow(
        MCAL_ADC_B,
        MCAL_ADC_INT_1,
        &AdcIntOverflow);

    if(AdcIntOverflow != 0U)
    {
        (void)Mcal_Adc_ClearIntFlag(
            MCAL_ADC_B,
            MCAL_ADC_INT_1);

        (void)Mcal_Adc_ClearIntOverflow(
            MCAL_ADC_B,
            MCAL_ADC_INT_1);
    }
    else
    {
        /* Do nothing. */
    }

    (void)Mcal_Pie_Ack(MCAL_PIE_GROUP_1);

    (void)Mcal_Gpio_Write(
        ADC_DEBUG_GPIO,
        MCAL_GPIO_LEVEL_LOW);
}
