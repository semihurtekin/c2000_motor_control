/**
 * @file    main.c
 * @brief   ePWM software-forced one-shot Trip Zone lab.
 */

#include <stdint.h>

#include "F2837xD_device.h"
#include "platform_clock.h"
#include "mcal_gpio.h"
#include "mcal_epwm.h"
#include "mcal_timer.h"

static uint16_t TripState;
uint16_t timerElapsed;
int main(void)
{
    Mcal_GpioConfigType gpioConfig;
    Mcal_EpwmTbConfigType epwmTbConfig;
    Mcal_EpwmCompareConfigType epwmCompConfig;
    Mcal_EpwmDeadBandConfigType epwmDbConfig;
    Mcal_TimerConfigType timerConfig;

    TripState = 0U;

    Platform_ClockInit();

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0U;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1U;
    EDIS;

    /* GPIO0 -> EPWM1A */
    gpioConfig.pin = 0U;
    gpioConfig.dir = MCAL_GPIO_DIR_OUTPUT;
    gpioConfig.initLevel = MCAL_GPIO_LEVEL_LOW;
    gpioConfig.pull = MCAL_GPIO_PULL_DISABLE;
    gpioConfig.odr = MCAL_GPIO_ODR_DISABLE;
    gpioConfig.inv = MCAL_GPIO_INV_DISABLE;
    gpioConfig.qual = MCAL_GPIO_QUAL_SYNC;
    gpioConfig.owner = MCAL_GPIO_OWNER_CPU1;

    (void)Mcal_Gpio_InitPin(&gpioConfig);
    (void)Mcal_Gpio_SetMux(0U, 0x1U);

    /* GPIO1 -> EPWM1B */
    gpioConfig.pin = 1U;

    (void)Mcal_Gpio_InitPin(&gpioConfig);
    (void)Mcal_Gpio_SetMux(1U, 0x1U);

    epwmTbConfig.module = MCAL_EPWM_1;
    epwmTbConfig.period = 5000U;
    epwmTbConfig.mode = MCAL_EPWM_COUNT_UP_DOWN;
    epwmTbConfig.clkDiv = MCAL_EPWM_CLKDIV_1;
    epwmTbConfig.hsClkDiv = MCAL_EPWM_HSCLKDIV_1;

    (void)Mcal_Epwm_InitTimeBase(&epwmTbConfig);

    epwmCompConfig.module = MCAL_EPWM_1;
    epwmCompConfig.compareA = 1000U;

    (void)Mcal_Epwm_InitCompareA(&epwmCompConfig);

    epwmDbConfig.module = MCAL_EPWM_1;
    epwmDbConfig.risingDelay = 100U;
    epwmDbConfig.fallingDelay = 100U;

    (void)Mcal_Epwm_InitDeadBand(&epwmDbConfig);
    (void)Mcal_Epwm_InitTrip(MCAL_EPWM_1);

    /*
     * Example polling timer.
     * Adjust these fields to your existing Mcal_TimerConfigType definition.
     * Goal: generate a slow event that is easy to see on the logic analyzer.
     */
    timerConfig.timer = MCAL_TIMER_0;
    timerConfig.period = 6102U;
    timerConfig.prescaler = 65535U;

    (void)Mcal_Timer_Init(&timerConfig);
    (void)Mcal_Timer_Start(MCAL_TIMER_0);

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
    EDIS;

    for(;;)
    {
        
        Mcal_Timer_IsElapsed(MCAL_TIMER_0, &timerElapsed);
        
        if(timerElapsed != 0U)
        {
            (void)Mcal_Timer_ClearFlag(MCAL_TIMER_0);
            timerElapsed = 0;
            if(TripState == 0U)
            {
                (void)Mcal_Epwm_ForceTrip(MCAL_EPWM_1);
                TripState = 1U;
            }
            else
            {
                (void)Mcal_Epwm_ClearTrip(MCAL_EPWM_1);
                TripState = 0U;
            }
        }
        else
        {
            /* Do nothing. */
        }
    }
}
