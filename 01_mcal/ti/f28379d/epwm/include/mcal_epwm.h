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

#ifdef __cplusplus
}
#endif

#endif /* MCAL_EPWM_H */
