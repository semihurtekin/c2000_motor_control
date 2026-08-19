/**
 * @file    main.c
 * @brief   CPU Timer0 interrupt verification lab.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "main.h"

#include "platform_clock.h"
#include "platform_interrupt.h"

#include "mcal_cpu_int.h"
#include "mcal_gpio.h"
#include "mcal_pie.h"
#include "mcal_timer.h"
#include "mcal_epwm.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/


/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

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
    
    // GPIO0 - PIN 50 Init
    gpioConfig.initLevel = MCAL_GPIO_LEVEL_HIGH;
    gpioConfig.dir = MCAL_GPIO_DIR_OUTPUT;
    gpioConfig.inv = MCAL_GPIO_INV_DISABLE;
    gpioConfig.odr = MCAL_GPIO_ODR_DISABLE;
    gpioConfig.owner = MCAL_GPIO_OWNER_CPU1;
    gpioConfig.pull = MCAL_GPIO_PULL_DISABLE;
    gpioConfig.qual = MCAL_GPIO_QUAL_SYNC;
    gpioConfig.pin = 0U;
    Mcal_Gpio_InitPin(&gpioConfig);
    Mcal_Gpio_SetMux(0U, 0x1U);

    // GPIO1 - PIN 49 Init
    gpioConfig.initLevel = MCAL_GPIO_LEVEL_HIGH;
    gpioConfig.dir = MCAL_GPIO_DIR_OUTPUT;
    gpioConfig.inv = MCAL_GPIO_INV_DISABLE;
    gpioConfig.odr = MCAL_GPIO_ODR_DISABLE;
    gpioConfig.owner = MCAL_GPIO_OWNER_CPU1;
    gpioConfig.pull = MCAL_GPIO_PULL_DISABLE;
    gpioConfig.qual = MCAL_GPIO_QUAL_SYNC;
    gpioConfig.pin = 1U;
    Mcal_Gpio_InitPin(&gpioConfig);
    Mcal_Gpio_SetMux(1U, 0x1U);

    epwmTbConfig.module = MCAL_EPWM_1;
    epwmTbConfig.clkDiv = MCAL_EPWM_CLKDIV_1;
    epwmTbConfig.hsClkDiv = MCAL_EPWM_HSCLKDIV_1;
    epwmTbConfig.mode = MCAL_EPWM_COUNT_UP_DOWN;
    epwmTbConfig.period = 5000U;
    Mcal_Epwm_InitTimeBase(&epwmTbConfig);

    epwmCompConfig.module = MCAL_EPWM_1;
    epwmCompConfig.compareA = 1000U;
    Mcal_Epwm_InitCompareA(&epwmCompConfig);

    epwmDbConfig.module = MCAL_EPWM_1;
    epwmDbConfig.fallingDelay = 100U;
    epwmDbConfig.risingDelay = 100U;
    Mcal_Epwm_InitDeadBand(&epwmDbConfig);

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
    EDIS;

    for(;;)
    {
        /* Idle. */
    }
}

/*==============================================================================
 * Interrupt Service Routines
 *============================================================================*/

