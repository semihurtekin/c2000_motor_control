/**
 * @file    platform_delay.h
 * @brief   F28379D platform blocking delay interface.
 */

#ifndef PLATFORM_DELAY_H
#define PLATFORM_DELAY_H

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Performs a blocking delay of at least the requested duration.
 *
 * @param delayUs Minimum delay duration in microseconds.
 */
void Platform_DelayUs(
    uint32_t delayUs);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_DELAY_H */
