/**
 * @file    mcal_xbar.h
 * @brief   F28379D Input X-BAR MCAL driver interface.
 */

#ifndef MCAL_XBAR_H
#define MCAL_XBAR_H

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
    MCAL_XBAR_INPUT_1 = 1U,
    MCAL_XBAR_INPUT_2 = 2U,
    MCAL_XBAR_INPUT_3 = 3U,
    MCAL_XBAR_INPUT_4 = 4U,
    MCAL_XBAR_INPUT_5 = 5U,
    MCAL_XBAR_INPUT_6 = 6U,
    MCAL_XBAR_INPUT_7 = 7U,
    MCAL_XBAR_INPUT_8 = 8U,
    MCAL_XBAR_INPUT_9 = 9U,
    MCAL_XBAR_INPUT_10 = 10U,
    MCAL_XBAR_INPUT_11 = 11U,
    MCAL_XBAR_INPUT_12 = 12U,
    MCAL_XBAR_INPUT_13 = 13U,
    MCAL_XBAR_INPUT_14 = 14U
} Mcal_XbarInputType;

typedef enum
{
    MCAL_XBAR_STATUS_OK = 0U,
    MCAL_XBAR_STATUS_INV_INPUT = 1U,
    MCAL_XBAR_STATUS_INV_PIN = 2U
} Mcal_XbarStatusType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Routes a GPIO input to an Input X-BAR channel.
 *
 * The GPIO number is the MCU GPIO index, not a LaunchPad header pin number.
 *
 * @param input Input X-BAR channel.
 * @param gpioPin MCU GPIO number in the range GPIO0 through GPIO168.
 *
 * @return Driver status.
 */
Mcal_XbarStatusType Mcal_Xbar_SetInput(
    Mcal_XbarInputType input,
    uint16_t gpioPin);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_XBAR_H */
