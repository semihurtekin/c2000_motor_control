/**
 * @file    mcal_sci.c
 * @brief   F28379D SCI MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stddef.h>

#include "mcal_sci.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_SCI_LSPCLK_HZ              (50000000UL)
#define MCAL_SCI_BAUD_OVERSAMPLE        (8UL)
#define MCAL_SCI_MAX_BAUD               (3125000UL)
#define MCAL_SCI_BAUD_DIV_MAX           (0xFFFFUL)

#define MCAL_SCI_CHAR_8BIT               (7U)
#define MCAL_SCI_IDLE_LINE_MODE          (0U)
#define MCAL_SCI_LOOPBACK_DISABLE        (0U)
#define MCAL_SCI_PARITY_DISABLE          (0U)
#define MCAL_SCI_STOP_1                  (0U)

#define MCAL_SCI_RX_ENABLE               (1U)
#define MCAL_SCI_TX_ENABLE               (1U)
#define MCAL_SCI_SLEEP_DISABLE           (0U)
#define MCAL_SCI_TX_WAKE_DISABLE         (0U)
#define MCAL_SCI_RESET_HOLD              (0U)
#define MCAL_SCI_RESET_RELEASE           (1U)
#define MCAL_SCI_RX_ERR_INT_DISABLE      (0U)
#define MCAL_SCI_TX_INT_DISABLE          (0U)
#define MCAL_SCI_RX_INT_DISABLE          (0U)

#define MCAL_SCI_FIFO_ENABLE             (1U)
#define MCAL_SCI_FIFO_RESET_HOLD         (0U)
#define MCAL_SCI_FIFO_RESET_RELEASE      (1U)
#define MCAL_SCI_FIFO_INT_DISABLE        (0U)
#define MCAL_SCI_FIFO_FLAG_CLEAR         (1U)
#define MCAL_SCI_FIFO_OVERFLOW_CLEAR     (1U)
#define MCAL_SCI_FIFO_DEPTH              (16U)

#define MCAL_SCI_RX_NOT_READY            (0U)
#define MCAL_SCI_RX_READY                (1U)
#define MCAL_SCI_DATA_MASK               (0x00FFU)

/*==============================================================================
 * Private Types
 *============================================================================*/

/*==============================================================================
 * Private Variables
 *============================================================================*/

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static volatile struct SCI_REGS * GetSciRegs(
    Mcal_SciIdType module);

static Mcal_SciStatusType IsModuleValid(
    Mcal_SciIdType module);

static Mcal_SciStatusType IsConfigValid(
    const Mcal_SciConfigType * config);

static Mcal_SciStatusType CalculateBaudDivider(
    uint32_t baudRate,
    uint16_t * divider);

static void EnablePeripheralClock(
    Mcal_SciIdType module);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_SciStatusType Mcal_Sci_Init(
    const Mcal_SciConfigType * config)
{
    Mcal_SciStatusType status;
    volatile struct SCI_REGS * sciRegs;
    uint16_t baudDivider;

    status = IsConfigValid(config);

    if(status == MCAL_SCI_STATUS_OK)
    {
        status = CalculateBaudDivider(
            config->baudRate,
            &baudDivider);

        if(status == MCAL_SCI_STATUS_OK)
        {
            EnablePeripheralClock(config->module);
            sciRegs = GetSciRegs(config->module);

            /* Keep SCI in reset while configuration is updated. */
            sciRegs->SCICTL1.bit.SWRESET =
                MCAL_SCI_RESET_HOLD;

            sciRegs->SCICCR.bit.SCICHAR =
                MCAL_SCI_CHAR_8BIT;
            sciRegs->SCICCR.bit.ADDRIDLE_MODE =
                MCAL_SCI_IDLE_LINE_MODE;
            sciRegs->SCICCR.bit.LOOPBKENA =
                MCAL_SCI_LOOPBACK_DISABLE;
            sciRegs->SCICCR.bit.PARITYENA =
                MCAL_SCI_PARITY_DISABLE;
            sciRegs->SCICCR.bit.PARITY = 0U;
            sciRegs->SCICCR.bit.STOPBITS =
                MCAL_SCI_STOP_1;

            sciRegs->SCIHBAUD.bit.BAUD =
                (uint16_t)((baudDivider >> 8U) & 0x00FFU);
            sciRegs->SCILBAUD.bit.BAUD =
                (uint16_t)(baudDivider & 0x00FFU);

            sciRegs->SCICTL1.bit.RXENA =
                MCAL_SCI_RX_ENABLE;
            sciRegs->SCICTL1.bit.TXENA =
                MCAL_SCI_TX_ENABLE;
            sciRegs->SCICTL1.bit.SLEEP =
                MCAL_SCI_SLEEP_DISABLE;
            sciRegs->SCICTL1.bit.TXWAKE =
                MCAL_SCI_TX_WAKE_DISABLE;
            sciRegs->SCICTL1.bit.RXERRINTENA =
                MCAL_SCI_RX_ERR_INT_DISABLE;

            sciRegs->SCICTL2.bit.TXINTENA =
                MCAL_SCI_TX_INT_DISABLE;
            sciRegs->SCICTL2.bit.RXBKINTENA =
                MCAL_SCI_RX_INT_DISABLE;

            sciRegs->SCIFFTX.bit.SCIFFENA =
                MCAL_SCI_FIFO_ENABLE;
            sciRegs->SCIFFTX.bit.SCIRST =
                MCAL_SCI_FIFO_RESET_RELEASE;
            sciRegs->SCIFFTX.bit.TXFFIENA =
                MCAL_SCI_FIFO_INT_DISABLE;
            sciRegs->SCIFFTX.bit.TXFFINTCLR =
                MCAL_SCI_FIFO_FLAG_CLEAR;
            sciRegs->SCIFFTX.bit.TXFIFORESET =
                MCAL_SCI_FIFO_RESET_HOLD;
            sciRegs->SCIFFTX.bit.TXFIFORESET =
                MCAL_SCI_FIFO_RESET_RELEASE;

            sciRegs->SCIFFRX.bit.RXFFIENA =
                MCAL_SCI_FIFO_INT_DISABLE;
            sciRegs->SCIFFRX.bit.RXFFINTCLR =
                MCAL_SCI_FIFO_FLAG_CLEAR;
            sciRegs->SCIFFRX.bit.RXFFOVRCLR =
                MCAL_SCI_FIFO_OVERFLOW_CLEAR;
            sciRegs->SCIFFRX.bit.RXFIFORESET =
                MCAL_SCI_FIFO_RESET_HOLD;
            sciRegs->SCIFFRX.bit.RXFIFORESET =
                MCAL_SCI_FIFO_RESET_RELEASE;

            sciRegs->SCIFFCT.all = 0U;

            /* Start the configured SCI module. */
            sciRegs->SCICTL1.bit.SWRESET =
                MCAL_SCI_RESET_RELEASE;
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

Mcal_SciStatusType Mcal_Sci_WriteByte(
    Mcal_SciIdType module,
    uint16_t data)
{
    Mcal_SciStatusType status;
    volatile struct SCI_REGS * sciRegs;

    status = IsModuleValid(module);

    if(status == MCAL_SCI_STATUS_OK)
    {
        sciRegs = GetSciRegs(module);

        while(sciRegs->SCIFFTX.bit.TXFFST >=
              MCAL_SCI_FIFO_DEPTH)
        {
            /* Wait for TX FIFO space. */
        }

        sciRegs->SCITXBUF.bit.TXDT =
            data & MCAL_SCI_DATA_MASK;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_SciStatusType Mcal_Sci_Write(
    Mcal_SciIdType module,
    const uint16_t * data,
    uint16_t length)
{
    Mcal_SciStatusType status;
    uint16_t index;

    status = IsModuleValid(module);

    if(status == MCAL_SCI_STATUS_OK)
    {
        if((data != NULL) || (length == 0U))
        {
            index = 0U;

            while((index < length) &&
                  (status == MCAL_SCI_STATUS_OK))
            {
                status = Mcal_Sci_WriteByte(
                    module,
                    data[index]);
                index++;
            }
        }
        else
        {
            status = MCAL_SCI_STATUS_INV_ARG;
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_SciStatusType Mcal_Sci_IsRxReady(
    Mcal_SciIdType module,
    uint16_t * ready)
{
    Mcal_SciStatusType status;
    volatile struct SCI_REGS * sciRegs;

    status = IsModuleValid(module);

    if(status == MCAL_SCI_STATUS_OK)
    {
        if(ready != NULL)
        {
            sciRegs = GetSciRegs(module);

            if(sciRegs->SCIFFRX.bit.RXFFST != 0U)
            {
                *ready = MCAL_SCI_RX_READY;
            }
            else
            {
                *ready = MCAL_SCI_RX_NOT_READY;
            }
        }
        else
        {
            status = MCAL_SCI_STATUS_INV_ARG;
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_SciStatusType Mcal_Sci_ReadByte(
    Mcal_SciIdType module,
    uint16_t * data)
{
    Mcal_SciStatusType status;
    volatile struct SCI_REGS * sciRegs;

    status = IsModuleValid(module);

    if(status == MCAL_SCI_STATUS_OK)
    {
        if(data != NULL)
        {
            sciRegs = GetSciRegs(module);

            if(sciRegs->SCIFFRX.bit.RXFFST != 0U)
            {
                *data = sciRegs->SCIRXBUF.bit.SAR &
                    MCAL_SCI_DATA_MASK;
            }
            else
            {
                status = MCAL_SCI_STATUS_NO_DATA;
            }
        }
        else
        {
            status = MCAL_SCI_STATUS_INV_ARG;
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

static volatile struct SCI_REGS * GetSciRegs(
    Mcal_SciIdType module)
{
    volatile struct SCI_REGS * sciRegs;

    sciRegs = NULL;

    switch(module)
    {
        case MCAL_SCI_A:
            sciRegs = &SciaRegs;
            break;

        case MCAL_SCI_B:
            sciRegs = &ScibRegs;
            break;

        case MCAL_SCI_C:
            sciRegs = &ScicRegs;
            break;

        case MCAL_SCI_D:
            sciRegs = &ScidRegs;
            break;

        default:
            /* Do nothing. */
            break;
    }

    return sciRegs;
}

static Mcal_SciStatusType IsModuleValid(
    Mcal_SciIdType module)
{
    Mcal_SciStatusType status;

    if((module == MCAL_SCI_A) ||
       (module == MCAL_SCI_B) ||
       (module == MCAL_SCI_C) ||
       (module == MCAL_SCI_D))
    {
        status = MCAL_SCI_STATUS_OK;
    }
    else
    {
        status = MCAL_SCI_STATUS_INV_ID;
    }

    return status;
}

static Mcal_SciStatusType IsConfigValid(
    const Mcal_SciConfigType * config)
{
    Mcal_SciStatusType status;

    if(config != NULL)
    {
        status = IsModuleValid(config->module);

        if(status == MCAL_SCI_STATUS_OK)
        {
            if((config->baudRate != 0UL) &&
               (config->baudRate <= MCAL_SCI_MAX_BAUD))
            {
                /* Valid baud-rate range is checked further by divider. */
            }
            else
            {
                status = MCAL_SCI_STATUS_INV_ARG;
            }
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        status = MCAL_SCI_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_SciStatusType CalculateBaudDivider(
    uint32_t baudRate,
    uint16_t * divider)
{
    Mcal_SciStatusType status;
    uint32_t denominator;
    uint32_t dividerPlusOne;
    uint32_t dividerValue;

    status = MCAL_SCI_STATUS_OK;

    denominator =
        baudRate * MCAL_SCI_BAUD_OVERSAMPLE;

    dividerPlusOne =
        (MCAL_SCI_LSPCLK_HZ +
         (denominator / 2UL)) /
        denominator;

    if(dividerPlusOne > 0UL)
    {
        dividerValue = dividerPlusOne - 1UL;

        if(dividerValue <= MCAL_SCI_BAUD_DIV_MAX)
        {
            *divider = (uint16_t)dividerValue;
        }
        else
        {
            status = MCAL_SCI_STATUS_INV_ARG;
        }
    }
    else
    {
        status = MCAL_SCI_STATUS_INV_ARG;
    }

    return status;
}

static void EnablePeripheralClock(
    Mcal_SciIdType module)
{
    EALLOW;

    switch(module)
    {
        case MCAL_SCI_A:
            CpuSysRegs.PCLKCR7.bit.SCI_A = 1U;
            break;

        case MCAL_SCI_B:
            CpuSysRegs.PCLKCR7.bit.SCI_B = 1U;
            break;

        case MCAL_SCI_C:
            CpuSysRegs.PCLKCR7.bit.SCI_C = 1U;
            break;

        case MCAL_SCI_D:
            CpuSysRegs.PCLKCR7.bit.SCI_D = 1U;
            break;

        default:
            /* Do nothing. */
            break;
    }

    EDIS;
}
