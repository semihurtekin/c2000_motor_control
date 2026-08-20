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
#include "mcal_xbar.h"




int main(void)
{
    Mcal_GpioConfigType gpioConfig;
    Mcal_EpwmTbConfigType epwmTbConfig;
    Mcal_EpwmCompareConfigType epwmCompConfig;
    Mcal_EpwmDeadBandConfigType epwmDbConfig;

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

    gpioConfig.pin = 2U;
    gpioConfig.dir = MCAL_GPIO_DIR_INPUT;
    gpioConfig.pull = MCAL_GPIO_PULL_ENABLE;
    gpioConfig.qual = MCAL_GPIO_QUAL_ASYNC;

    (void)Mcal_Gpio_InitPin(&gpioConfig);
    (void)Mcal_Gpio_SetMux(2U, 0x0U);
    (void)Mcal_Xbar_SetInput(MCAL_XBAR_INPUT_1, 2U);

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
    (void)Mcal_Epwm_EnableOneShotTrip(MCAL_EPWM_1, MCAL_EPWM_TRIP_SOURCE_TZ1);

    // /*
    //  * Example polling timer.
    //  * Adjust these fields to your existing Mcal_TimerConfigType definition.
    //  * Goal: generate a slow event that is easy to see on the logic analyzer.
    //  */
    // timerConfig.timer = MCAL_TIMER_0;
    // timerConfig.period = 6102U;
    // timerConfig.prescaler = 65535U;

    // (void)Mcal_Timer_Init(&timerConfig);
    // (void)Mcal_Timer_Start(MCAL_TIMER_0);

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
    EDIS;

    for(;;)
    {
        
    }
}
