/**
 * @file    mcal_cmpss.c
 * @brief   F28379D CMPSS MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stddef.h>

#include "mcal_cmpss.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_CMPSS_MODULE_DISABLE       (0U)
#define MCAL_CMPSS_MODULE_ENABLE        (1U)

#define MCAL_CMPSS_ASYNC_OR_DISABLE     (0U)

#define MCAL_CMPSS_DAC_SOURCE_SHADOW    (0U)
#define MCAL_CMPSS_DAC_LOAD_SYSCLK      (0U)

#define MCAL_CMPSS_DAC_VALUE_MAX        (4095U)
#define MCAL_CMPSS_HYSTERESIS_MAX       (4U)

#define MCAL_CMPSS_SAMPLE_CYCLES_MIN    (1U)
#define MCAL_CMPSS_SAMPLE_CYCLES_MAX    (1024U)
#define MCAL_CMPSS_SAMPLE_WINDOW_MIN    (1U)
#define MCAL_CMPSS_SAMPLE_WINDOW_MAX    (32U)

#define MCAL_CMPSS_LATCH_CLEAR          (1U)
#define MCAL_CMPSS_FLAG_RESET           (0U)
#define MCAL_CMPSS_FLAG_SET             (1U)

/*==============================================================================
 * Private Types
 *============================================================================*/

/*==============================================================================
 * Private Variables
 *============================================================================*/

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static volatile struct CMPSS_REGS * GetCmpssRegs(
    Mcal_CmpssIdType module);

static Mcal_CmpssStatusType IsModuleValid(
    Mcal_CmpssIdType module);

static Mcal_CmpssStatusType IsComparatorValid(
    Mcal_CmpssComparatorType comparator);

static Mcal_CmpssStatusType IsInputSourceValid(
    Mcal_CmpssInputSourceType inputSource);

static Mcal_CmpssStatusType IsPolarityValid(
    Mcal_CmpssPolarityType polarity);

static Mcal_CmpssStatusType IsTripSourceValid(
    Mcal_CmpssTripSourceType tripSource);

static Mcal_CmpssStatusType IsDacRefValid(
    Mcal_CmpssDacRefType reference);

static Mcal_CmpssStatusType IsCmpConfigValid(
    const Mcal_CmpssCmpConfigType * config);

static Mcal_CmpssStatusType IsDacConfigValid(
    const Mcal_CmpssDacConfigType * config);

static Mcal_CmpssStatusType IsFilterConfigValid(
    const Mcal_CmpssFilterConfigType * config);

static Mcal_CmpssStatusType IsDacValueValid(
    uint16_t value);

static Mcal_CmpssStatusType IsHysteresisValid(
    uint16_t multiplier);

static void ConfigureComparator(
    volatile struct CMPSS_REGS * cmpssRegs,
    const Mcal_CmpssCmpConfigType * config);

static void ConfigureFilter(
    volatile struct CMPSS_REGS * cmpssRegs,
    const Mcal_CmpssFilterConfigType * config);

static uint16_t ReadFilterStatus(
    volatile struct CMPSS_REGS * cmpssRegs,
    Mcal_CmpssComparatorType comparator);

static uint16_t ReadLatchStatus(
    volatile struct CMPSS_REGS * cmpssRegs,
    Mcal_CmpssComparatorType comparator);

static void ClearLatch(
    volatile struct CMPSS_REGS * cmpssRegs,
    Mcal_CmpssComparatorType comparator);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_CmpssStatusType Mcal_Cmpss_Init(
    Mcal_CmpssIdType module)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsModuleValid(module);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        cmpssRegs = GetCmpssRegs(module);

        EALLOW;

        /*
         * Keep the analog comparator/DAC circuitry disabled while the module
         * is returned to a deterministic configuration.
         */
        cmpssRegs->COMPCTL.bit.COMPDACE =
            MCAL_CMPSS_MODULE_DISABLE;

        /*
         * Default both comparators to their internal DAC threshold and the
         * shortest asynchronous path. A later configuration API may select a
         * synchronized, filtered or latched path before the module is armed.
         */
        cmpssRegs->COMPCTL.bit.COMPHSOURCE =
            (uint16_t)MCAL_CMPSS_INPUT_INTERNAL_DAC;
        cmpssRegs->COMPCTL.bit.COMPHINV =
            (uint16_t)MCAL_CMPSS_POLARITY_NORMAL;
        cmpssRegs->COMPCTL.bit.CTRIPHSEL =
            (uint16_t)MCAL_CMPSS_TRIP_ASYNC;
        cmpssRegs->COMPCTL.bit.CTRIPOUTHSEL =
            (uint16_t)MCAL_CMPSS_TRIP_ASYNC;
        cmpssRegs->COMPCTL.bit.ASYNCHEN =
            MCAL_CMPSS_ASYNC_OR_DISABLE;

        cmpssRegs->COMPCTL.bit.COMPLSOURCE =
            (uint16_t)MCAL_CMPSS_INPUT_INTERNAL_DAC;
        cmpssRegs->COMPCTL.bit.COMPLINV =
            (uint16_t)MCAL_CMPSS_POLARITY_NORMAL;
        cmpssRegs->COMPCTL.bit.CTRIPLSEL =
            (uint16_t)MCAL_CMPSS_TRIP_ASYNC;
        cmpssRegs->COMPCTL.bit.CTRIPOUTLSEL =
            (uint16_t)MCAL_CMPSS_TRIP_ASYNC;
        cmpssRegs->COMPCTL.bit.ASYNCLEN =
            MCAL_CMPSS_ASYNC_OR_DISABLE;

        cmpssRegs->COMPHYSCTL.bit.COMPHYS = 0U;

        /*
         * CMPSS v0.1 uses static DAC thresholds. Ramp generation and
         * PWMSYNC-based DAC loading are intentionally outside this version.
         */
        cmpssRegs->COMPDACCTL.bit.DACSOURCE =
            MCAL_CMPSS_DAC_SOURCE_SHADOW;
        cmpssRegs->COMPDACCTL.bit.SELREF =
            (uint16_t)MCAL_CMPSS_DAC_REF_VDDA;
        cmpssRegs->COMPDACCTL.bit.SWLOADSEL =
            MCAL_CMPSS_DAC_LOAD_SYSCLK;
        cmpssRegs->COMPDACCTL.bit.RAMPSOURCE = 0U;
        cmpssRegs->COMPDACCTL.bit.RAMPLOADSEL = 0U;

        /*
         * Reset both filter blocks to the smallest valid configuration.
         * A filter that is actually used is configured explicitly before
         * enabling the module.
         */
        cmpssRegs->CTRIPHFILCLKCTL.bit.CLKPRESCALE = 0U;
        cmpssRegs->CTRIPHFILCTL.bit.SAMPWIN = 0U;
        cmpssRegs->CTRIPHFILCTL.bit.THRESH = 0U;
        cmpssRegs->CTRIPHFILCTL.bit.FILINIT = 1U;

        cmpssRegs->CTRIPLFILCLKCTL.bit.CLKPRESCALE = 0U;
        cmpssRegs->CTRIPLFILCTL.bit.SAMPWIN = 0U;
        cmpssRegs->CTRIPLFILCTL.bit.THRESH = 0U;
        cmpssRegs->CTRIPLFILCTL.bit.FILINIT = 1U;

        /*
         * Automatic PWMSYNC latch clearing is disabled. Fault recovery is an
         * explicit software decision in this version.
         */
        cmpssRegs->COMPSTSCLR.bit.HSYNCCLREN = 0U;
        cmpssRegs->COMPSTSCLR.bit.LSYNCCLREN = 0U;

        cmpssRegs->COMPSTSCLR.bit.HLATCHCLR =
            MCAL_CMPSS_LATCH_CLEAR;
        cmpssRegs->COMPSTSCLR.bit.LLATCHCLR =
            MCAL_CMPSS_LATCH_CLEAR;

        EDIS;

        cmpssRegs->DACHVALS.bit.DACVAL = 0U;
        cmpssRegs->DACLVALS.bit.DACVAL = 0U;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_CmpssStatusType Mcal_Cmpss_InitComparator(
    const Mcal_CmpssCmpConfigType * config)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsCmpConfigValid(config);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        cmpssRegs = GetCmpssRegs(config->module);

        EALLOW;
        ConfigureComparator(cmpssRegs, config);
        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_CmpssStatusType Mcal_Cmpss_InitDac(
    const Mcal_CmpssDacConfigType * config)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsDacConfigValid(config);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        cmpssRegs = GetCmpssRegs(config->module);

        EALLOW;

        cmpssRegs->COMPDACCTL.bit.DACSOURCE =
            MCAL_CMPSS_DAC_SOURCE_SHADOW;
        cmpssRegs->COMPDACCTL.bit.SELREF =
            (uint16_t)config->reference;
        cmpssRegs->COMPDACCTL.bit.SWLOADSEL =
            MCAL_CMPSS_DAC_LOAD_SYSCLK;

        EDIS;

        cmpssRegs->DACHVALS.bit.DACVAL =
            config->highValue;
        cmpssRegs->DACLVALS.bit.DACVAL =
            config->lowValue;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_CmpssStatusType Mcal_Cmpss_InitFilter(
    const Mcal_CmpssFilterConfigType * config)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsFilterConfigValid(config);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        cmpssRegs = GetCmpssRegs(config->module);

        EALLOW;
        ConfigureFilter(cmpssRegs, config);
        ClearLatch(cmpssRegs, config->comparator);
        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_CmpssStatusType Mcal_Cmpss_SetHysteresis(
    Mcal_CmpssIdType module,
    uint16_t multiplier)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsModuleValid(module);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        status = IsHysteresisValid(multiplier);

        if(status == MCAL_CMPSS_STATUS_OK)
        {
            cmpssRegs = GetCmpssRegs(module);

            EALLOW;
            cmpssRegs->COMPHYSCTL.bit.COMPHYS = multiplier;
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

Mcal_CmpssStatusType Mcal_Cmpss_Enable(
    Mcal_CmpssIdType module)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsModuleValid(module);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        cmpssRegs = GetCmpssRegs(module);

        EALLOW;

        /*
         * Do not arm the module with a fault indication left from an earlier
         * configuration or run.
         */
        cmpssRegs->COMPSTSCLR.bit.HLATCHCLR =
            MCAL_CMPSS_LATCH_CLEAR;
        cmpssRegs->COMPSTSCLR.bit.LLATCHCLR =
            MCAL_CMPSS_LATCH_CLEAR;

        cmpssRegs->COMPCTL.bit.COMPDACE =
            MCAL_CMPSS_MODULE_ENABLE;

        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_CmpssStatusType Mcal_Cmpss_Disable(
    Mcal_CmpssIdType module)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsModuleValid(module);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        cmpssRegs = GetCmpssRegs(module);

        EALLOW;
        cmpssRegs->COMPCTL.bit.COMPDACE =
            MCAL_CMPSS_MODULE_DISABLE;
        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_CmpssStatusType Mcal_Cmpss_SetDacValue(
    Mcal_CmpssIdType module,
    Mcal_CmpssComparatorType comparator,
    uint16_t value)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsModuleValid(module);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        status = IsComparatorValid(comparator);

        if(status == MCAL_CMPSS_STATUS_OK)
        {
            status = IsDacValueValid(value);

            if(status == MCAL_CMPSS_STATUS_OK)
            {
                cmpssRegs = GetCmpssRegs(module);

                if(comparator == MCAL_CMPSS_HIGH)
                {
                    cmpssRegs->DACHVALS.bit.DACVAL = value;
                }
                else
                {
                    cmpssRegs->DACLVALS.bit.DACVAL = value;
                }
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
        /* Do nothing. */
    }

    return status;
}

Mcal_CmpssStatusType Mcal_Cmpss_IsFilterActive(
    Mcal_CmpssIdType module,
    Mcal_CmpssComparatorType comparator,
    uint16_t * active)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsModuleValid(module);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        status = IsComparatorValid(comparator);

        if(status == MCAL_CMPSS_STATUS_OK)
        {
            if(active != NULL)
            {
                cmpssRegs = GetCmpssRegs(module);
                *active = ReadFilterStatus(cmpssRegs, comparator);
            }
            else
            {
                status = MCAL_CMPSS_STATUS_INV_ARG;
            }
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

Mcal_CmpssStatusType Mcal_Cmpss_IsLatchActive(
    Mcal_CmpssIdType module,
    Mcal_CmpssComparatorType comparator,
    uint16_t * active)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsModuleValid(module);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        status = IsComparatorValid(comparator);

        if(status == MCAL_CMPSS_STATUS_OK)
        {
            if(active != NULL)
            {
                cmpssRegs = GetCmpssRegs(module);
                *active = ReadLatchStatus(cmpssRegs, comparator);
            }
            else
            {
                status = MCAL_CMPSS_STATUS_INV_ARG;
            }
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

Mcal_CmpssStatusType Mcal_Cmpss_ClearLatch(
    Mcal_CmpssIdType module,
    Mcal_CmpssComparatorType comparator)
{
    Mcal_CmpssStatusType status;
    volatile struct CMPSS_REGS * cmpssRegs;

    status = IsModuleValid(module);

    if(status == MCAL_CMPSS_STATUS_OK)
    {
        status = IsComparatorValid(comparator);

        if(status == MCAL_CMPSS_STATUS_OK)
        {
            cmpssRegs = GetCmpssRegs(module);

            EALLOW;
            ClearLatch(cmpssRegs, comparator);
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

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

static volatile struct CMPSS_REGS * GetCmpssRegs(
    Mcal_CmpssIdType module)
{
    volatile struct CMPSS_REGS * cmpssRegs;

    cmpssRegs = NULL;

    switch(module)
    {
        case MCAL_CMPSS_1:
            cmpssRegs = &Cmpss1Regs;
            break;

        case MCAL_CMPSS_2:
            cmpssRegs = &Cmpss2Regs;
            break;

        case MCAL_CMPSS_3:
            cmpssRegs = &Cmpss3Regs;
            break;

        case MCAL_CMPSS_4:
            cmpssRegs = &Cmpss4Regs;
            break;

        case MCAL_CMPSS_5:
            cmpssRegs = &Cmpss5Regs;
            break;

        case MCAL_CMPSS_6:
            cmpssRegs = &Cmpss6Regs;
            break;

        case MCAL_CMPSS_7:
            cmpssRegs = &Cmpss7Regs;
            break;

        case MCAL_CMPSS_8:
            cmpssRegs = &Cmpss8Regs;
            break;

        default:
            /* Do nothing. */
            break;
    }

    return cmpssRegs;
}

static Mcal_CmpssStatusType IsModuleValid(
    Mcal_CmpssIdType module)
{
    Mcal_CmpssStatusType status;

    switch(module)
    {
        case MCAL_CMPSS_1:
        case MCAL_CMPSS_2:
        case MCAL_CMPSS_3:
        case MCAL_CMPSS_4:
        case MCAL_CMPSS_5:
        case MCAL_CMPSS_6:
        case MCAL_CMPSS_7:
        case MCAL_CMPSS_8:
            status = MCAL_CMPSS_STATUS_OK;
            break;

        default:
            status = MCAL_CMPSS_STATUS_INV_ID;
            break;
    }

    return status;
}

static Mcal_CmpssStatusType IsComparatorValid(
    Mcal_CmpssComparatorType comparator)
{
    Mcal_CmpssStatusType status;

    if((comparator == MCAL_CMPSS_HIGH) ||
       (comparator == MCAL_CMPSS_LOW))
    {
        status = MCAL_CMPSS_STATUS_OK;
    }
    else
    {
        status = MCAL_CMPSS_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_CmpssStatusType IsInputSourceValid(
    Mcal_CmpssInputSourceType inputSource)
{
    Mcal_CmpssStatusType status;

    if((inputSource == MCAL_CMPSS_INPUT_INTERNAL_DAC) ||
       (inputSource == MCAL_CMPSS_INPUT_EXTERNAL_PIN))
    {
        status = MCAL_CMPSS_STATUS_OK;
    }
    else
    {
        status = MCAL_CMPSS_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_CmpssStatusType IsPolarityValid(
    Mcal_CmpssPolarityType polarity)
{
    Mcal_CmpssStatusType status;

    if((polarity == MCAL_CMPSS_POLARITY_NORMAL) ||
       (polarity == MCAL_CMPSS_POLARITY_INVERTED))
    {
        status = MCAL_CMPSS_STATUS_OK;
    }
    else
    {
        status = MCAL_CMPSS_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_CmpssStatusType IsTripSourceValid(
    Mcal_CmpssTripSourceType tripSource)
{
    Mcal_CmpssStatusType status;

    switch(tripSource)
    {
        case MCAL_CMPSS_TRIP_ASYNC:
        case MCAL_CMPSS_TRIP_SYNC:
        case MCAL_CMPSS_TRIP_FILTER:
        case MCAL_CMPSS_TRIP_LATCH:
            status = MCAL_CMPSS_STATUS_OK;
            break;

        default:
            status = MCAL_CMPSS_STATUS_INV_ARG;
            break;
    }

    return status;
}

static Mcal_CmpssStatusType IsDacRefValid(
    Mcal_CmpssDacRefType reference)
{
    Mcal_CmpssStatusType status;

    if((reference == MCAL_CMPSS_DAC_REF_VDDA) ||
       (reference == MCAL_CMPSS_DAC_REF_VDAC))
    {
        status = MCAL_CMPSS_STATUS_OK;
    }
    else
    {
        status = MCAL_CMPSS_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_CmpssStatusType IsCmpConfigValid(
    const Mcal_CmpssCmpConfigType * config)
{
    Mcal_CmpssStatusType status;

    if(config != NULL)
    {
        status = IsModuleValid(config->module);

        if(status == MCAL_CMPSS_STATUS_OK)
        {
            status = IsComparatorValid(config->comparator);

            if(status == MCAL_CMPSS_STATUS_OK)
            {
                status = IsInputSourceValid(config->inputSource);

                if(status == MCAL_CMPSS_STATUS_OK)
                {
                    status = IsPolarityValid(config->polarity);

                    if(status == MCAL_CMPSS_STATUS_OK)
                    {
                        status = IsTripSourceValid(config->tripSource);
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
        status = MCAL_CMPSS_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_CmpssStatusType IsDacConfigValid(
    const Mcal_CmpssDacConfigType * config)
{
    Mcal_CmpssStatusType status;

    if(config != NULL)
    {
        status = IsModuleValid(config->module);

        if(status == MCAL_CMPSS_STATUS_OK)
        {
            status = IsDacRefValid(config->reference);

            if(status == MCAL_CMPSS_STATUS_OK)
            {
                status = IsDacValueValid(config->highValue);

                if(status == MCAL_CMPSS_STATUS_OK)
                {
                    status = IsDacValueValid(config->lowValue);
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
            /* Do nothing. */
        }
    }
    else
    {
        status = MCAL_CMPSS_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_CmpssStatusType IsFilterConfigValid(
    const Mcal_CmpssFilterConfigType * config)
{
    Mcal_CmpssStatusType status;

    if(config != NULL)
    {
        status = IsModuleValid(config->module);

        if(status == MCAL_CMPSS_STATUS_OK)
        {
            status = IsComparatorValid(config->comparator);

            if(status == MCAL_CMPSS_STATUS_OK)
            {
                if((config->samplePeriodCycles >=
                    MCAL_CMPSS_SAMPLE_CYCLES_MIN) &&
                   (config->samplePeriodCycles <=
                    MCAL_CMPSS_SAMPLE_CYCLES_MAX) &&
                   (config->sampleWindow >=
                    MCAL_CMPSS_SAMPLE_WINDOW_MIN) &&
                   (config->sampleWindow <=
                    MCAL_CMPSS_SAMPLE_WINDOW_MAX) &&
                   (config->threshold <= config->sampleWindow) &&
                   (config->threshold > (config->sampleWindow / 2U)))
                {
                    status = MCAL_CMPSS_STATUS_OK;
                }
                else
                {
                    status = MCAL_CMPSS_STATUS_INV_ARG;
                }
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
        status = MCAL_CMPSS_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_CmpssStatusType IsDacValueValid(
    uint16_t value)
{
    Mcal_CmpssStatusType status;

    if(value <= MCAL_CMPSS_DAC_VALUE_MAX)
    {
        status = MCAL_CMPSS_STATUS_OK;
    }
    else
    {
        status = MCAL_CMPSS_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_CmpssStatusType IsHysteresisValid(
    uint16_t multiplier)
{
    Mcal_CmpssStatusType status;

    if(multiplier <= MCAL_CMPSS_HYSTERESIS_MAX)
    {
        status = MCAL_CMPSS_STATUS_OK;
    }
    else
    {
        status = MCAL_CMPSS_STATUS_INV_ARG;
    }

    return status;
}

static void ConfigureComparator(
    volatile struct CMPSS_REGS * cmpssRegs,
    const Mcal_CmpssCmpConfigType * config)
{
    if(config->comparator == MCAL_CMPSS_HIGH)
    {
        cmpssRegs->COMPCTL.bit.COMPHSOURCE =
            (uint16_t)config->inputSource;
        cmpssRegs->COMPCTL.bit.COMPHINV =
            (uint16_t)config->polarity;
        cmpssRegs->COMPCTL.bit.CTRIPHSEL =
            (uint16_t)config->tripSource;
        cmpssRegs->COMPCTL.bit.CTRIPOUTHSEL =
            (uint16_t)config->tripSource;
        cmpssRegs->COMPCTL.bit.ASYNCHEN =
            MCAL_CMPSS_ASYNC_OR_DISABLE;
    }
    else
    {
        cmpssRegs->COMPCTL.bit.COMPLSOURCE =
            (uint16_t)config->inputSource;
        cmpssRegs->COMPCTL.bit.COMPLINV =
            (uint16_t)config->polarity;
        cmpssRegs->COMPCTL.bit.CTRIPLSEL =
            (uint16_t)config->tripSource;
        cmpssRegs->COMPCTL.bit.CTRIPOUTLSEL =
            (uint16_t)config->tripSource;
        cmpssRegs->COMPCTL.bit.ASYNCLEN =
            MCAL_CMPSS_ASYNC_OR_DISABLE;
    }
}

static void ConfigureFilter(
    volatile struct CMPSS_REGS * cmpssRegs,
    const Mcal_CmpssFilterConfigType * config)
{
    uint16_t clkPrescale;
    uint16_t sampleWindow;
    uint16_t threshold;

    /*
     * Hardware encodes all three filter parameters as N - 1.
     */
    clkPrescale = config->samplePeriodCycles - 1U;
    sampleWindow = config->sampleWindow - 1U;
    threshold = config->threshold - 1U;

    if(config->comparator == MCAL_CMPSS_HIGH)
    {
        cmpssRegs->CTRIPHFILCLKCTL.bit.CLKPRESCALE =
            clkPrescale;
        cmpssRegs->CTRIPHFILCTL.bit.SAMPWIN =
            sampleWindow;
        cmpssRegs->CTRIPHFILCTL.bit.THRESH =
            threshold;

        /*
         * Initialize the filter FIFO to the current input value to avoid a
         * startup transient caused by stale sample history.
         */
        cmpssRegs->CTRIPHFILCTL.bit.FILINIT = 1U;
    }
    else
    {
        cmpssRegs->CTRIPLFILCLKCTL.bit.CLKPRESCALE =
            clkPrescale;
        cmpssRegs->CTRIPLFILCTL.bit.SAMPWIN =
            sampleWindow;
        cmpssRegs->CTRIPLFILCTL.bit.THRESH =
            threshold;

        cmpssRegs->CTRIPLFILCTL.bit.FILINIT = 1U;
    }
}

static uint16_t ReadFilterStatus(
    volatile struct CMPSS_REGS * cmpssRegs,
    Mcal_CmpssComparatorType comparator)
{
    uint16_t status;

    if(comparator == MCAL_CMPSS_HIGH)
    {
        if(cmpssRegs->COMPSTS.bit.COMPHSTS != 0U)
        {
            status = MCAL_CMPSS_FLAG_SET;
        }
        else
        {
            status = MCAL_CMPSS_FLAG_RESET;
        }
    }
    else
    {
        if(cmpssRegs->COMPSTS.bit.COMPLSTS != 0U)
        {
            status = MCAL_CMPSS_FLAG_SET;
        }
        else
        {
            status = MCAL_CMPSS_FLAG_RESET;
        }
    }

    return status;
}

static uint16_t ReadLatchStatus(
    volatile struct CMPSS_REGS * cmpssRegs,
    Mcal_CmpssComparatorType comparator)
{
    uint16_t status;

    if(comparator == MCAL_CMPSS_HIGH)
    {
        if(cmpssRegs->COMPSTS.bit.COMPHLATCH != 0U)
        {
            status = MCAL_CMPSS_FLAG_SET;
        }
        else
        {
            status = MCAL_CMPSS_FLAG_RESET;
        }
    }
    else
    {
        if(cmpssRegs->COMPSTS.bit.COMPLLATCH != 0U)
        {
            status = MCAL_CMPSS_FLAG_SET;
        }
        else
        {
            status = MCAL_CMPSS_FLAG_RESET;
        }
    }

    return status;
}

static void ClearLatch(
    volatile struct CMPSS_REGS * cmpssRegs,
    Mcal_CmpssComparatorType comparator)
{
    if(comparator == MCAL_CMPSS_HIGH)
    {
        cmpssRegs->COMPSTSCLR.bit.HLATCHCLR =
            MCAL_CMPSS_LATCH_CLEAR;
    }
    else
    {
        cmpssRegs->COMPSTSCLR.bit.LLATCHCLR =
            MCAL_CMPSS_LATCH_CLEAR;
    }
}

