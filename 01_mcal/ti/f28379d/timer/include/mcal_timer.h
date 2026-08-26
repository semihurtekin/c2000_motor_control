/**
 * @file    mcal_timer.h
 * @brief   C2000 CPU Timer MCAL driver interface.
 */

#ifndef MCAL_TIMER_H
#define MCAL_TIMER_H

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
    MCAL_TIMER_0 = 0U,
    MCAL_TIMER_1 = 1U,
    MCAL_TIMER_2 = 2U
} Mcal_TimerIdType;

typedef enum
{
    MCAL_TIMER_STATUS_OK = 0U,
    MCAL_TIMER_STATUS_INV_ID = 1U,
    MCAL_TIMER_STATUS_INV_ARG = 2U
} Mcal_TimerStatusType;

typedef struct
{
    Mcal_TimerIdType timer;
    uint32_t period;
    uint16_t prescaler;
} Mcal_TimerConfigType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes the selected CPU timer and leaves it stopped.
 *
 * @param config Timer configuration.
 *
 * @return Driver status.
 */
Mcal_TimerStatusType Mcal_Timer_Init(
    const Mcal_TimerConfigType * config);

/**
 * @brief Starts the selected CPU timer after reloading the period value.
 *
 * @param timer CPU timer identifier.
 *
 * @return Driver status.
 */
Mcal_TimerStatusType Mcal_Timer_Start(
    Mcal_TimerIdType timer);

/**
 * @brief Stops the selected CPU timer.
 *
 * @param timer CPU timer identifier.
 *
 * @return Driver status.
 */
Mcal_TimerStatusType Mcal_Timer_Stop(
    Mcal_TimerIdType timer);

/**
 * @brief Resumes the selected CPU timer without reloading the counter.
 *
 * @param timer CPU timer identifier.
 *
 * @return Driver status.
 */
Mcal_TimerStatusType Mcal_Timer_Resume(
    Mcal_TimerIdType timer);

/**
 * @brief Reloads the selected CPU timer counter from its period register.
 *
 * @param timer CPU timer identifier.
 *
 * @return Driver status.
 */
Mcal_TimerStatusType Mcal_Timer_Reload(
    Mcal_TimerIdType timer);

/**
 * @brief Gets the current count value of the selected CPU timer.
 *
 * @param timer    CPU timer identifier.
 * @param countPtr Address where the counter value is stored.
 *
 * @return Driver status.
 */
Mcal_TimerStatusType Mcal_Timer_GetCount(
    Mcal_TimerIdType timer,
    uint32_t * countPtr);

/**
 * @brief Gets the overflow status of the selected CPU timer.
 *
 * @param timer      CPU timer identifier.
 * @param elapsedPtr Address where 0U or 1U is stored.
 *
 * @return Driver status.
 */
Mcal_TimerStatusType Mcal_Timer_IsElapsed(
    Mcal_TimerIdType timer,
    uint16_t * elapsedPtr);

/**
 * @brief Clears the overflow flag of the selected CPU timer.
 *
 * @param timer CPU timer identifier.
 *
 * @return Driver status.
 */
Mcal_TimerStatusType Mcal_Timer_ClearFlag(
    Mcal_TimerIdType timer);

/**
 * @brief Enables local interrupt generation for the selected CPU timer.
 *
 * @param timer CPU timer identifier.
 *
 * @return Driver status.
 */
Mcal_TimerStatusType Mcal_Timer_EnableInt(
    Mcal_TimerIdType timer);

/**
 * @brief Disables local interrupt generation for the selected CPU timer.
 *
 * @param timer CPU timer identifier.
 *
 * @return Driver status.
 */
Mcal_TimerStatusType Mcal_Timer_DisableInt(
    Mcal_TimerIdType timer);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_TIMER_H */
