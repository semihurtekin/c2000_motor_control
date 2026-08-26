/**
 * @file    mcal_sci.h
 * @brief   F28379D SCI MCAL driver interface.
 */

#ifndef MCAL_SCI_H
#define MCAL_SCI_H

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
    MCAL_SCI_A = 0U,
    MCAL_SCI_B = 1U,
    MCAL_SCI_C = 2U,
    MCAL_SCI_D = 3U
} Mcal_SciIdType;

typedef enum
{
    MCAL_SCI_STATUS_OK = 0U,
    MCAL_SCI_STATUS_INV_ID = 1U,
    MCAL_SCI_STATUS_INV_ARG = 2U,
    MCAL_SCI_STATUS_NO_DATA = 3U
} Mcal_SciStatusType;

typedef struct
{
    Mcal_SciIdType module;
    uint32_t baudRate;
} Mcal_SciConfigType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes an SCI module for 8-N-1 operation.
 *
 * @param config SCI configuration.
 *
 * @return Driver status.
 */
Mcal_SciStatusType Mcal_Sci_Init(
    const Mcal_SciConfigType * config);

/**
 * @brief Writes one 8-bit character using the TX FIFO.
 *
 * @param module Selected SCI module.
 * @param data Character to transmit.
 *
 * @return Driver status.
 */
Mcal_SciStatusType Mcal_Sci_WriteByte(
    Mcal_SciIdType module,
    uint16_t data);

/**
 * @brief Writes a character buffer using the TX FIFO.
 *
 * @param module Selected SCI module.
 * @param data Character buffer.
 * @param length Number of characters to transmit.
 *
 * @return Driver status.
 */
Mcal_SciStatusType Mcal_Sci_Write(
    Mcal_SciIdType module,
    const uint16_t * data,
    uint16_t length);

/**
 * @brief Checks whether at least one received character is available.
 *
 * @param module Selected SCI module.
 * @param ready Receives 1U when RX data is available, otherwise 0U.
 *
 * @return Driver status.
 */
Mcal_SciStatusType Mcal_Sci_IsRxReady(
    Mcal_SciIdType module,
    uint16_t * ready);

/**
 * @brief Reads one received character without blocking.
 *
 * @param module Selected SCI module.
 * @param data Receives the low eight bits of the received character.
 *
 * @return MCAL_SCI_STATUS_NO_DATA when the RX FIFO is empty.
 */
Mcal_SciStatusType Mcal_Sci_ReadByte(
    Mcal_SciIdType module,
    uint16_t * data);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_SCI_H */
