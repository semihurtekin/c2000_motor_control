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

#define MCAL_SPI_LSPCLK_HZ               (50000000UL)
#define MCAL_SPI_MAX_CLK_HZ              (12500000UL)
#define MCAL_SPI_MIN_CLK_HZ              (390625UL)

#define MCAL_SPI_FIFO_DEPTH              (16U)
#define MCAL_SPI_FIFO_ENABLE             (1U)
#define MCAL_SPI_FIFO_RESET_HOLD         (0U)
#define MCAL_SPI_FIFO_RESET_RELEASE      (1U)
#define MCAL_SPI_FIFO_INT_DISABLE        (0U)
#define MCAL_SPI_FIFO_FLAG_CLEAR         (1U)
#define MCAL_SPI_FIFO_OVERFLOW_CLEAR     (1U)

#define MCAL_SPI_SW_RESET_HOLD           (0U)
#define MCAL_SPI_SW_RESET_RELEASE        (1U)
#define MCAL_SPI_LOOPBACK_DISABLE        (0U)
#define MCAL_SPI_HIGH_SPEED_DISABLE      (0U)
#define MCAL_SPI_INT_DISABLE             (0U)
#define MCAL_SPI_TRANSMIT_ENABLE         (1U)
#define MCAL_SPI_CONTROLLER_MODE         (1U)
#define MCAL_SPI_OVERRUN_INT_DISABLE     (0U)
#define MCAL_SPI_THREE_WIRE_DISABLE      (0U)

#define MCAL_SPI_MODE_CPOL_MASK          (2U)
#define MCAL_SPI_MODE_CPHA_MASK          (1U)

/*==============================================================================
 * Private Types
 *============================================================================*/

/*==============================================================================
 * Private Variables
 *============================================================================*/

static uint16_t DataWidthBits[3] = {0U, 0U, 0U};

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

static Mcal_SpiStatusType IsConfigValid(
    const Mcal_SpiConfigType * config);

static uint16_t CalculateBaudDivider(
    uint32_t bitRateHz);

static void EnablePeripheralClock(
    Mcal_SpiIdType module);

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
    uint16_t baudDivider;
    uint16_t cpol;
    uint16_t cpha;

    status = IsConfigValid(config);

    if(status == MCAL_SPI_STATUS_OK)
    {
        EnablePeripheralClock(config->module);
        spiRegs = GetSpiRegs(config->module);

        baudDivider = CalculateBaudDivider(
            config->bitRateHz);

        cpol = ((uint16_t)config->mode &
                MCAL_SPI_MODE_CPOL_MASK) >> 1U;

        cpha = (uint16_t)config->mode &
            MCAL_SPI_MODE_CPHA_MASK;

        /* Keep SPI in reset while configuration is updated. */
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
            baudDivider;

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
        spiRegs->SPIFFTX.bit.TXFIFO =
            MCAL_SPI_FIFO_RESET_HOLD;
        spiRegs->SPIFFTX.bit.TXFIFO =
            MCAL_SPI_FIFO_RESET_RELEASE;

        spiRegs->SPIFFRX.bit.RXFFIENA =
            MCAL_SPI_FIFO_INT_DISABLE;
        spiRegs->SPIFFRX.bit.RXFFINTCLR =
            MCAL_SPI_FIFO_FLAG_CLEAR;
        spiRegs->SPIFFRX.bit.RXFFOVFCLR =
            MCAL_SPI_FIFO_OVERFLOW_CLEAR;
        spiRegs->SPIFFRX.bit.RXFIFORESET =
            MCAL_SPI_FIFO_RESET_HOLD;
        spiRegs->SPIFFRX.bit.RXFIFORESET =
            MCAL_SPI_FIFO_RESET_RELEASE;

        spiRegs->SPIFFCT.all = 0U;

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
                    while(spiRegs->SPIFFTX.bit.TXFFST >=
                          MCAL_SPI_FIFO_DEPTH)
                    {
                        /* Wait for TX FIFO space. */
                    }

                    spiRegs->SPITXBUF =
                        (uint16_t)(txData << shift);

                    while(spiRegs->SPIFFRX.bit.RXFFST == 0U)
                    {
                        /* Wait for full-duplex response. */
                    }

                    *rxData =
                        spiRegs->SPIRXBUF & mask;
                }
                else
                {
                    status = MCAL_SPI_STATUS_INV_ARG;
                }
            }
            else
            {
                status = MCAL_SPI_STATUS_INV_ARG;
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
        if(((txData != NULL) && (rxData != NULL)) ||
           (length == 0U))
        {
            index = 0U;

            while((index < length) &&
                  (status == MCAL_SPI_STATUS_OK))
            {
                status = Mcal_Spi_TransferWord(
                    module,
                    txData[index],
                    &rxData[index]);

                index++;
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
            /* Do nothing. */
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

            if(status == MCAL_SPI_STATUS_OK)
            {
                status = IsWidthValid(config->dataWidth);

                if(status == MCAL_SPI_STATUS_OK)
                {
                    if((config->bitRateHz >= MCAL_SPI_MIN_CLK_HZ) &&
                       (config->bitRateHz <= MCAL_SPI_MAX_CLK_HZ))
                    {
                        /* Configuration is valid. */
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
        status = MCAL_SPI_STATUS_INV_ARG;
    }

    return status;
}

static uint16_t CalculateBaudDivider(
    uint32_t bitRateHz)
{
    uint32_t clockDivider;

    /* Never generate an SCLK above the requested rate. Ceiling */
    clockDivider =
        (MCAL_SPI_LSPCLK_HZ + bitRateHz - 1UL) /
        bitRateHz;

    return (uint16_t)(clockDivider - 1UL);
}

static void EnablePeripheralClock(
    Mcal_SpiIdType module)
{
    EALLOW;

    switch(module)
    {
        case MCAL_SPI_A:
            CpuSysRegs.PCLKCR8.bit.SPI_A = 1U;
            break;

        case MCAL_SPI_B:
            CpuSysRegs.PCLKCR8.bit.SPI_B = 1U;
            break;

        case MCAL_SPI_C:
            CpuSysRegs.PCLKCR8.bit.SPI_C = 1U;
            break;

        default:
            /* Do nothing. */
            break;
    }

    EDIS;
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
