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

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define TIMER0_PIE_CHANNEL    (7U)

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

int main(void)
{
    Mcal_GpioConfigType gpioConfig;
    Mcal_TimerConfigType timerConfig;

    /*
     * Configure the target clock first. CPU interrupts remain disabled during
     * the complete interrupt routing configuration.
     */
    (void)Platform_ClockInit();

    (void)Mcal_CpuInt_Init();
    (void)Mcal_Pie_Init();

    /*
     * Bind the CPU Timer0 PIE vector before enabling any interrupt source.
     */
    (void)Platform_IntSetTimer0(&Timer0Isr);

    /*
     * GPIO31 drives the LaunchPad blue LED.
     * The LED is active-low, therefore HIGH keeps it initially off.
     */
    gpioConfig.pin = 0U;
    gpioConfig.dir = MCAL_GPIO_DIR_OUTPUT;
    gpioConfig.pull = MCAL_GPIO_PULL_DISABLE;
    gpioConfig.odr = MCAL_GPIO_ODR_DISABLE;
    gpioConfig.inv = MCAL_GPIO_INV_DISABLE;
    gpioConfig.qual = MCAL_GPIO_QUAL_SYNC;
    gpioConfig.owner = MCAL_GPIO_OWNER_CPU1;
    gpioConfig.initLevel = MCAL_GPIO_LEVEL_HIGH;

    (void)Mcal_Gpio_InitPin(&gpioConfig);

    /*
     * With a 200 MHz Timer0 input clock:
     *
     * Timer tick = (65535 + 1) / 200 MHz = 327.68 us
     * Overflow   = (999 + 1) * 327.68 us = 327.68 ms
     */
    timerConfig.timer = MCAL_TIMER_0;
    timerConfig.period = 999U;
    timerConfig.prescaler = 65535U;

    (void)Mcal_Timer_Init(&timerConfig);

    /*
     * Remove any stale peripheral event before opening the interrupt path.
     */
    (void)Mcal_Timer_ClearFlag(MCAL_TIMER_0);

    /*
     * Timer0 interrupt route:
     * Timer0 -> PIE Group 1 / Channel 7 -> CPU INT1.
     */
    (void)Mcal_Pie_Enable(
        MCAL_PIE_GROUP_1,
        TIMER0_PIE_CHANNEL);

    (void)Mcal_CpuInt_Enable(MCAL_CPU_INT_1);
    (void)Mcal_Timer_EnableInt(MCAL_TIMER_0);

    /*
     * Start the interrupt source only after its complete route is configured.
     */
    (void)Mcal_Timer_Start(MCAL_TIMER_0);

    /*
     * Global CPU interrupt enable is the final step.
     */
    Mcal_CpuInt_EnableGlobal();

    for(;;)
    {
        /* Main loop intentionally idle. */
    }
}

/*==============================================================================
 * Interrupt Service Routines
 *============================================================================*/

__interrupt void Timer0Isr(void)
{
    /*
     * Acknowledge the peripheral event first so the next Timer0 event can be
     * observed independently of the current ISR execution.
     */
    (void)Mcal_Timer_ClearFlag(MCAL_TIMER_0);

    (void)Mcal_Gpio_Toggle(0U);

    /*
     * Release PIE Group 1 after servicing the interrupt.
     */
    (void)Mcal_Pie_Ack(MCAL_PIE_GROUP_1);
}
