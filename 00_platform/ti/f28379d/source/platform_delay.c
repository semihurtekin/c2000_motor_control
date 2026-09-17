/**
 * @file    platform_delay.c
 * @brief   F28379D platform blocking delay implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "platform_delay.h"

#include "platform_clock.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define PLATFORM_DELAY_US_PER_SECOND       (1000000UL)

#define PLATFORM_DELAY_CYCLES_PER_US       (PLATFORM_CLOCK_SYSCLK_HZ / PLATFORM_DELAY_US_PER_SECOND)


/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

void Platform_DelayUs(
    uint32_t delayUs)
{
    uint32_t remainingUs;

    remainingUs = delayUs;

    if(remainingUs > 0U)
    {
        while(remainingUs > 0U)
        {
            asm(" RPT #199 || NOP");

            remainingUs--;
        }
    }
    else
    {
        /* Do nothing. */
    }
}
