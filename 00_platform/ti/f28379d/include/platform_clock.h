/**
 * @file    platform_clock.h
 * @brief   F28379D platform clock driver interface.
 */

#ifndef PLATFORM_CLOCK_H
#define PLATFORM_CLOCK_H

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * Public Macros
 *============================================================================*/


/*==============================================================================
 * Public Types
 *============================================================================*/

typedef enum
{
    PLATFORM_CLOCK_STATUS_OK = 0U,
    PLATFORM_CLOCK_STATUS_PLL_FAIL
}Platform_ClockStatusType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes the system clock.
 *
 * @return Clock status.
 */
Platform_ClockStatusType Platform_ClockInit(void);


#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_CLOCK_H */
