/**
 * @file    mcal_epwm.c
 * @brief   F28379D ePWM MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stddef.h>

#include "mcal_epwm.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_EPWM_PHASE_LOAD_DISABLE    (0U)
#define MCAL_EPWM_PERIOD_DIRECT_LOAD    (1U)

#define MCAL_EPWM_CMP_SHADOW_MODE       (0U)
#define MCAL_EPWM_CMP_LOAD_PERIOD       (1U)

#define MCAL_EPWM_AQ_NO_ACTION          (0U)
#define MCAL_EPWM_AQ_CLEAR              (1U)
#define MCAL_EPWM_AQ_SET                (2U)

#define MCAL_EPWM_DB_INPUT_A_ALL        (0U)
#define MCAL_EPWM_DB_FULL_ENABLE        (3U)
#define MCAL_EPWM_DB_ACTIVE_HIGH_COMP   (2U)
#define MCAL_EPWM_DB_FULL_CYCLE         (0U)
#define MCAL_EPWM_DB_MAX_DELAY          (0x3FFFU)

#define MCAL_EPWM_TZ_FORCE_LOW           (2U)
#define MCAL_EPWM_TZ_INACTIVE            (0U)
#define MCAL_EPWM_TZ_ACTIVE              (1U)

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static volatile struct EPWM_REGS * GetEpwmRegs(
    Mcal_EpwmIdType module);

static Mcal_EpwmStatusType IsTbConfigValid(
    const Mcal_EpwmTbConfigType * config);

static Mcal_EpwmStatusType IsCmpConfigValid(
    const Mcal_EpwmCompareConfigType * config);

static Mcal_EpwmStatusType IsDbConfigValid(
    const Mcal_EpwmDeadBandConfigType * config);

static Mcal_EpwmStatusType IsModuleValid(
    Mcal_EpwmIdType module);

static Mcal_EpwmStatusType IsModeValid(
    Mcal_EpwmCountModeType mode);

static Mcal_EpwmStatusType IsClkDivValid(
    Mcal_EpwmClkDivType clkDiv);

static Mcal_EpwmStatusType IsHsClkDivValid(
    Mcal_EpwmHsClkDivType hsClkDiv);

static Mcal_EpwmStatusType IsCompareValid(
    Mcal_EpwmIdType module,
    uint16_t compare);

static Mcal_EpwmStatusType IsTripSourceValid(
    Mcal_EpwmTripSourceType source);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_EpwmStatusType Mcal_Epwm_InitTimeBase(
    const Mcal_EpwmTbConfigType * config)
{
    Mcal_EpwmStatusType status;
    volatile struct EPWM_REGS * epwmRegs;

    status = IsTbConfigValid(config);

    if(status == MCAL_EPWM_STATUS_OK)
    {
        epwmRegs = GetEpwmRegs(config->module);

        epwmRegs->TBCTL.bit.CTRMODE =
            (uint16_t)MCAL_EPWM_COUNT_FREEZE;

        epwmRegs->TBCTL.bit.PRDLD =
            MCAL_EPWM_PERIOD_DIRECT_LOAD;

        epwmRegs->TBCTR = 0U;
        epwmRegs->TBPRD = config->period;

        epwmRegs->TBCTL.bit.CLKDIV =
            (uint16_t)config->clkDiv;
        epwmRegs->TBCTL.bit.HSPCLKDIV =
            (uint16_t)config->hsClkDiv;

        epwmRegs->TBCTL.bit.PHSEN =
            MCAL_EPWM_PHASE_LOAD_DISABLE;
        epwmRegs->TBPHS.all = 0UL;

        epwmRegs->TBCTL.bit.CTRMODE =
            (uint16_t)config->mode;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_EpwmStatusType Mcal_Epwm_InitCompareA(
    const Mcal_EpwmCompareConfigType * config)
{
    Mcal_EpwmStatusType status;
    volatile struct EPWM_REGS * epwmRegs;

    status = IsCmpConfigValid(config);

    if(status == MCAL_EPWM_STATUS_OK)
    {
        epwmRegs = GetEpwmRegs(config->module);

        epwmRegs->CMPCTL.bit.LOADAMODE =
            MCAL_EPWM_CMP_LOAD_PERIOD;
        epwmRegs->CMPCTL.bit.SHDWAMODE =
            MCAL_EPWM_CMP_SHADOW_MODE;
        epwmRegs->CMPA.bit.CMPA =
            config->compareA;

        epwmRegs->AQCTLA.all =
            MCAL_EPWM_AQ_NO_ACTION;
        epwmRegs->AQCTLA.bit.CAU =
            MCAL_EPWM_AQ_CLEAR;
        epwmRegs->AQCTLA.bit.CAD =
            MCAL_EPWM_AQ_SET;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_EpwmStatusType Mcal_Epwm_SetCompareA(
    Mcal_EpwmIdType module,
    uint16_t compare)
{
    Mcal_EpwmStatusType status;
    volatile struct EPWM_REGS * epwmRegs;

    status = IsModuleValid(module);

    if(status == MCAL_EPWM_STATUS_OK)
    {
        status = IsCompareValid(module, compare);

        if(status == MCAL_EPWM_STATUS_OK)
        {
            epwmRegs = GetEpwmRegs(module);
            epwmRegs->CMPA.bit.CMPA = compare;
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_EpwmStatusType Mcal_Epwm_InitDeadBand(
    const Mcal_EpwmDeadBandConfigType * config)
{
    Mcal_EpwmStatusType status;
    volatile struct EPWM_REGS * epwmRegs;

    status = IsDbConfigValid(config);

    if(status == MCAL_EPWM_STATUS_OK)
    {
        epwmRegs = GetEpwmRegs(config->module);

        epwmRegs->DBCTL.bit.IN_MODE =
            MCAL_EPWM_DB_INPUT_A_ALL;
        epwmRegs->DBCTL.bit.OUT_MODE =
            MCAL_EPWM_DB_FULL_ENABLE;
        epwmRegs->DBCTL.bit.POLSEL =
            MCAL_EPWM_DB_ACTIVE_HIGH_COMP;
        epwmRegs->DBCTL.bit.HALFCYCLE =
            MCAL_EPWM_DB_FULL_CYCLE;

        epwmRegs->DBRED.bit.DBRED =
            config->risingDelay;
        epwmRegs->DBFED.bit.DBFED =
            config->fallingDelay;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}


Mcal_EpwmStatusType Mcal_Epwm_InitTrip(
    Mcal_EpwmIdType module)
{
    Mcal_EpwmStatusType status;
    volatile struct EPWM_REGS * epwmRegs;

    status = IsModuleValid(module);

    if(status == MCAL_EPWM_STATUS_OK)
    {
        epwmRegs = GetEpwmRegs(module);

        EALLOW;

        /*
         * Start the basic Trip Zone source selection from a known state.
         * External trip sources are enabled explicitly by a separate API.
         */
        epwmRegs->TZSEL.all = 0U;

        /*
         * Any enabled basic Trip Zone event forces both complementary
         * outputs LOW.
         */
        epwmRegs->TZCTL.bit.TZA = MCAL_EPWM_TZ_FORCE_LOW;
        epwmRegs->TZCTL.bit.TZB = MCAL_EPWM_TZ_FORCE_LOW;

        /*
         * Start from a known, non-tripped state.
         */
        epwmRegs->TZCLR.bit.OST = 1U;

        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}


Mcal_EpwmStatusType Mcal_Epwm_EnableOneShotTrip(
    Mcal_EpwmIdType module,
    Mcal_EpwmTripSourceType source)
{
    Mcal_EpwmStatusType status;
    volatile struct EPWM_REGS * epwmRegs;

    status = IsModuleValid(module);

    if(status == MCAL_EPWM_STATUS_OK)
    {
        status = IsTripSourceValid(source);

        if(status == MCAL_EPWM_STATUS_OK)
        {
            epwmRegs = GetEpwmRegs(module);

            EALLOW;

            switch(source)
            {
                case MCAL_EPWM_TRIP_SOURCE_TZ1:
                    epwmRegs->TZSEL.bit.OSHT1 = 1U;
                    break;

                default:
                    /* Do nothing. */
                    break;
            }

            EDIS;
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_EpwmStatusType Mcal_Epwm_ForceTrip(
    Mcal_EpwmIdType module)
{
    Mcal_EpwmStatusType status;
    volatile struct EPWM_REGS * epwmRegs;

    status = IsModuleValid(module);

    if(status == MCAL_EPWM_STATUS_OK)
    {
        epwmRegs = GetEpwmRegs(module);

        EALLOW;
        epwmRegs->TZFRC.bit.OST = 1U;
        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_EpwmStatusType Mcal_Epwm_ClearTrip(
    Mcal_EpwmIdType module)
{
    Mcal_EpwmStatusType status;
    volatile struct EPWM_REGS * epwmRegs;

    status = IsModuleValid(module);

    if(status == MCAL_EPWM_STATUS_OK)
    {
        epwmRegs = GetEpwmRegs(module);

        EALLOW;
        epwmRegs->TZCLR.bit.OST = 1U;
        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_EpwmStatusType Mcal_Epwm_IsTripActive(
    Mcal_EpwmIdType module,
    uint16_t * active)
{
    Mcal_EpwmStatusType status;
    volatile struct EPWM_REGS * epwmRegs;

    status = IsModuleValid(module);

    if(status == MCAL_EPWM_STATUS_OK)
    {
        if(active != NULL)
        {
            epwmRegs = GetEpwmRegs(module);

            if(epwmRegs->TZFLG.bit.OST != 0U)
            {
                *active = MCAL_EPWM_TZ_ACTIVE;
            }
            else
            {
                *active = MCAL_EPWM_TZ_INACTIVE;
            }
        }
        else
        {
            status = MCAL_EPWM_STATUS_INV_ARG;
        }
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

static volatile struct EPWM_REGS * GetEpwmRegs(
    Mcal_EpwmIdType module)
{
    volatile struct EPWM_REGS * epwmRegs;

    epwmRegs = NULL;

    switch(module)
    {
        case MCAL_EPWM_1:
            epwmRegs = &EPwm1Regs;
            break;
        case MCAL_EPWM_2:
            epwmRegs = &EPwm2Regs;
            break;
        case MCAL_EPWM_3:
            epwmRegs = &EPwm3Regs;
            break;
        case MCAL_EPWM_4:
            epwmRegs = &EPwm4Regs;
            break;
        case MCAL_EPWM_5:
            epwmRegs = &EPwm5Regs;
            break;
        case MCAL_EPWM_6:
            epwmRegs = &EPwm6Regs;
            break;
        case MCAL_EPWM_7:
            epwmRegs = &EPwm7Regs;
            break;
        case MCAL_EPWM_8:
            epwmRegs = &EPwm8Regs;
            break;
        case MCAL_EPWM_9:
            epwmRegs = &EPwm9Regs;
            break;
        case MCAL_EPWM_10:
            epwmRegs = &EPwm10Regs;
            break;
        case MCAL_EPWM_11:
            epwmRegs = &EPwm11Regs;
            break;
        case MCAL_EPWM_12:
            epwmRegs = &EPwm12Regs;
            break;
        default:
            /* Do nothing. */
            break;
    }

    return epwmRegs;
}

static Mcal_EpwmStatusType IsTbConfigValid(
    const Mcal_EpwmTbConfigType * config)
{
    Mcal_EpwmStatusType status;

    if(config != NULL)
    {
        status = IsModuleValid(config->module);

        if(status == MCAL_EPWM_STATUS_OK)
        {
            status = IsModeValid(config->mode);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_EPWM_STATUS_OK)
        {
            status = IsClkDivValid(config->clkDiv);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_EPWM_STATUS_OK)
        {
            status = IsHsClkDivValid(config->hsClkDiv);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_EPWM_STATUS_OK)
        {
            if(config->period == 0U)
            {
                status = MCAL_EPWM_STATUS_INV_ARG;
            }
            else
            {
                /* Do nothing. */
            }
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        status = MCAL_EPWM_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_EpwmStatusType IsCmpConfigValid(
    const Mcal_EpwmCompareConfigType * config)
{
    Mcal_EpwmStatusType status;

    if(config != NULL)
    {
        status = IsModuleValid(config->module);

        if(status == MCAL_EPWM_STATUS_OK)
        {
            status = IsCompareValid(
                config->module,
                config->compareA);
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        status = MCAL_EPWM_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_EpwmStatusType IsDbConfigValid(
    const Mcal_EpwmDeadBandConfigType * config)
{
    Mcal_EpwmStatusType status;

    if(config != NULL)
    {
        status = IsModuleValid(config->module);

        if(status == MCAL_EPWM_STATUS_OK)
        {
            if((config->risingDelay <= MCAL_EPWM_DB_MAX_DELAY) &&
               (config->fallingDelay <= MCAL_EPWM_DB_MAX_DELAY))
            {
                /* Do nothing. */
            }
            else
            {
                status = MCAL_EPWM_STATUS_INV_ARG;
            }
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        status = MCAL_EPWM_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_EpwmStatusType IsModuleValid(
    Mcal_EpwmIdType module)
{
    Mcal_EpwmStatusType status;

    if(((uint16_t)module >= (uint16_t)MCAL_EPWM_1) &&
       ((uint16_t)module <= (uint16_t)MCAL_EPWM_12))
    {
        status = MCAL_EPWM_STATUS_OK;
    }
    else
    {
        status = MCAL_EPWM_STATUS_INV_ID;
    }

    return status;
}

static Mcal_EpwmStatusType IsModeValid(
    Mcal_EpwmCountModeType mode)
{
    Mcal_EpwmStatusType status;

    if(((uint16_t)mode >= (uint16_t)MCAL_EPWM_COUNT_UP) &&
       ((uint16_t)mode <= (uint16_t)MCAL_EPWM_COUNT_FREEZE))
    {
        status = MCAL_EPWM_STATUS_OK;
    }
    else
    {
        status = MCAL_EPWM_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_EpwmStatusType IsClkDivValid(
    Mcal_EpwmClkDivType clkDiv)
{
    Mcal_EpwmStatusType status;

    if(((uint16_t)clkDiv >= (uint16_t)MCAL_EPWM_CLKDIV_1) &&
       ((uint16_t)clkDiv <= (uint16_t)MCAL_EPWM_CLKDIV_128))
    {
        status = MCAL_EPWM_STATUS_OK;
    }
    else
    {
        status = MCAL_EPWM_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_EpwmStatusType IsHsClkDivValid(
    Mcal_EpwmHsClkDivType hsClkDiv)
{
    Mcal_EpwmStatusType status;

    if(((uint16_t)hsClkDiv >= (uint16_t)MCAL_EPWM_HSCLKDIV_1) &&
       ((uint16_t)hsClkDiv <= (uint16_t)MCAL_EPWM_HSCLKDIV_14))
    {
        status = MCAL_EPWM_STATUS_OK;
    }
    else
    {
        status = MCAL_EPWM_STATUS_INV_ARG;
    }

    return status;
}


static Mcal_EpwmStatusType IsTripSourceValid(
    Mcal_EpwmTripSourceType source)
{
    Mcal_EpwmStatusType status;

    if(source == MCAL_EPWM_TRIP_SOURCE_TZ1)
    {
        status = MCAL_EPWM_STATUS_OK;
    }
    else
    {
        status = MCAL_EPWM_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_EpwmStatusType IsCompareValid(
    Mcal_EpwmIdType module,
    uint16_t compare)
{
    Mcal_EpwmStatusType status;
    volatile struct EPWM_REGS * epwmRegs;

    epwmRegs = GetEpwmRegs(module);

    if(epwmRegs != NULL)
    {
        if(compare <= epwmRegs->TBPRD)
        {
            status = MCAL_EPWM_STATUS_OK;
        }
        else
        {
            status = MCAL_EPWM_STATUS_INV_ARG;
        }
    }
    else
    {
        status = MCAL_EPWM_STATUS_INV_ID;
    }

    return status;
}
