/**
 * @file    main.c
 * @brief   ePWM SOCA event-trigger lab.
 *
 * The lab verifies the following hardware timing path without configuring
 * the ADC yet:
 *
 *     ePWM1 TBCTR = ZERO
 *              |
 *              +--> SOCA event
 *
 * The SOCA event flag is polled and cleared. CPU Timer0 provides an
 * approximately one-second observation window. For a 10 kHz ePWM and
 * eventPrescale = 1U, SocaEventsPerSecond should be close to 10000.
 */

#include <stdint.h>

#include "F2837xD_device.h"
#include "platform_clock.h"
#include "mcal_gpio.h"
#include "mcal_epwm.h"
#include "mcal_timer.h"

/*==============================================================================
 * Private Variables
 *============================================================================*/

static uint32_t SocaEventCount;
static uint32_t SocaEventsPerSecond;
static uint16_t SocaFlag;
static uint16_t TimerElapsed;

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

int main(void)
{
    Mcal_GpioConfigType gpioConfig;
    Mcal_EpwmTbConfigType epwmTbConfig;
    Mcal_EpwmCompareConfigType epwmCmpConfig;
    Mcal_EpwmDeadBandConfigType epwmDbConfig;
    Mcal_EpwmAdcTrigConfigType adcTrigConfig;
    Mcal_TimerConfigType timerConfig;

    SocaEventCount = 0UL;
    SocaEventsPerSecond = 0UL;
    SocaFlag = 0U;
    TimerElapsed = 0U;

    (void)Platform_ClockInit();

    EALLOW;

    /*
     * SYSCLK = 200 MHz.
     * EPWMCLKDIV = /2 gives EPWMCLK = 100 MHz.
     */
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1U;

    /*
     * Stop all ePWM time-base counters while ePWM1 is configured.
     */
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0U;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1U;

    EDIS;

    /* GPIO0 -> EPWM1A. */
    gpioConfig.pin = 0U;
    gpioConfig.dir = MCAL_GPIO_DIR_OUTPUT;
    gpioConfig.pull = MCAL_GPIO_PULL_DISABLE;
    gpioConfig.odr = MCAL_GPIO_ODR_DISABLE;
    gpioConfig.inv = MCAL_GPIO_INV_DISABLE;
    gpioConfig.qual = MCAL_GPIO_QUAL_SYNC;
    gpioConfig.owner = MCAL_GPIO_OWNER_CPU1;
    gpioConfig.initLevel = MCAL_GPIO_LEVEL_LOW;

    (void)Mcal_Gpio_InitPin(&gpioConfig);
    (void)Mcal_Gpio_SetMux(0U, 1U);

    /* GPIO1 -> EPWM1B. */
    gpioConfig.pin = 1U;

    (void)Mcal_Gpio_InitPin(&gpioConfig);
    (void)Mcal_Gpio_SetMux(1U, 1U);

    /*
     * EPWMCLK = 100 MHz
     * TBCLK   = 100 MHz
     * Up-down mode:
     *
     * fPWM = TBCLK / (2 * TBPRD)
     *      = 100 MHz / (2 * 5000)
     *      = 10 kHz
     */
    epwmTbConfig.module = MCAL_EPWM_1;
    epwmTbConfig.period = 5000U;
    epwmTbConfig.mode = MCAL_EPWM_COUNT_UP_DOWN;
    epwmTbConfig.clkDiv = MCAL_EPWM_CLKDIV_1;
    epwmTbConfig.hsClkDiv = MCAL_EPWM_HSCLKDIV_1;

    (void)Mcal_Epwm_InitTimeBase(&epwmTbConfig);

    /* 50 percent duty with the current CAU-clear / CAD-set convention. */
    epwmCmpConfig.module = MCAL_EPWM_1;
    epwmCmpConfig.compareA = 2500U;

    (void)Mcal_Epwm_InitCompareA(&epwmCmpConfig);

    /* Approximately 1 us rising/falling dead time at TBCLK = 100 MHz. */
    epwmDbConfig.module = MCAL_EPWM_1;
    epwmDbConfig.risingDelay = 100U;
    epwmDbConfig.fallingDelay = 100U;

    (void)Mcal_Epwm_InitDeadBand(&epwmDbConfig);

    /*
     * Generate ePWM1 SOCA every time TBCTR reaches ZERO.
     * In up-down mode ZERO occurs once per complete PWM period.
     * Therefore a 10 kHz PWM produces a 10 kHz SOCA stream.
     */
    adcTrigConfig.module = MCAL_EPWM_1;
    adcTrigConfig.soc = MCAL_EPWM_ADC_SOCA;
    adcTrigConfig.source = MCAL_EPWM_ADC_TRIG_ZERO;
    adcTrigConfig.eventPrescale = 1U;

    (void)Mcal_Epwm_InitAdcTrigger(&adcTrigConfig);

    /*
     * CPU Timer0 observation window:
     *
     * SYSCLK = 200 MHz
     * prescaler = 65535 -> divide by 65536
     * period = 3051 gives approximately 1.00008 s.
     */
    timerConfig.timer = MCAL_TIMER_0;
    timerConfig.period = 3051UL;
    timerConfig.prescaler = 65535U;

    (void)Mcal_Timer_Init(&timerConfig);
    (void)Mcal_Timer_Start(MCAL_TIMER_0);

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
    EDIS;

    for(;;)
    {
        (void)Mcal_Epwm_IsAdcTrigFlagSet(
            MCAL_EPWM_1,
            MCAL_EPWM_ADC_SOCA,
            &SocaFlag);

        if(SocaFlag != 0U)
        {
            (void)Mcal_Epwm_ClearAdcTrigFlag(
                MCAL_EPWM_1,
                MCAL_EPWM_ADC_SOCA);

            SocaEventCount++;
        }
        else
        {
            /* Do nothing. */
        }

        (void)Mcal_Timer_IsElapsed(
            MCAL_TIMER_0,
            &TimerElapsed);

        if(TimerElapsed != 0U)
        {
            (void)Mcal_Timer_ClearFlag(MCAL_TIMER_0);
            (void)Mcal_Timer_Reload(MCAL_TIMER_0);

            SocaEventsPerSecond = SocaEventCount;
            SocaEventCount = 0UL;
        }
        else
        {
            /* Do nothing. */
        }
    }
}
