#include "mcal_timer.h"
#include "mcal_gpio.h"

int main(void)
{
    Mcal_GpioConfigType gpioConfig;

    gpioConfig.pin = 31U;
    gpioConfig.dir = MCAL_GPIO_DIR_OUTPUT;
    gpioConfig.initLevel = MCAL_GPIO_LEVEL_LOW;
    gpioConfig.inv = MCAL_GPIO_INV_DISABLE;
    gpioConfig.odr = MCAL_GPIO_ODR_DISABLE;
    gpioConfig.owner = MCAL_GPIO_OWNER_CPU1;
    gpioConfig.pull = MCAL_GPIO_PULL_DISABLE;
    gpioConfig.qual = MCAL_GPIO_QUAL_SYNC;

    (void)Mcal_Gpio_InitPin(&gpioConfig);

    Mcal_TimerConfigType timerConfig;

    timerConfig.timer = MCAL_TIMER_0;
    timerConfig.period = 999U;
    timerConfig.prescaler = 65535U;

    (void)Mcal_Timer_Init(&timerConfig);
    (void)Mcal_Timer_Start(MCAL_TIMER_0);

    uint16_t elapsed;

    for (;;)
    {
        elapsed = 0U;

        (void)Mcal_Timer_IsElapsed(MCAL_TIMER_0, &elapsed);

        if (elapsed != 0U)
        {
            (void)Mcal_Timer_ClearFlag(MCAL_TIMER_0);

            (void)Mcal_Gpio_Toggle(31U);
        }
    }
}
