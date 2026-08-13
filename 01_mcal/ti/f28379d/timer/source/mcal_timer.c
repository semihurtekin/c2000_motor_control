/**
 * @file    mcal_timer.c
 * @brief   C2000 CPU Timer MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stddef.h>

#include "mcal_timer.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_TIMER_TCR_TSS_MASK    (0x0010U)
#define MCAL_TIMER_TCR_TRB_MASK    (0x0020U)
#define MCAL_TIMER_TCR_TIE_MASK    (0x4000U)
#define MCAL_TIMER_TCR_TIF_MASK    (0x8000U)

/*==============================================================================
 * Private Types
 *============================================================================*/

/*==============================================================================
 * Private Variables
 *============================================================================*/

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

/**
 * @brief Returns the register address associated with the requested timer.
 */
static volatile struct CPUTIMER_REGS * GetTimerRegs(
    Mcal_TimerIdType timer);

/**
 * @brief Returns a TCR value with TIF cleared for safe register writes.
 */
static uint16_t GetTcrForWrite(
    volatile struct CPUTIMER_REGS * timerRegs);

/**
 * @brief Checks whether the given configuration is valid.
 */
static Mcal_TimerStatusType IsConfigValid(
    const Mcal_TimerConfigType * config);

/**
 * @brief Checks whether the given timer identifier is valid.
 */
static Mcal_TimerStatusType IsTimerValid(
    Mcal_TimerIdType timer);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_TimerStatusType Mcal_Timer_Init(
    const Mcal_TimerConfigType * config)
{
    Mcal_TimerStatusType status;
    volatile struct CPUTIMER_REGS * timerRegs;
    uint16_t tcrValue;

    status = IsConfigValid(config);

    if(status == MCAL_TIMER_STATUS_OK)
    {
        timerRegs = GetTimerRegs(config->timer);

        /* Stop the timer and disable its local interrupt generation. */
        tcrValue = GetTcrForWrite(timerRegs);
        tcrValue |= MCAL_TIMER_TCR_TSS_MASK;
        tcrValue &= (uint16_t)(~MCAL_TIMER_TCR_TIE_MASK);
        timerRegs->TCR.all = tcrValue;

        /* Configure period. */
        timerRegs->PRD.all = config->period;

        /* Configure divider and reset the prescale counters. */
        timerRegs->TPR.all =
            (uint16_t)(config->prescaler & 0x00FFU);
        timerRegs->TPRH.all =
            (uint16_t)(config->prescaler >> 8U);

        /* Reload the counter and clear any stale overflow flag. */
        tcrValue = GetTcrForWrite(timerRegs);
        tcrValue |= MCAL_TIMER_TCR_TRB_MASK;
        tcrValue |= MCAL_TIMER_TCR_TIF_MASK;
        timerRegs->TCR.all = tcrValue;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_TimerStatusType Mcal_Timer_Start(
    Mcal_TimerIdType timer)
{
    Mcal_TimerStatusType status;
    volatile struct CPUTIMER_REGS * timerRegs;
    uint16_t tcrValue;

    status = IsTimerValid(timer);

    if(status == MCAL_TIMER_STATUS_OK)
    {
        timerRegs = GetTimerRegs(timer);

        /* Reload the counter and start the timer. */
        tcrValue = GetTcrForWrite(timerRegs);
        tcrValue |= MCAL_TIMER_TCR_TRB_MASK;
        tcrValue &= (uint16_t)(~MCAL_TIMER_TCR_TSS_MASK);
        timerRegs->TCR.all = tcrValue;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_TimerStatusType Mcal_Timer_Stop(
    Mcal_TimerIdType timer)
{
    Mcal_TimerStatusType status;
    volatile struct CPUTIMER_REGS * timerRegs;
    uint16_t tcrValue;

    status = IsTimerValid(timer);

    if(status == MCAL_TIMER_STATUS_OK)
    {
        timerRegs = GetTimerRegs(timer);

        tcrValue = GetTcrForWrite(timerRegs);
        tcrValue |= MCAL_TIMER_TCR_TSS_MASK;
        timerRegs->TCR.all = tcrValue;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_TimerStatusType Mcal_Timer_Resume(
    Mcal_TimerIdType timer)
{
    Mcal_TimerStatusType status;
    volatile struct CPUTIMER_REGS * timerRegs;
    uint16_t tcrValue;

    status = IsTimerValid(timer);

    if(status == MCAL_TIMER_STATUS_OK)
    {
        timerRegs = GetTimerRegs(timer);

        tcrValue = GetTcrForWrite(timerRegs);
        tcrValue &= (uint16_t)(~MCAL_TIMER_TCR_TSS_MASK);
        timerRegs->TCR.all = tcrValue;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_TimerStatusType Mcal_Timer_Reload(
    Mcal_TimerIdType timer)
{
    Mcal_TimerStatusType status;
    volatile struct CPUTIMER_REGS * timerRegs;
    uint16_t tcrValue;

    status = IsTimerValid(timer);

    if(status == MCAL_TIMER_STATUS_OK)
    {
        timerRegs = GetTimerRegs(timer);

        tcrValue = GetTcrForWrite(timerRegs);
        tcrValue |= MCAL_TIMER_TCR_TRB_MASK;
        timerRegs->TCR.all = tcrValue;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_TimerStatusType Mcal_Timer_GetCount(
    Mcal_TimerIdType timer,
    uint32_t * countPtr)
{
    Mcal_TimerStatusType status;
    volatile struct CPUTIMER_REGS * timerRegs;

    status = IsTimerValid(timer);

    if(status == MCAL_TIMER_STATUS_OK)
    {
        if(countPtr != NULL)
        {
            timerRegs = GetTimerRegs(timer);
            *countPtr = timerRegs->TIM.all;
        }
        else
        {
            status = MCAL_TIMER_STATUS_INV_ARG;
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_TimerStatusType Mcal_Timer_IsElapsed(
    Mcal_TimerIdType timer,
    uint16_t * elapsedPtr)
{
    Mcal_TimerStatusType status;
    volatile struct CPUTIMER_REGS * timerRegs;

    status = IsTimerValid(timer);

    if(status == MCAL_TIMER_STATUS_OK)
    {
        if(elapsedPtr != NULL)
        {
            timerRegs = GetTimerRegs(timer);

            if((timerRegs->TCR.all &
                MCAL_TIMER_TCR_TIF_MASK) != 0U)
            {
                *elapsedPtr = 1U;
            }
            else
            {
                *elapsedPtr = 0U;
            }
        }
        else
        {
            status = MCAL_TIMER_STATUS_INV_ARG;
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_TimerStatusType Mcal_Timer_ClearFlag(
    Mcal_TimerIdType timer)
{
    Mcal_TimerStatusType status;
    volatile struct CPUTIMER_REGS * timerRegs;
    uint16_t tcrValue;

    status = IsTimerValid(timer);

    if(status == MCAL_TIMER_STATUS_OK)
    {
        timerRegs = GetTimerRegs(timer);

        tcrValue = GetTcrForWrite(timerRegs);
        tcrValue |= MCAL_TIMER_TCR_TIF_MASK;
        timerRegs->TCR.all = tcrValue;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_TimerStatusType Mcal_Timer_EnableInt(
    Mcal_TimerIdType timer)
{
    Mcal_TimerStatusType status;
    volatile struct CPUTIMER_REGS * timerRegs;
    uint16_t tcrValue;

    status = IsTimerValid(timer);

    if(status == MCAL_TIMER_STATUS_OK)
    {
        timerRegs = GetTimerRegs(timer);
        tcrValue = GetTcrForWrite(timerRegs);

        tcrValue |= MCAL_TIMER_TCR_TIE_MASK;
        timerRegs->TCR.all = tcrValue;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_TimerStatusType Mcal_Timer_DisableInt(
    Mcal_TimerIdType timer)
{
    Mcal_TimerStatusType status;
    volatile struct CPUTIMER_REGS * timerRegs;
    uint16_t tcrValue;

    status = IsTimerValid(timer);

    if(status == MCAL_TIMER_STATUS_OK)
    {
        timerRegs = GetTimerRegs(timer);
        tcrValue = GetTcrForWrite(timerRegs);

        tcrValue &= (uint16_t)(~MCAL_TIMER_TCR_TIE_MASK);
        timerRegs->TCR.all = tcrValue;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

static volatile struct CPUTIMER_REGS * GetTimerRegs(
    Mcal_TimerIdType timer)
{
    volatile struct CPUTIMER_REGS * timerRegs;

    timerRegs = NULL;

    switch(timer)
    {
        case MCAL_TIMER_0:
            timerRegs = &CpuTimer0Regs;
            break;

        case MCAL_TIMER_1:
            timerRegs = &CpuTimer1Regs;
            break;

        case MCAL_TIMER_2:
            timerRegs = &CpuTimer2Regs;
            break;

        default:
            break;
    }

    return timerRegs;
}

static uint16_t GetTcrForWrite(
    volatile struct CPUTIMER_REGS * timerRegs)
{
    uint16_t tcrValue;

    tcrValue = timerRegs->TCR.all;
    tcrValue &= (uint16_t)(~MCAL_TIMER_TCR_TIF_MASK);

    return tcrValue;
}

static Mcal_TimerStatusType IsConfigValid(
    const Mcal_TimerConfigType * config)
{
    Mcal_TimerStatusType status;

    if(config != NULL)
    {
        status = IsTimerValid(config->timer);
    }
    else
    {
        status = MCAL_TIMER_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_TimerStatusType IsTimerValid(
    Mcal_TimerIdType timer)
{
    Mcal_TimerStatusType status;

    if((timer == MCAL_TIMER_0) ||
       (timer == MCAL_TIMER_1) ||
       (timer == MCAL_TIMER_2))
    {
        status = MCAL_TIMER_STATUS_OK;
    }
    else
    {
        status = MCAL_TIMER_STATUS_INV_ID;
    }

    return status;
}
