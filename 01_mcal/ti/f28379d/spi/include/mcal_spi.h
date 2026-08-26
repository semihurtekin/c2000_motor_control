/**
 * @file    mcal_spi.h
 * @brief   F28379D SPI MCAL driver interface.
 */

#ifndef MCAL_SPI_H
#define MCAL_SPI_H

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
    MCAL_SPI_A = 0U,
    MCAL_SPI_B = 1U,
    MCAL_SPI_C = 2U
} Mcal_SpiIdType;

typedef enum
{
    MCAL_SPI_MODE_0 = 0U,
    MCAL_SPI_MODE_1 = 1U,
    MCAL_SPI_MODE_2 = 2U,
    MCAL_SPI_MODE_3 = 3U
} Mcal_SpiModeType;

typedef enum
{
    MCAL_SPI_WIDTH_8 = 8U,
    MCAL_SPI_WIDTH_16 = 16U
} Mcal_SpiDataWidthType;

typedef enum
{
    MCAL_SPI_STATUS_OK = 0U,
    MCAL_SPI_STATUS_INV_ID = 1U,
    MCAL_SPI_STATUS_INV_ARG = 2U
} Mcal_SpiStatusType;

typedef struct
{
    Mcal_SpiIdType module;
    Mcal_SpiModeType mode;
    uint32_t bitRateHz;
    Mcal_SpiDataWidthType dataWidth;
} Mcal_SpiConfigType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes an SPI module as controller.
 *
 * @param config SPI configuration.
 *
 * @return Driver status.
 */
Mcal_SpiStatusType Mcal_Spi_Init(
    const Mcal_SpiConfigType * config);

/**
 * @brief Performs one full-duplex SPI transfer.
 *
 * @param module Selected SPI module.
 * @param txData Data to transmit.
 * @param rxData Receives the simultaneously received data.
 *
 * @return Driver status.
 */
Mcal_SpiStatusType Mcal_Spi_TransferWord(
    Mcal_SpiIdType module,
    uint16_t txData,
    uint16_t * rxData);

/**
 * @brief Performs multiple full-duplex SPI transfers.
 *
 * @param module Selected SPI module.
 * @param txData Transmit buffer.
 * @param rxData Receive buffer.
 * @param length Number of configured-width words to transfer.
 *
 * @return Driver status.
 */
Mcal_SpiStatusType Mcal_Spi_Transfer(
    Mcal_SpiIdType module,
    const uint16_t * txData,
    uint16_t * rxData,
    uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_SPI_H */
