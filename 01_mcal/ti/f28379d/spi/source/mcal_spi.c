/**
 * @file    mcal_spi.c
 * @brief   F28379D SPI MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stddef.h>

#include "mcal_spi.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_SPI_MODULE_COUNT              (3U)
#define MCAL_SPI_BRR_MIN                   (3U)
#define MCAL_SPI_BRR_MAX                   (127U)
#define MCAL_SPI_DIVISOR_MIN               (4UL)
#define MCAL_SPI_DIVISOR_MAX               (128UL)
#define MCAL_SPI_POLL_LIMIT                (100000UL)

#define MCAL_SPI_FIFO_DEPTH                (16U)
#define MCAL_SPI_FIFO_ENABLE               (1U)
#define MCAL_SPI_FIFO_RESET_HOLD           (0U)
#define MCAL_SPI_FIFO_RESET_RELEASE        (1U)
#define MCAL_SPI_FIFO_INT_DISABLE          (0U)
#define MCAL_SPI_FIFO_FLAG_CLEAR           (1U)
#define MCAL_SPI_FIFO_OVERFLOW_CLEAR       (1U)

#define MCAL_SPI_SW_RESET_HOLD             (0U)
#define MCAL_SPI_SW_RESET_RELEASE          (1U)
#define MCAL_SPI_LOOPBACK_DISABLE          (0U)
#define MCAL_SPI_HIGH_SPEED_DISABLE        (0U)
#define MCAL_SPI_INT_DISABLE               (0U)
#define MCAL_SPI_TRANSMIT_ENABLE           (1U)
#define MCAL_SPI_CONTROLLER_MODE           (1U)
#define MCAL_SPI_OVERRUN_INT_DISABLE       (0U)
#define MCAL_SPI_THREE_WIRE_DISABLE        (0U)

#define MCAL_SPI_MODE_CPOL_MASK            (2U)
#define MCAL_SPI_MODE_CPHA_MASK            (1U)

/*==============================================================================
 * Private Variables
 *============================================================================*/

static uint16_t DataWidthBits[MCAL_SPI_MODULE_COUNT] =
{
    0U,
    0U,
    0U
};

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static volatile struct SPI_REGS * GetSpiRegs(
    Mcal_SpiIdType module);

static Mcal_SpiStatusType IsModuleValid(
    Mcal_SpiIdType module);

static Mcal_SpiStatusType IsModeValid(
    Mcal_SpiModeType mode);

static Mcal_SpiStatusType IsWidthValid(
    Mcal_SpiDataWidthType dataWidth);

static Mcal_SpiStatusType IsBaudConfigValid(
    uint32_t sourceClockHz,
    uint32_t bitRateHz);

static Mcal_SpiStatusType IsConfigValid(
    const Mcal_SpiConfigType * config);

static uint16_t CalculateBaudRegister(
    uint32_t sourceClockHz,
    uint32_t bitRateHz);

static Mcal_SpiStatusType WaitForTxSpace(
    volatile struct SPI_REGS * spiRegs);

static Mcal_SpiStatusType WaitForRxData(
    volatile struct SPI_REGS * spiRegs);

static void ResetFifos(
    volatile struct SPI_REGS * spiRegs);

static uint16_t GetTxShift(
    Mcal_SpiIdType module);

static uint16_t GetRxMask(
    Mcal_SpiIdType module);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_SpiStatusType Mcal_Spi_Init(
    const Mcal_SpiConfigType * config)
{
    Mcal_SpiStatusType status;
    volatile struct SPI_REGS * spiRegs;
    uint16_t baudRegister;
    uint16_t cpol;
    uint16_t cpha;

    status = IsConfigValid(config);

    if(status == MCAL_SPI_STATUS_OK)
    {
        spiRegs = GetSpiRegs(config->module);

        baudRegister =
            CalculateBaudRegister(
                config->sourceClockHz,
                config->bitRateHz);

        cpol =
            ((uint16_t)config->mode &
             MCAL_SPI_MODE_CPOL_MASK) >> 1U;

        cpha =
            (uint16_t)config->mode &
            MCAL_SPI_MODE_CPHA_MASK;

        spiRegs->SPICCR.bit.SPISWRESET =
            MCAL_SPI_SW_RESET_HOLD;

        spiRegs->SPICCR.bit.SPICHAR =
            (uint16_t)config->dataWidth - 1U;
        spiRegs->SPICCR.bit.SPILBK =
            MCAL_SPI_LOOPBACK_DISABLE;
        spiRegs->SPICCR.bit.HS_MODE =
            MCAL_SPI_HIGH_SPEED_DISABLE;
        spiRegs->SPICCR.bit.CLKPOLARITY =
            cpol;

        spiRegs->SPICTL.bit.SPIINTENA =
            MCAL_SPI_INT_DISABLE;
        spiRegs->SPICTL.bit.TALK =
            MCAL_SPI_TRANSMIT_ENABLE;
        spiRegs->SPICTL.bit.MASTER_SLAVE =
            MCAL_SPI_CONTROLLER_MODE;
        spiRegs->SPICTL.bit.CLK_PHASE =
            cpha;
        spiRegs->SPICTL.bit.OVERRUNINTENA =
            MCAL_SPI_OVERRUN_INT_DISABLE;

        spiRegs->SPIBRR.bit.SPI_BIT_RATE =
            baudRegister;

        spiRegs->SPIPRI.bit.TRIWIRE =
            MCAL_SPI_THREE_WIRE_DISABLE;

        spiRegs->SPIFFTX.bit.SPIFFENA =
            MCAL_SPI_FIFO_ENABLE;
        spiRegs->SPIFFTX.bit.SPIRST =
            MCAL_SPI_FIFO_RESET_RELEASE;
        spiRegs->SPIFFTX.bit.TXFFIENA =
            MCAL_SPI_FIFO_INT_DISABLE;
        spiRegs->SPIFFTX.bit.TXFFINTCLR =
            MCAL_SPI_FIFO_FLAG_CLEAR;

        spiRegs->SPIFFRX.bit.RXFFIENA =
            MCAL_SPI_FIFO_INT_DISABLE;
        spiRegs->SPIFFRX.bit.RXFFINTCLR =
            MCAL_SPI_FIFO_FLAG_CLEAR;
        spiRegs->SPIFFRX.bit.RXFFOVFCLR =
            MCAL_SPI_FIFO_OVERFLOW_CLEAR;

        spiRegs->SPIFFCT.all = 0U;

        ResetFifos(spiRegs);

        DataWidthBits[(uint16_t)config->module] =
            (uint16_t)config->dataWidth;

        spiRegs->SPICCR.bit.SPISWRESET =
            MCAL_SPI_SW_RESET_RELEASE;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_SpiStatusType Mcal_Spi_TransferWord(
    Mcal_SpiIdType module,
    uint16_t txData,
    uint16_t * rxData)
{
    Mcal_SpiStatusType status;
    volatile struct SPI_REGS * spiRegs;
    uint16_t shift;
    uint16_t mask;

    status = IsModuleValid(module);

    if(status == MCAL_SPI_STATUS_OK)
    {
        if(rxData != NULL)
        {
            if(DataWidthBits[(uint16_t)module] != 0U)
            {
                spiRegs = GetSpiRegs(module);
                shift = GetTxShift(module);
                mask = GetRxMask(module);

                if((txData & (uint16_t)(~mask)) == 0U)
                {
                    status = WaitForTxSpace(spiRegs);

                    if(status == MCAL_SPI_STATUS_OK)
                    {
                        spiRegs->SPITXBUF =
                            (uint16_t)(txData << shift);

                        status = WaitForRxData(spiRegs);

                        if(status == MCAL_SPI_STATUS_OK)
                        {
                            *rxData =
                                spiRegs->SPIRXBUF & mask;
                        }
                        else
                        {
                            ResetFifos(spiRegs);
                        }
                    }
                    else
                    {
                        ResetFifos(spiRegs);
                    }
                }
                else
                {
                    status = MCAL_SPI_STATUS_INV_ARG;
                }
            }
            else
            {
                status = MCAL_SPI_STATUS_NOT_INITIALIZED;
            }
        }
        else
        {
            status = MCAL_SPI_STATUS_INV_ARG;
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_SpiStatusType Mcal_Spi_Transfer(
    Mcal_SpiIdType module,
    const uint16_t * txData,
    uint16_t * rxData,
    uint16_t length)
{
    Mcal_SpiStatusType status;
    uint16_t index;

    status = IsModuleValid(module);

    if(status == MCAL_SPI_STATUS_OK)
    {
        if(length == 0U)
        {
            /* Zero-length transfer is valid. */
        }
        else if((txData != NULL) && (rxData != NULL))
        {
            index = 0U;

            while((index < length) &&
                  (status == MCAL_SPI_STATUS_OK))
            {
                status =
                    Mcal_Spi_TransferWord(
                        module,
                        txData[index],
                        &rxData[index]);

                if(status == MCAL_SPI_STATUS_OK)
                {
                    index++;
                }
                else
                {
                    /* Stop at the first failed word. */
                }
            }
        }
        else
        {
            status = MCAL_SPI_STATUS_INV_ARG;
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

static volatile struct SPI_REGS * GetSpiRegs(
    Mcal_SpiIdType module)
{
    volatile struct SPI_REGS * spiRegs;

    spiRegs = NULL;

    switch(module)
    {
        case MCAL_SPI_A:
            spiRegs = &SpiaRegs;
            break;

        case MCAL_SPI_B:
            spiRegs = &SpibRegs;
            break;

        case MCAL_SPI_C:
            spiRegs = &SpicRegs;
            break;

        default:
            /* Invalid module. */
            break;
    }

    return spiRegs;
}

static Mcal_SpiStatusType IsModuleValid(
    Mcal_SpiIdType module)
{
    Mcal_SpiStatusType status;

    if((module == MCAL_SPI_A) ||
       (module == MCAL_SPI_B) ||
       (module == MCAL_SPI_C))
    {
        status = MCAL_SPI_STATUS_OK;
    }
    else
    {
        status = MCAL_SPI_STATUS_INV_ID;
    }

    return status;
}

static Mcal_SpiStatusType IsModeValid(
    Mcal_SpiModeType mode)
{
    Mcal_SpiStatusType status;

    if((mode == MCAL_SPI_MODE_0) ||
       (mode == MCAL_SPI_MODE_1) ||
       (mode == MCAL_SPI_MODE_2) ||
       (mode == MCAL_SPI_MODE_3))
    {
        status = MCAL_SPI_STATUS_OK;
    }
    else
    {
        status = MCAL_SPI_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_SpiStatusType IsWidthValid(
    Mcal_SpiDataWidthType dataWidth)
{
    Mcal_SpiStatusType status;

    if((dataWidth == MCAL_SPI_WIDTH_8) ||
       (dataWidth == MCAL_SPI_WIDTH_16))
    {
        status = MCAL_SPI_STATUS_OK;
    }
    else
    {
        status = MCAL_SPI_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_SpiStatusType IsBaudConfigValid(
    uint32_t sourceClockHz,
    uint32_t bitRateHz)
{
    Mcal_SpiStatusType status;
    uint32_t divisor;

    status = MCAL_SPI_STATUS_INV_ARG;

    if((sourceClockHz != 0UL) &&
       (bitRateHz != 0UL))
    {
        divisor = sourceClockHz / bitRateHz;

        if((sourceClockHz % bitRateHz) != 0UL)
        {
            divisor++;
        }
        else
        {
            /* Exact integer divider. */
        }

        if((divisor >= MCAL_SPI_DIVISOR_MIN) &&
           (divisor <= MCAL_SPI_DIVISOR_MAX))
        {
            status = MCAL_SPI_STATUS_OK;
        }
        else
        {
            /* Requested rate is outside the hardware divider range. */
        }
    }
    else
    {
        /* Zero source clock or bit rate is invalid. */
    }

    return status;
}

static Mcal_SpiStatusType IsConfigValid(
    const Mcal_SpiConfigType * config)
{
    Mcal_SpiStatusType status;

    if(config != NULL)
    {
        status = IsModuleValid(config->module);

        if(status == MCAL_SPI_STATUS_OK)
        {
            status = IsModeValid(config->mode);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_SPI_STATUS_OK)
        {
            status = IsWidthValid(config->dataWidth);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_SPI_STATUS_OK)
        {
            status =
                IsBaudConfigValid(
                    config->sourceClockHz,
                    config->bitRateHz);
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        status = MCAL_SPI_STATUS_INV_ARG;
    }

    return status;
}

static uint16_t CalculateBaudRegister(
    uint32_t sourceClockHz,
    uint32_t bitRateHz)
{
    uint32_t divisor;
    uint16_t baudRegister;

    divisor = sourceClockHz / bitRateHz;

    if((sourceClockHz % bitRateHz) != 0UL)
    {
        divisor++;
    }
    else
    {
        /* Exact integer divider. */
    }

    baudRegister = (uint16_t)(divisor - 1UL);

    if(baudRegister < MCAL_SPI_BRR_MIN)
    {
        baudRegister = MCAL_SPI_BRR_MIN;
    }
    else if(baudRegister > MCAL_SPI_BRR_MAX)
    {
        baudRegister = MCAL_SPI_BRR_MAX;
    }
    else
    {
        /* Value is valid. */
    }

    return baudRegister;
}

static Mcal_SpiStatusType WaitForTxSpace(
    volatile struct SPI_REGS * spiRegs)
{
    Mcal_SpiStatusType status;
    uint32_t pollCount;

    status = MCAL_SPI_STATUS_OK;
    pollCount = MCAL_SPI_POLL_LIMIT;

    while((spiRegs->SPIFFTX.bit.TXFFST >= MCAL_SPI_FIFO_DEPTH) && (pollCount > 0UL))
    {
        pollCount--;
    }

    if(spiRegs->SPIFFTX.bit.TXFFST >= MCAL_SPI_FIFO_DEPTH)
    {
        status = MCAL_SPI_STATUS_TIMEOUT;
    }
    else
    {
        /* TX FIFO has space. */
    }

    return status;
}

static Mcal_SpiStatusType WaitForRxData(
    volatile struct SPI_REGS * spiRegs)
{
    Mcal_SpiStatusType status;
    uint32_t pollCount;

    status = MCAL_SPI_STATUS_OK;
    pollCount = MCAL_SPI_POLL_LIMIT;

    while((spiRegs->SPIFFRX.bit.RXFFST == 0U) &&
          (pollCount > 0UL))
    {
        pollCount--;
    }

    if(spiRegs->SPIFFRX.bit.RXFFST == 0U)
    {
        status = MCAL_SPI_STATUS_TIMEOUT;
    }
    else
    {
        /* RX FIFO contains received data. */
    }

    return status;
}

static void ResetFifos(
    volatile struct SPI_REGS * spiRegs)
{
    spiRegs->SPIFFTX.bit.TXFIFO =
        MCAL_SPI_FIFO_RESET_HOLD;
    spiRegs->SPIFFRX.bit.RXFIFORESET =
        MCAL_SPI_FIFO_RESET_HOLD;

    spiRegs->SPIFFTX.bit.TXFFINTCLR =
        MCAL_SPI_FIFO_FLAG_CLEAR;
    spiRegs->SPIFFRX.bit.RXFFINTCLR =
        MCAL_SPI_FIFO_FLAG_CLEAR;
    spiRegs->SPIFFRX.bit.RXFFOVFCLR =
        MCAL_SPI_FIFO_OVERFLOW_CLEAR;

    spiRegs->SPIFFTX.bit.TXFIFO =
        MCAL_SPI_FIFO_RESET_RELEASE;
    spiRegs->SPIFFRX.bit.RXFIFORESET =
        MCAL_SPI_FIFO_RESET_RELEASE;
}

static uint16_t GetTxShift(
    Mcal_SpiIdType module)
{
    return 16U - DataWidthBits[(uint16_t)module];
}

static uint16_t GetRxMask(
    Mcal_SpiIdType module)
{
    uint16_t mask;

    if(DataWidthBits[(uint16_t)module] == 16U)
    {
        mask = 0xFFFFU;
    }
    else
    {
        mask = 0x00FFU;
    }

    return mask;
}
