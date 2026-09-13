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
    MCAL_SPI_STATUS_INV_ID,
    MCAL_SPI_STATUS_INV_ARG,
    MCAL_SPI_STATUS_NOT_INITIALIZED,
    MCAL_SPI_STATUS_TIMEOUT
} Mcal_SpiStatusType;

typedef struct
{
    Mcal_SpiIdType module;
    Mcal_SpiModeType mode;
    uint32_t sourceClockHz;
    uint32_t bitRateHz;
    Mcal_SpiDataWidthType dataWidth;
} Mcal_SpiConfigType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes an SPI module as controller.
 *
 * The peripheral clock shall already be enabled by the platform/BSP layer
 * before this service is called.
 *
 * @param[in] config SPI configuration.
 *
 * @return Driver status.
 */
Mcal_SpiStatusType Mcal_Spi_Init(
    const Mcal_SpiConfigType * config);

/**
 * @brief Performs one blocking full-duplex SPI transfer.
 *
 * The wait for TX FIFO space and RX data is bounded. If the SPI hardware does
 * not make progress within the internal polling limit, the function returns
 * MCAL_SPI_STATUS_TIMEOUT.
 *
 * @param[in]  module Selected SPI module.
 * @param[in]  txData Data to transmit.
 * @param[out] rxData Receives the simultaneously received data.
 *
 * @return Driver status.
 */
Mcal_SpiStatusType Mcal_Spi_TransferWord(
    Mcal_SpiIdType module,
    uint16_t txData,
    uint16_t * rxData);

/**
 * @brief Performs multiple blocking full-duplex SPI transfers.
 *
 * @param[in]  module Selected SPI module.
 * @param[in]  txData Transmit buffer.
 * @param[out] rxData Receive buffer.
 * @param[in]  length Number of configured-width words to transfer.
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
