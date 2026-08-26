/**
 * @file    mcal_dac.h
 * @brief   F28379D buffered DAC MCAL driver interface.
 */

#ifndef MCAL_DAC_H
#define MCAL_DAC_H

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
    MCAL_DAC_A = 0U,
    MCAL_DAC_B = 1U,
    MCAL_DAC_C = 2U
} Mcal_DacIdType;

typedef enum
{
    MCAL_DAC_REF_VDAC = 0U,
    MCAL_DAC_REF_VREFHI = 1U
} Mcal_DacRefType;

typedef enum
{
    MCAL_DAC_STATUS_OK = 0U,
    MCAL_DAC_STATUS_INV_ID = 1U,
    MCAL_DAC_STATUS_INV_ARG = 2U
} Mcal_DacStatusType;

typedef struct
{
    Mcal_DacIdType module;
    Mcal_DacRefType reference;
} Mcal_DacConfigType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes a buffered DAC module.
 *
 * The DAC uses SYSCLK loading and starts with output disabled.
 *
 * @param config DAC configuration.
 *
 * @return Driver status.
 */
Mcal_DacStatusType Mcal_Dac_Init(
    const Mcal_DacConfigType * config);

/**
 * @brief Enables the selected DAC output.
 *
 * @param module Selected DAC module.
 *
 * @return Driver status.
 */
Mcal_DacStatusType Mcal_Dac_Enable(
    Mcal_DacIdType module);

/**
 * @brief Disables the selected DAC output.
 *
 * @param module Selected DAC module.
 *
 * @return Driver status.
 */
Mcal_DacStatusType Mcal_Dac_Disable(
    Mcal_DacIdType module);

/**
 * @brief Writes a 12-bit DAC output code.
 *
 * @param module Selected DAC module.
 * @param value DAC code in the range 0 through 4095.
 *
 * @return Driver status.
 */
Mcal_DacStatusType Mcal_Dac_SetValue(
    Mcal_DacIdType module,
    uint16_t value);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_DAC_H */
