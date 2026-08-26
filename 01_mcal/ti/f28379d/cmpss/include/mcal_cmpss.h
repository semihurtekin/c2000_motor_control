/**
 * @file    mcal_cmpss.h
 * @brief   F28379D CMPSS MCAL driver interface.
 */

#ifndef MCAL_CMPSS_H
#define MCAL_CMPSS_H

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
    MCAL_CMPSS_1 = 1U,
    MCAL_CMPSS_2 = 2U,
    MCAL_CMPSS_3 = 3U,
    MCAL_CMPSS_4 = 4U,
    MCAL_CMPSS_5 = 5U,
    MCAL_CMPSS_6 = 6U,
    MCAL_CMPSS_7 = 7U,
    MCAL_CMPSS_8 = 8U
} Mcal_CmpssIdType;

typedef enum
{
    MCAL_CMPSS_HIGH = 0U,
    MCAL_CMPSS_LOW = 1U
} Mcal_CmpssComparatorType;

typedef enum
{
    MCAL_CMPSS_INPUT_INTERNAL_DAC = 0U,
    MCAL_CMPSS_INPUT_EXTERNAL_PIN = 1U
} Mcal_CmpssInputSourceType;

typedef enum
{
    MCAL_CMPSS_POLARITY_NORMAL = 0U,
    MCAL_CMPSS_POLARITY_INVERTED = 1U
} Mcal_CmpssPolarityType;

typedef enum
{
    MCAL_CMPSS_TRIP_ASYNC = 0U,
    MCAL_CMPSS_TRIP_SYNC = 1U,
    MCAL_CMPSS_TRIP_FILTER = 2U,
    MCAL_CMPSS_TRIP_LATCH = 3U
} Mcal_CmpssTripSourceType;

typedef enum
{
    MCAL_CMPSS_DAC_REF_VDDA = 0U,
    MCAL_CMPSS_DAC_REF_VDAC = 1U
} Mcal_CmpssDacRefType;

typedef enum
{
    MCAL_CMPSS_STATUS_OK = 0U,
    MCAL_CMPSS_STATUS_INV_ID = 1U,
    MCAL_CMPSS_STATUS_INV_ARG = 2U
} Mcal_CmpssStatusType;

typedef struct
{
    Mcal_CmpssIdType module;
    Mcal_CmpssComparatorType comparator;
    Mcal_CmpssInputSourceType inputSource;
    Mcal_CmpssPolarityType polarity;
    Mcal_CmpssTripSourceType tripSource;
} Mcal_CmpssCmpConfigType;

typedef struct
{
    Mcal_CmpssIdType module;
    Mcal_CmpssDacRefType reference;
    uint16_t highValue;
    uint16_t lowValue;
} Mcal_CmpssDacConfigType;

typedef struct
{
    Mcal_CmpssIdType module;
    Mcal_CmpssComparatorType comparator;
    uint16_t samplePeriodCycles;
    uint16_t sampleWindow;
    uint16_t threshold;
} Mcal_CmpssFilterConfigType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes one CMPSS module to a known disabled state.
 *
 * @param module Selected CMPSS module.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_Init(
    Mcal_CmpssIdType module);

/**
 * @brief Configures one high or low comparator path.
 *
 * @param config Comparator configuration.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_InitComparator(
    const Mcal_CmpssCmpConfigType * config);

/**
 * @brief Configures the internal high and low comparator DAC thresholds.
 *
 * @param config DAC configuration.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_InitDac(
    const Mcal_CmpssDacConfigType * config);

/**
 * @brief Configures and initializes one CMPSS digital filter.
 *
 * @param config Digital-filter configuration.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_InitFilter(
    const Mcal_CmpssFilterConfigType * config);

/**
 * @brief Sets comparator hysteresis for one CMPSS module.
 *
 * @param module Selected CMPSS module.
 * @param multiplier Hysteresis multiplier.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_SetHysteresis(
    Mcal_CmpssIdType module,
    uint16_t multiplier);

/**
 * @brief Enables the comparator and DAC analog circuitry.
 *
 * @param module Selected CMPSS module.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_Enable(
    Mcal_CmpssIdType module);

/**
 * @brief Disables the comparator and DAC analog circuitry.
 *
 * @param module Selected CMPSS module.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_Disable(
    Mcal_CmpssIdType module);

/**
 * @brief Updates one internal DAC threshold.
 *
 * @param module Selected CMPSS module.
 * @param comparator Selected high or low comparator.
 * @param value 12-bit DAC code in the range 0 through 4095.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_SetDacValue(
    Mcal_CmpssIdType module,
    Mcal_CmpssComparatorType comparator,
    uint16_t value);

/**
 * @brief Reads the selected comparator digital-filter output state.
 *
 * @param module Selected CMPSS module.
 * @param comparator Selected high or low comparator.
 * @param active Receives 1U when the filter output is active, otherwise 0U.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_IsFilterActive(
    Mcal_CmpssIdType module,
    Mcal_CmpssComparatorType comparator,
    uint16_t * active);

/**
 * @brief Reads the selected comparator digital-filter latch state.
 *
 * @param module Selected CMPSS module.
 * @param comparator Selected high or low comparator.
 * @param active Receives 1U when the filter latch is active, otherwise 0U.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_IsLatchActive(
    Mcal_CmpssIdType module,
    Mcal_CmpssComparatorType comparator,
    uint16_t * active);

/**
 * @brief Clears the selected comparator digital-filter latch.
 *
 * @param module Selected CMPSS module.
 * @param comparator Selected high or low comparator.
 *
 * @return Driver status.
 */
Mcal_CmpssStatusType Mcal_Cmpss_ClearLatch(
    Mcal_CmpssIdType module,
    Mcal_CmpssComparatorType comparator);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_CMPSS_H */
