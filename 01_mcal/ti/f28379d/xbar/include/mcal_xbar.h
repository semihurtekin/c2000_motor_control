/**
 * @file    mcal_xbar.h
 * @brief   F28379D X-BAR MCAL driver interface.
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
    MCAL_XBAR_EPWM_TRIP_4 = 4U,
    MCAL_XBAR_EPWM_TRIP_5 = 5U,
    MCAL_XBAR_EPWM_TRIP_7 = 7U,
    MCAL_XBAR_EPWM_TRIP_8 = 8U,
    MCAL_XBAR_EPWM_TRIP_9 = 9U,
    MCAL_XBAR_EPWM_TRIP_10 = 10U,
    MCAL_XBAR_EPWM_TRIP_11 = 11U,
    MCAL_XBAR_EPWM_TRIP_12 = 12U
} Mcal_XbarEpwmTripType;

typedef enum
{
    MCAL_XBAR_CMPSS_1 = 1U,
    MCAL_XBAR_CMPSS_2 = 2U,
    MCAL_XBAR_CMPSS_3 = 3U,
    MCAL_XBAR_CMPSS_4 = 4U,
    MCAL_XBAR_CMPSS_5 = 5U,
    MCAL_XBAR_CMPSS_6 = 6U,
    MCAL_XBAR_CMPSS_7 = 7U,
    MCAL_XBAR_CMPSS_8 = 8U
} Mcal_XbarCmpssType;

typedef enum
{
    MCAL_XBAR_CMPSS_HIGH = 0U,
    MCAL_XBAR_CMPSS_LOW = 1U,
    MCAL_XBAR_CMPSS_HIGH_OR_LOW = 2U
} Mcal_XbarCmpssOutputType;

typedef enum
{
    MCAL_XBAR_STATUS_OK = 0U,
    MCAL_XBAR_STATUS_INV_INPUT = 1U,
    MCAL_XBAR_STATUS_INV_PIN = 2U,
    MCAL_XBAR_STATUS_INV_TRIP = 3U,
    MCAL_XBAR_STATUS_INV_CMPSS = 4U,
    MCAL_XBAR_STATUS_INV_SOURCE = 5U
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

/**
 * @brief Adds a CMPSS output as a source for an ePWM X-BAR trip.
 *
 * Multiple CMPSS sources may be added to the same ePWM trip.
 *
 * @param trip ePWM X-BAR trip output.
 * @param cmpss CMPSS module.
 * @param output CMPSS output to route.
 *
 * @return Driver status.
 */
Mcal_XbarStatusType Mcal_Xbar_AddCmpssTripSource(
    Mcal_XbarEpwmTripType trip,
    Mcal_XbarCmpssType cmpss,
    Mcal_XbarCmpssOutputType output);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_XBAR_H */
