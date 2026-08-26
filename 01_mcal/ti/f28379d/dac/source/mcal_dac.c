/**
 * @file    mcal_dac.c
 * @brief   F28379D buffered DAC MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stddef.h>

#include "mcal_dac.h"
#include "F2837xD_device.h"
#include "F2837xD_Examples.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_DAC_LOAD_SYSCLK         (0U)
#define MCAL_DAC_OUTPUT_DISABLE      (0U)
#define MCAL_DAC_OUTPUT_ENABLE       (1U)
#define MCAL_DAC_VALUE_MAX           (4095U)
#define MCAL_DAC_POWER_UP_US         (500.0L)

/*==============================================================================
 * Private Types
 *============================================================================*/

/*==============================================================================
 * Private Variables
 *============================================================================*/

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static volatile struct DAC_REGS * GetDacRegs(
    Mcal_DacIdType module);

static Mcal_DacStatusType IsModuleValid(
    Mcal_DacIdType module);

static Mcal_DacStatusType IsReferenceValid(
    Mcal_DacRefType reference);

static Mcal_DacStatusType IsConfigValid(
    const Mcal_DacConfigType * config);

static void EnablePeripheralClock(
    Mcal_DacIdType module);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_DacStatusType Mcal_Dac_Init(
    const Mcal_DacConfigType * config)
{
    Mcal_DacStatusType status;
    volatile struct DAC_REGS * dacRegs;

    status = IsConfigValid(config);

    if(status == MCAL_DAC_STATUS_OK)
    {
        EnablePeripheralClock(config->module);
        dacRegs = GetDacRegs(config->module);

        EALLOW;

        dacRegs->DACOUTEN.bit.DACOUTEN =
            MCAL_DAC_OUTPUT_DISABLE;

        dacRegs->DACCTL.bit.DACREFSEL =
            (uint16_t)config->reference;

        dacRegs->DACCTL.bit.LOADMODE =
            MCAL_DAC_LOAD_SYSCLK;

        dacRegs->DACVALS.bit.DACVALS = 0U;

        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_DacStatusType Mcal_Dac_Enable(
    Mcal_DacIdType module)
{
    Mcal_DacStatusType status;
    volatile struct DAC_REGS * dacRegs;

    status = IsModuleValid(module);

    if(status == MCAL_DAC_STATUS_OK)
    {
        dacRegs = GetDacRegs(module);

        EALLOW;
        dacRegs->DACOUTEN.bit.DACOUTEN =
            MCAL_DAC_OUTPUT_ENABLE;
        EDIS;

        DELAY_US(MCAL_DAC_POWER_UP_US);
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_DacStatusType Mcal_Dac_Disable(
    Mcal_DacIdType module)
{
    Mcal_DacStatusType status;
    volatile struct DAC_REGS * dacRegs;

    status = IsModuleValid(module);

    if(status == MCAL_DAC_STATUS_OK)
    {
        dacRegs = GetDacRegs(module);

        EALLOW;
        dacRegs->DACOUTEN.bit.DACOUTEN =
            MCAL_DAC_OUTPUT_DISABLE;
        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_DacStatusType Mcal_Dac_SetValue(
    Mcal_DacIdType module,
    uint16_t value)
{
    Mcal_DacStatusType status;
    volatile struct DAC_REGS * dacRegs;

    status = IsModuleValid(module);

    if(status == MCAL_DAC_STATUS_OK)
    {
        if(value <= MCAL_DAC_VALUE_MAX)
        {
            dacRegs = GetDacRegs(module);
            dacRegs->DACVALS.bit.DACVALS = value;
        }
        else
        {
            status = MCAL_DAC_STATUS_INV_ARG;
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

static volatile struct DAC_REGS * GetDacRegs(
    Mcal_DacIdType module)
{
    volatile struct DAC_REGS * dacRegs;

    dacRegs = NULL;

    switch(module)
    {
        case MCAL_DAC_A:
            dacRegs = &DacaRegs;
            break;

        case MCAL_DAC_B:
            dacRegs = &DacbRegs;
            break;

        case MCAL_DAC_C:
            dacRegs = &DaccRegs;
            break;

        default:
            /* Do nothing. */
            break;
    }

    return dacRegs;
}

static Mcal_DacStatusType IsModuleValid(
    Mcal_DacIdType module)
{
    Mcal_DacStatusType status;

    if((module == MCAL_DAC_A) ||
       (module == MCAL_DAC_B) ||
       (module == MCAL_DAC_C))
    {
        status = MCAL_DAC_STATUS_OK;
    }
    else
    {
        status = MCAL_DAC_STATUS_INV_ID;
    }

    return status;
}

static Mcal_DacStatusType IsReferenceValid(
    Mcal_DacRefType reference)
{
    Mcal_DacStatusType status;

    if((reference == MCAL_DAC_REF_VDAC) ||
       (reference == MCAL_DAC_REF_VREFHI))
    {
        status = MCAL_DAC_STATUS_OK;
    }
    else
    {
        status = MCAL_DAC_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_DacStatusType IsConfigValid(
    const Mcal_DacConfigType * config)
{
    Mcal_DacStatusType status;

    if(config != NULL)
    {
        status = IsModuleValid(config->module);

        if(status == MCAL_DAC_STATUS_OK)
        {
            status = IsReferenceValid(config->reference);
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        status = MCAL_DAC_STATUS_INV_ARG;
    }

    return status;
}

static void EnablePeripheralClock(
    Mcal_DacIdType module)
{
    EALLOW;

    switch(module)
    {
        case MCAL_DAC_A:
            CpuSysRegs.PCLKCR16.bit.DAC_A = 1U;
            break;

        case MCAL_DAC_B:
            CpuSysRegs.PCLKCR16.bit.DAC_B = 1U;
            break;

        case MCAL_DAC_C:
            CpuSysRegs.PCLKCR16.bit.DAC_C = 1U;
            break;

        default:
            /* Do nothing. */
            break;
    }

    EDIS;
}
