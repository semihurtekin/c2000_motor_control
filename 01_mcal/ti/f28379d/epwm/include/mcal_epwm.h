/**
 * @file    mcal_epwm.h
 * @brief   F28379D ePWM MCAL driver interface.
 */

#ifndef MCAL_EPWM_H
#define MCAL_EPWM_H

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
    MCAL_EPWM_1 = 1U,
    MCAL_EPWM_2 = 2U,
    MCAL_EPWM_3 = 3U,
    MCAL_EPWM_4 = 4U,
    MCAL_EPWM_5 = 5U,
    MCAL_EPWM_6 = 6U,
    MCAL_EPWM_7 = 7U,
    MCAL_EPWM_8 = 8U,
    MCAL_EPWM_9 = 9U,
    MCAL_EPWM_10 = 10U,
    MCAL_EPWM_11 = 11U,
    MCAL_EPWM_12 = 12U
} Mcal_EpwmIdType;

typedef enum
{
    MCAL_EPWM_COUNT_UP = 0U,
    MCAL_EPWM_COUNT_DOWN = 1U,
    MCAL_EPWM_COUNT_UP_DOWN = 2U,
    MCAL_EPWM_COUNT_FREEZE = 3U
} Mcal_EpwmCountModeType;

typedef enum
{
    MCAL_EPWM_CLKDIV_1 = 0U,
    MCAL_EPWM_CLKDIV_2 = 1U,
    MCAL_EPWM_CLKDIV_4 = 2U,
    MCAL_EPWM_CLKDIV_8 = 3U,
    MCAL_EPWM_CLKDIV_16 = 4U,
    MCAL_EPWM_CLKDIV_32 = 5U,
    MCAL_EPWM_CLKDIV_64 = 6U,
    MCAL_EPWM_CLKDIV_128 = 7U
} Mcal_EpwmClkDivType;

typedef enum
{
    MCAL_EPWM_HSCLKDIV_1 = 0U,
    MCAL_EPWM_HSCLKDIV_2 = 1U,
    MCAL_EPWM_HSCLKDIV_4 = 2U,
    MCAL_EPWM_HSCLKDIV_6 = 3U,
    MCAL_EPWM_HSCLKDIV_8 = 4U,
    MCAL_EPWM_HSCLKDIV_10 = 5U,
    MCAL_EPWM_HSCLKDIV_12 = 6U,
    MCAL_EPWM_HSCLKDIV_14 = 7U
} Mcal_EpwmHsClkDivType;

typedef enum
{
    MCAL_EPWM_STATUS_OK = 0U,
    MCAL_EPWM_STATUS_INV_ID = 1U,
    MCAL_EPWM_STATUS_INV_ARG = 2U
} Mcal_EpwmStatusType;

typedef struct
{
    Mcal_EpwmIdType module;
    uint16_t period;
    Mcal_EpwmCountModeType mode;
    Mcal_EpwmClkDivType clkDiv;
    Mcal_EpwmHsClkDivType hsClkDiv;
} Mcal_EpwmTbConfigType;

typedef struct
{
    Mcal_EpwmIdType module;
    uint16_t compareA;
} Mcal_EpwmCompareConfigType;

typedef struct
{
    Mcal_EpwmIdType module;
    uint16_t risingDelay;
    uint16_t fallingDelay;
} Mcal_EpwmDeadBandConfigType;


typedef enum
{
    MCAL_EPWM_TRIP_SOURCE_TZ1 = 1U,
    MCAL_EPWM_TRIP_SOURCE_TZ4 = 4U
} Mcal_EpwmTripSourceType;


typedef enum
{
    MCAL_EPWM_ADC_SOCA = 0U,
    MCAL_EPWM_ADC_SOCB = 1U
} Mcal_EpwmAdcSocType;

/**
 * @brief ePWM timing event used to generate an ADC SOC pulse.
 *
 * Compare-based values in this version use the CMPA/CMPB compare family.
 * The current public API exposes CMPA up/down events only.
 */
typedef enum
{
    MCAL_EPWM_ADC_TRIG_ZERO = 1U,
    MCAL_EPWM_ADC_TRIG_PERIOD = 2U,
    MCAL_EPWM_ADC_TRIG_ZERO_PERIOD = 3U,
    MCAL_EPWM_ADC_TRIG_CMPA_UP = 4U,
    MCAL_EPWM_ADC_TRIG_CMPA_DOWN = 5U
} Mcal_EpwmAdcTrigSourceType;

typedef enum
{
    MCAL_EPWM_TBCLK_SYNC_DISABLE = 0U,
    MCAL_EPWM_TBCLK_SYNC_ENABLE = 1U
} Mcal_EpwmTbClkSyncType;

typedef struct
{
    Mcal_EpwmIdType module;
    Mcal_EpwmAdcSocType soc;
    Mcal_EpwmAdcTrigSourceType source;
    uint16_t eventPrescale;
} Mcal_EpwmAdcTrigConfigType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes the selected ePWM time-base submodule.
 *
 * @param config Time-base configuration.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_InitTimeBase(
    const Mcal_EpwmTbConfigType * config);

/**
 * @brief Initializes compare A and ePWMxA action qualifier behavior.
 *
 * Compare A uses shadow mode. The shadow value becomes active when TBCTR
 * reaches TBPRD.
 *
 * @param config Compare A configuration.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_InitCompareA(
    const Mcal_EpwmCompareConfigType * config);

/**
 * @brief Updates the compare A shadow value.
 *
 * The new compare value becomes active when TBCTR reaches TBPRD.
 *
 * @param module Selected ePWM module.
 * @param compare Compare A value.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_SetCompareA(
    Mcal_EpwmIdType module,
    uint16_t compare);

/**
 * @brief Initializes complementary dead-band generation.
 *
 * Both dead-band paths use ePWMxA as the source. Rising-edge and falling-edge
 * delay paths are enabled. The resulting outputs use active-high complementary
 * polarity and full-cycle dead-band clocking.
 *
 * @param config Dead-band configuration.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_InitDeadBand(
    const Mcal_EpwmDeadBandConfigType * config);


/**
 * @brief Initializes the one-shot Trip Zone safety policy.
 *
 * A one-shot trip forces both ePWMxA and ePWMxB LOW. No external trip source
 * is selected by this function. The one-shot latch is cleared during
 * initialization.
 *
 * @param module Selected ePWM module.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_InitTrip(
    Mcal_EpwmIdType module);


/**
 * @brief Enables an external source as a one-shot Trip Zone source.
 *
 * This version supports TZ1. The routing that drives TZ1 is configured
 * separately by the Input X-BAR MCAL driver.
 *
 * @param module Selected ePWM module.
 * @param source One-shot trip source.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_EnableOneShotTrip(
    Mcal_EpwmIdType module,
    Mcal_EpwmTripSourceType source);

/**
 * @brief Forces a one-shot Trip Zone event by software.
 *
 * @param module Selected ePWM module.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_ForceTrip(
    Mcal_EpwmIdType module);

/**
 * @brief Clears the latched one-shot Trip Zone event.
 *
 * Safety and recovery conditions shall be evaluated by an upper software
 * layer before this API is called.
 *
 * @param module Selected ePWM module.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_ClearTrip(
    Mcal_EpwmIdType module);

/**
 * @brief Reads the latched one-shot Trip Zone status.
 *
 * @param module Selected ePWM module.
 * @param active Receives 1U when an OST is latched, otherwise 0U.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_IsTripActive(
    Mcal_EpwmIdType module,
    uint16_t * active);


/**
 * @brief Initializes an ePWM ADC start-of-conversion trigger.
 *
 * The selected SOCA or SOCB trigger is configured and enabled. The event
 * prescaler supports one trigger every 1 through 15 selected ePWM events.
 *
 * This initialization API is intended to be called while the ePWM time-base
 * clock is stopped.
 *
 * @param config ADC trigger configuration.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_InitAdcTrigger(
    const Mcal_EpwmAdcTrigConfigType * config);

/**
 * @brief Reads the selected ePWM ADC trigger event flag.
 *
 * @param module Selected ePWM module.
 * @param soc Selected SOCA or SOCB trigger.
 * @param flagSet Receives 1U when the event flag is set, otherwise 0U.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_IsAdcTrigFlagSet(
    Mcal_EpwmIdType module,
    Mcal_EpwmAdcSocType soc,
    uint16_t * flagSet);

/**
 * @brief Clears the selected ePWM ADC trigger event flag.
 *
 * @param module Selected ePWM module.
 * @param soc Selected SOCA or SOCB trigger.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_ClearAdcTrigFlag(
    Mcal_EpwmIdType module,
    Mcal_EpwmAdcSocType soc);

/**
 * @brief Controls the global ePWM time-base clock synchronization gate.
 *
 * @param state Time-base clock synchronization state.
 *
 * @return Driver status.
 */
Mcal_EpwmStatusType Mcal_Epwm_SetTbClkSync(
    Mcal_EpwmTbClkSyncType state);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_EPWM_H */
