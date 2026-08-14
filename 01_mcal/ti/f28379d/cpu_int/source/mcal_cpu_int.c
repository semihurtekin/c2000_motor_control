/**
 * @file    mcal_cpu_int.c
 * @brief   F28379D CPU Interrupt MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

#include "mcal_cpu_int.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

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
 * @brief Checks whether the given CPU interrupt identifier is valid.
 */
static Mcal_CpuIntStatusType IsIntTypeValid(
    Mcal_CpuIntType intType);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_CpuIntStatusType Mcal_CpuInt_Init(void)
{
    DINT;

    IER = 0x0000U;
    IFR = 0x0000U;

    return MCAL_CPU_INT_STATUS_OK;
}

Mcal_CpuIntStatusType Mcal_CpuInt_Enable(
    Mcal_CpuIntType intType)
{
    Mcal_CpuIntStatusType status;
    uint16_t mask;

    status = IsIntTypeValid(intType);

    if(status == MCAL_CPU_INT_STATUS_OK)
    {
        mask = (uint16_t)((uint16_t)1U <<
            ((uint16_t)intType - 1U));

        IER |= mask;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_CpuIntStatusType Mcal_CpuInt_Disable(
    Mcal_CpuIntType intType)
{
    Mcal_CpuIntStatusType status;
    uint16_t mask;

    status = IsIntTypeValid(intType);

    if(status == MCAL_CPU_INT_STATUS_OK)
    {
        mask = (uint16_t)((uint16_t)1U <<
            ((uint16_t)intType - 1U));

        IER &= (uint16_t)(~mask);
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

void Mcal_CpuInt_EnableGlobal(void)
{
    EINT;
}

void Mcal_CpuInt_DisableGlobal(void)
{
    DINT;
}

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

static Mcal_CpuIntStatusType IsIntTypeValid(
    Mcal_CpuIntType intType)
{
    Mcal_CpuIntStatusType status;

    if(((uint16_t)intType >= (uint16_t)MCAL_CPU_INT_1) &&
       ((uint16_t)intType <= (uint16_t)MCAL_CPU_INT_14))
    {
        status = MCAL_CPU_INT_STATUS_OK;
    }
    else
    {
        status = MCAL_CPU_INT_STATUS_INV_INT;
    }

    return status;
}
