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
    PLATFORM_CLOCK_STATUS_PLL_FAIL,
    PLATFORM_CLOCK_STATUS_INVALID_PARAM
} Platform_ClockStatusType;

typedef enum
{
    PLATFORM_EPWM_MODULE_1 = 0U,
    PLATFORM_EPWM_MODULE_2,
    PLATFORM_EPWM_MODULE_3,
    PLATFORM_EPWM_MODULE_4,
    PLATFORM_EPWM_MODULE_5,
    PLATFORM_EPWM_MODULE_6,
    PLATFORM_EPWM_MODULE_7,
    PLATFORM_EPWM_MODULE_8,
    PLATFORM_EPWM_MODULE_9,
    PLATFORM_EPWM_MODULE_10,
    PLATFORM_EPWM_MODULE_11,
    PLATFORM_EPWM_MODULE_12
} Platform_EpwmModuleType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes the system clock.
 *
 * @return Clock status.
 */
Platform_ClockStatusType Platform_ClockInit(void);

/**
 * @brief Enables the peripheral clock for an ePWM module.
 *
 * @param module ePWM module whose peripheral clock shall be enabled.
 *
 * @return Clock status.
 */
Platform_ClockStatusType Platform_ClockEnableEpwm(
    Platform_EpwmModuleType module);

/**
 * @brief Returns the configured ePWM peripheral clock frequency.
 *
 * @return ePWM peripheral clock frequency in Hz.
 */
uint32_t Platform_ClockGetEpwmClkHz(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_CLOCK_H */

