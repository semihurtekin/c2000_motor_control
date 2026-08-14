/**
 * @file    mcal_cpu_int.h
 * @brief   F28379D CPU Interrupt MCAL driver interface.
 */

#ifndef MCAL_CPU_INT_H
#define MCAL_CPU_INT_H

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
    MCAL_CPU_INT_1 = 1U,
    MCAL_CPU_INT_2 = 2U,
    MCAL_CPU_INT_3 = 3U,
    MCAL_CPU_INT_4 = 4U,
    MCAL_CPU_INT_5 = 5U,
    MCAL_CPU_INT_6 = 6U,
    MCAL_CPU_INT_7 = 7U,
    MCAL_CPU_INT_8 = 8U,
    MCAL_CPU_INT_9 = 9U,
    MCAL_CPU_INT_10 = 10U,
    MCAL_CPU_INT_11 = 11U,
    MCAL_CPU_INT_12 = 12U,
    MCAL_CPU_INT_13 = 13U,
    MCAL_CPU_INT_14 = 14U
} Mcal_CpuIntType;

typedef enum
{
    MCAL_CPU_INT_STATUS_OK = 0U,
    MCAL_CPU_INT_STATUS_INV_INT = 1U
} Mcal_CpuIntStatusType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes the CPU interrupt controller.
 *
 * Global maskable interrupts are disabled and the CPU interrupt enable and
 * pending flag registers are cleared.
 *
 * @return CPU interrupt driver status.
 */
Mcal_CpuIntStatusType Mcal_CpuInt_Init(void);

/**
 * @brief Enables the selected CPU interrupt.
 *
 * @param intType CPU interrupt identifier.
 *
 * @return CPU interrupt driver status.
 */
Mcal_CpuIntStatusType Mcal_CpuInt_Enable(
    Mcal_CpuIntType intType);

/**
 * @brief Disables the selected CPU interrupt.
 *
 * @param intType CPU interrupt identifier.
 *
 * @return CPU interrupt driver status.
 */
Mcal_CpuIntStatusType Mcal_CpuInt_Disable(
    Mcal_CpuIntType intType);

/**
 * @brief Enables global maskable CPU interrupts.
 */
void Mcal_CpuInt_EnableGlobal(void);

/**
 * @brief Disables global maskable CPU interrupts.
 */
void Mcal_CpuInt_DisableGlobal(void);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_CPU_INT_H */
