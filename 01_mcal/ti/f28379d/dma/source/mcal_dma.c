/**
 * @file    mcal_dma.c
 * @brief   F28379D DMA peripheral MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stddef.h>

#include "mcal_dma.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_DMA_BURST_WORDS_MIN       (1U)
#define MCAL_DMA_BURST_WORDS_MAX       (32U)

#define MCAL_DMA_TRANSFER_MIN          (1UL)
#define MCAL_DMA_TRANSFER_MAX          (65536UL)

#define MCAL_DMA_STEP_MIN              (-4096)
#define MCAL_DMA_STEP_MAX              (4095)

#define MCAL_DMA_WRAP_DISABLED_SIZE    (0xFFFFU)
#define MCAL_DMA_DATA_SIZE_16BIT       (0U)

#define MCAL_DMA_FLAG_CLEAR            (1U)
#define MCAL_DMA_FLAG_RESET            (0U)
#define MCAL_DMA_FLAG_SET              (1U)

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static volatile struct CH_REGS * GetChannelRegs(
    Mcal_DmaChannelType channel);

static Mcal_DmaStatusType IsChannelValid(
    Mcal_DmaChannelType channel);

static Mcal_DmaStatusType IsTriggerValid(
    Mcal_DmaTriggerType trigger);


static Mcal_DmaStatusType IsIntModeValid(
    Mcal_DmaIntModeType mode);

static Mcal_DmaStatusType IsConfigValid(
    const Mcal_DmaChannelConfigType * config);

static uint16_t IsStepValid(
    int16_t step);

static uint16_t GetHwChannelId(
    Mcal_DmaChannelType channel);

static void SetTriggerSource(
    Mcal_DmaChannelType channel,
    Mcal_DmaTriggerType trigger);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_DmaStatusType Mcal_Dma_Init(void)
{
    EALLOW;

    /*
     * The platform owns peripheral clock gating. PCLKCR0.DMA shall already
     * be enabled before this reset is issued.
     */
    DmaRegs.DMACTRL.bit.HARDRESET = 1U;

    /*
     * F2837xD requires one NOP immediately after HARDRESET.
     */
    __asm(" NOP");

    /*
     * Use round-robin arbitration for DMA channels.
     */
    DmaRegs.PRIORITYCTRL1.bit.CH1PRIORITY = 0U;

    EDIS;

    return MCAL_DMA_STATUS_OK;
}

Mcal_DmaStatusType Mcal_Dma_InitChannel(
    const Mcal_DmaChannelConfigType * config)
{
    volatile struct CH_REGS * dmaChRegs;
    Mcal_DmaStatusType status;
    uint16_t hwChannelId;

    status = IsConfigValid(config);

    if(status == MCAL_DMA_STATUS_OK)
    {
        dmaChRegs = GetChannelRegs(config->channel);
        hwChannelId = GetHwChannelId(config->channel);

        EALLOW;

        /*
         * Reset the channel first so that it cannot consume a peripheral
         * trigger while its transfer contract is only partially configured.
         */
        dmaChRegs->CONTROL.bit.SOFTRESET = 1U;
        __asm(" NOP");

        dmaChRegs->MODE.bit.PERINTE = 0U;
        dmaChRegs->CONTROL.bit.PERINTCLR = MCAL_DMA_FLAG_CLEAR;
        dmaChRegs->CONTROL.bit.ERRCLR = MCAL_DMA_FLAG_CLEAR;

        /*
         * Route the selected system trigger to this DMA channel and select
         * the channel's local peripheral-interrupt input.
         */
        SetTriggerSource(
            config->channel,
            config->trigger);

        dmaChRegs->MODE.bit.PERINTSEL = hwChannelId;

        /*
         * Initialize both beginning and current shadow addresses.
         */
        dmaChRegs->SRC_BEG_ADDR_SHADOW =
            (uint32_t)config->sourceAddress;

        dmaChRegs->SRC_ADDR_SHADOW =
            (uint32_t)config->sourceAddress;

        dmaChRegs->DST_BEG_ADDR_SHADOW =
            (uint32_t)config->destinationAddress;

        dmaChRegs->DST_ADDR_SHADOW =
            (uint32_t)config->destinationAddress;

        /*
         * Hardware encodes the number of words per burst as N - 1.
         */
        dmaChRegs->BURST_SIZE.bit.BURSTSIZE =
            (uint16_t)(config->burstWords - 1U);

        dmaChRegs->SRC_BURST_STEP =
            config->sourceBurstStep;

        dmaChRegs->DST_BURST_STEP =
            config->destinationBurstStep;

        /*
         * Hardware encodes the number of bursts per transfer as N - 1.
         */
        dmaChRegs->TRANSFER_SIZE =
            (uint16_t)(config->transferBursts - 1UL);

        dmaChRegs->SRC_TRANSFER_STEP =
            config->sourceTransferStep;

        dmaChRegs->DST_TRANSFER_STEP =
            config->destinationTransferStep;

        /*
         * DMA v0.1 does not expose wrap behavior. A maximum wrap size keeps
         * wrapping inactive throughout any supported transfer.
         */
        dmaChRegs->SRC_WRAP_SIZE =
            MCAL_DMA_WRAP_DISABLED_SIZE;

        dmaChRegs->SRC_WRAP_STEP = 0;

        dmaChRegs->DST_WRAP_SIZE =
            MCAL_DMA_WRAP_DISABLED_SIZE;

        dmaChRegs->DST_WRAP_STEP = 0;

        /*
         * DMA v0.1 supports 16-bit transfers only. CPU/DMA channel interrupts
         * and overrun interrupts are not enabled in this vertical slice.
         */
        dmaChRegs->MODE.bit.DATASIZE =
            MCAL_DMA_DATA_SIZE_16BIT;

        dmaChRegs->MODE.bit.ONESHOT =
            (uint16_t)config->oneShot;

        dmaChRegs->MODE.bit.CONTINUOUS =
            (uint16_t)config->continuous;

        /*
         * Channel interrupts are configured separately so that the transfer
         * contract and CPU notification policy remain independent.
         */
        dmaChRegs->MODE.bit.CHINTMODE =
            (uint16_t)MCAL_DMA_INT_AT_BEGINNING;
        dmaChRegs->MODE.bit.CHINTE = 0U;
        dmaChRegs->MODE.bit.OVRINTE = 0U;

        /*
         * Leave peripheral triggering disabled. Mcal_Dma_Start() is the
         * explicit arming boundary.
         */
        dmaChRegs->MODE.bit.PERINTE = 0U;

        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_DmaStatusType Mcal_Dma_Start(
    Mcal_DmaChannelType channel)
{
    volatile struct CH_REGS * dmaChRegs;
    Mcal_DmaStatusType status;

    status = IsChannelValid(channel);

    if(status == MCAL_DMA_STATUS_OK)
    {
        dmaChRegs = GetChannelRegs(channel);

        EALLOW;

        /*
         * Do not inherit a stale pending trigger or stale error indication
         * from an earlier run.
         */
        dmaChRegs->CONTROL.bit.PERINTCLR =
            MCAL_DMA_FLAG_CLEAR;

        dmaChRegs->CONTROL.bit.ERRCLR =
            MCAL_DMA_FLAG_CLEAR;

        /*
         * Enable trigger reception before RUN so that an event occurring at
         * the arming boundary becomes pending instead of being lost.
         */
        dmaChRegs->MODE.bit.PERINTE = 1U;
        dmaChRegs->CONTROL.bit.RUN = 1U;

        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_DmaStatusType Mcal_Dma_Stop(
    Mcal_DmaChannelType channel)
{
    volatile struct CH_REGS * dmaChRegs;
    Mcal_DmaStatusType status;

    status = IsChannelValid(channel);

    if(status == MCAL_DMA_STATUS_OK)
    {
        dmaChRegs = GetChannelRegs(channel);

        EALLOW;

        /*
         * Block new peripheral events before requesting a channel halt.
         * The DMA completes any read-write access already in progress.
         */
        dmaChRegs->MODE.bit.PERINTE = 0U;
        dmaChRegs->CONTROL.bit.HALT = 1U;
        dmaChRegs->CONTROL.bit.PERINTCLR =
            MCAL_DMA_FLAG_CLEAR;

        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_DmaStatusType Mcal_Dma_IsRunning(
    Mcal_DmaChannelType channel,
    uint16_t * running)
{
    volatile struct CH_REGS * dmaChRegs;
    Mcal_DmaStatusType status;

    status = IsChannelValid(channel);

    if(status == MCAL_DMA_STATUS_OK)
    {
        if(running != NULL)
        {
            dmaChRegs = GetChannelRegs(channel);

            if(dmaChRegs->CONTROL.bit.RUNSTS != 0U)
            {
                *running = MCAL_DMA_FLAG_SET;
            }
            else
            {
                *running = MCAL_DMA_FLAG_RESET;
            }
        }
        else
        {
            status = MCAL_DMA_STATUS_INV_ARG;
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_DmaStatusType Mcal_Dma_IsTransferDone(
    Mcal_DmaChannelType channel,
    uint16_t * done)
{
    volatile struct CH_REGS * dmaChRegs;
    Mcal_DmaStatusType status;

    status = IsChannelValid(channel);

    if(status == MCAL_DMA_STATUS_OK)
    {
        if(done != NULL)
        {
            dmaChRegs = GetChannelRegs(channel);

            if((dmaChRegs->CONTROL.bit.RUNSTS == 0U) &&
               (dmaChRegs->TRANSFER_COUNT == 0U))
            {
                *done = MCAL_DMA_FLAG_SET;
            }
            else
            {
                *done = MCAL_DMA_FLAG_RESET;
            }
        }
        else
        {
            status = MCAL_DMA_STATUS_INV_ARG;
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_DmaStatusType Mcal_Dma_IsOverrun(
    Mcal_DmaChannelType channel,
    uint16_t * overrun)
{
    volatile struct CH_REGS * dmaChRegs;
    Mcal_DmaStatusType status;

    status = IsChannelValid(channel);

    if(status == MCAL_DMA_STATUS_OK)
    {
        if(overrun != NULL)
        {
            dmaChRegs = GetChannelRegs(channel);

            if(dmaChRegs->CONTROL.bit.OVRFLG != 0U)
            {
                *overrun = MCAL_DMA_FLAG_SET;
            }
            else
            {
                *overrun = MCAL_DMA_FLAG_RESET;
            }
        }
        else
        {
            status = MCAL_DMA_STATUS_INV_ARG;
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_DmaStatusType Mcal_Dma_ClearOverrun(
    Mcal_DmaChannelType channel)
{
    volatile struct CH_REGS * dmaChRegs;
    Mcal_DmaStatusType status;

    status = IsChannelValid(channel);

    if(status == MCAL_DMA_STATUS_OK)
    {
        dmaChRegs = GetChannelRegs(channel);

        EALLOW;

        /*
         * ERRCLR clears both DMA synchronization and overrun errors.
         */
        dmaChRegs->CONTROL.bit.ERRCLR =
            MCAL_DMA_FLAG_CLEAR;

        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}


Mcal_DmaStatusType Mcal_Dma_EnableInterrupt(
    Mcal_DmaChannelType channel,
    Mcal_DmaIntModeType mode)
{
    volatile struct CH_REGS * dmaChRegs;
    Mcal_DmaStatusType status;

    status = IsChannelValid(channel);

    if(status == MCAL_DMA_STATUS_OK)
    {
        status = IsIntModeValid(mode);

        if(status == MCAL_DMA_STATUS_OK)
        {
            dmaChRegs = GetChannelRegs(channel);

            EALLOW;

            dmaChRegs->MODE.bit.CHINTMODE =
                (uint16_t)mode;

            dmaChRegs->MODE.bit.CHINTE = 1U;

            EDIS;
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

Mcal_DmaStatusType Mcal_Dma_DisableInterrupt(
    Mcal_DmaChannelType channel)
{
    volatile struct CH_REGS * dmaChRegs;
    Mcal_DmaStatusType status;

    status = IsChannelValid(channel);

    if(status == MCAL_DMA_STATUS_OK)
    {
        dmaChRegs = GetChannelRegs(channel);

        EALLOW;
        dmaChRegs->MODE.bit.CHINTE = 0U;
        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_DmaStatusType Mcal_Dma_SetDstStartAddress(
    Mcal_DmaChannelType channel,
    volatile uint16_t * destinationAddress)
{
    volatile struct CH_REGS * dmaChRegs;
    Mcal_DmaStatusType status;

    status = IsChannelValid(channel);

    if(status == MCAL_DMA_STATUS_OK)
    {
        if(destinationAddress != NULL)
        {
            dmaChRegs = GetChannelRegs(channel);

            EALLOW;

            /*
             * Continuous mode reloads the active destination from this shadow
             * address when the next transfer begins. Do not touch the active
             * destination pointer of the transfer currently in progress.
             */
            dmaChRegs->DST_ADDR_SHADOW =
                (uint32_t)destinationAddress;

            EDIS;
        }
        else
        {
            status = MCAL_DMA_STATUS_INV_ARG;
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

static volatile struct CH_REGS * GetChannelRegs(
    Mcal_DmaChannelType channel)
{
    volatile struct CH_REGS * dmaChRegs;

    dmaChRegs = NULL;

    switch(channel)
    {
        case MCAL_DMA_CHANNEL_1:
            dmaChRegs = &DmaRegs.CH1;
            break;

        case MCAL_DMA_CHANNEL_2:
            dmaChRegs = &DmaRegs.CH2;
            break;

        case MCAL_DMA_CHANNEL_3:
            dmaChRegs = &DmaRegs.CH3;
            break;

        case MCAL_DMA_CHANNEL_4:
            dmaChRegs = &DmaRegs.CH4;
            break;

        case MCAL_DMA_CHANNEL_5:
            dmaChRegs = &DmaRegs.CH5;
            break;

        case MCAL_DMA_CHANNEL_6:
            dmaChRegs = &DmaRegs.CH6;
            break;

        default:
            /* Do nothing. */
            break;
    }

    return dmaChRegs;
}

static Mcal_DmaStatusType IsChannelValid(
    Mcal_DmaChannelType channel)
{
    Mcal_DmaStatusType status;

    switch(channel)
    {
        case MCAL_DMA_CHANNEL_1:
        case MCAL_DMA_CHANNEL_2:
        case MCAL_DMA_CHANNEL_3:
        case MCAL_DMA_CHANNEL_4:
        case MCAL_DMA_CHANNEL_5:
        case MCAL_DMA_CHANNEL_6:
            status = MCAL_DMA_STATUS_OK;
            break;

        default:
            status = MCAL_DMA_STATUS_INV_CH;
            break;
    }

    return status;
}

static Mcal_DmaStatusType IsTriggerValid(
    Mcal_DmaTriggerType trigger)
{
    Mcal_DmaStatusType status;

    switch(trigger)
    {
        case MCAL_DMA_TRIG_ADCA1:
        case MCAL_DMA_TRIG_ADCB1:
            status = MCAL_DMA_STATUS_OK;
            break;

        default:
            status = MCAL_DMA_STATUS_INV_ARG;
            break;
    }

    return status;
}

static Mcal_DmaStatusType IsConfigValid(
    const Mcal_DmaChannelConfigType * config)
{
    Mcal_DmaStatusType status;

    if(config != NULL)
    {
        status = IsChannelValid(config->channel);

        if(status == MCAL_DMA_STATUS_OK)
        {
            status = IsTriggerValid(config->trigger);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_DMA_STATUS_OK)
        {
            if((config->sourceAddress != NULL) &&
               (config->destinationAddress != NULL))
            {
                /* Configuration remains valid. */
            }
            else
            {
                status = MCAL_DMA_STATUS_INV_ARG;
            }
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_DMA_STATUS_OK)
        {
            if((config->burstWords >=
                MCAL_DMA_BURST_WORDS_MIN) &&
               (config->burstWords <=
                MCAL_DMA_BURST_WORDS_MAX))
            {
                /* Configuration remains valid. */
            }
            else
            {
                status = MCAL_DMA_STATUS_INV_ARG;
            }
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_DMA_STATUS_OK)
        {
            if((config->transferBursts >=
                MCAL_DMA_TRANSFER_MIN) &&
               (config->transferBursts <=
                MCAL_DMA_TRANSFER_MAX))
            {
                /* Configuration remains valid. */
            }
            else
            {
                status = MCAL_DMA_STATUS_INV_ARG;
            }
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_DMA_STATUS_OK)
        {
            if((IsStepValid(config->sourceBurstStep) != 0U) &&
               (IsStepValid(config->destinationBurstStep) != 0U) &&
               (IsStepValid(config->sourceTransferStep) != 0U) &&
               (IsStepValid(config->destinationTransferStep) != 0U))
            {
                /* Configuration remains valid. */
            }
            else
            {
                status = MCAL_DMA_STATUS_INV_ARG;
            }
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_DMA_STATUS_OK)
        {
            if(((config->oneShot ==
                 MCAL_DMA_ONESHOT_DISABLE) ||
                (config->oneShot ==
                 MCAL_DMA_ONESHOT_ENABLE)) &&
               ((config->continuous ==
                 MCAL_DMA_CONT_DISABLE) ||
                (config->continuous ==
                 MCAL_DMA_CONT_ENABLE)))
            {
                /* Configuration remains valid. */
            }
            else
            {
                status = MCAL_DMA_STATUS_INV_ARG;
            }
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        status = MCAL_DMA_STATUS_INV_ARG;
    }

    return status;
}

static uint16_t IsStepValid(
    int16_t step)
{
    uint16_t valid;

    if((step >= MCAL_DMA_STEP_MIN) &&
       (step <= MCAL_DMA_STEP_MAX))
    {
        valid = MCAL_DMA_FLAG_SET;
    }
    else
    {
        valid = MCAL_DMA_FLAG_RESET;
    }

    return valid;
}


static Mcal_DmaStatusType IsIntModeValid(
    Mcal_DmaIntModeType mode)
{
    Mcal_DmaStatusType status;

    if((mode == MCAL_DMA_INT_AT_BEGINNING) ||
       (mode == MCAL_DMA_INT_AT_END))
    {
        status = MCAL_DMA_STATUS_OK;
    }
    else
    {
        status = MCAL_DMA_STATUS_INV_ARG;
    }

    return status;
}

static uint16_t GetHwChannelId(
    Mcal_DmaChannelType channel)
{
    uint16_t hwChannelId;

    hwChannelId = 0U;

    switch(channel)
    {
        case MCAL_DMA_CHANNEL_1:
            hwChannelId = 1U;
            break;

        case MCAL_DMA_CHANNEL_2:
            hwChannelId = 2U;
            break;

        case MCAL_DMA_CHANNEL_3:
            hwChannelId = 3U;
            break;

        case MCAL_DMA_CHANNEL_4:
            hwChannelId = 4U;
            break;

        case MCAL_DMA_CHANNEL_5:
            hwChannelId = 5U;
            break;

        case MCAL_DMA_CHANNEL_6:
            hwChannelId = 6U;
            break;

        default:
            /* Do nothing. */
            break;
    }

    return hwChannelId;
}

static void SetTriggerSource(
    Mcal_DmaChannelType channel,
    Mcal_DmaTriggerType trigger)
{
    switch(channel)
    {
        case MCAL_DMA_CHANNEL_1:
            DmaClaSrcSelRegs.DMACHSRCSEL1.bit.CH1 =
                (uint16_t)trigger;
            break;

        case MCAL_DMA_CHANNEL_2:
            DmaClaSrcSelRegs.DMACHSRCSEL1.bit.CH2 =
                (uint16_t)trigger;
            break;

        case MCAL_DMA_CHANNEL_3:
            DmaClaSrcSelRegs.DMACHSRCSEL1.bit.CH3 =
                (uint16_t)trigger;
            break;

        case MCAL_DMA_CHANNEL_4:
            DmaClaSrcSelRegs.DMACHSRCSEL1.bit.CH4 =
                (uint16_t)trigger;
            break;

        case MCAL_DMA_CHANNEL_5:
            DmaClaSrcSelRegs.DMACHSRCSEL2.bit.CH5 =
                (uint16_t)trigger;
            break;

        case MCAL_DMA_CHANNEL_6:
            DmaClaSrcSelRegs.DMACHSRCSEL2.bit.CH6 =
                (uint16_t)trigger;
            break;

        default:
            /* Do nothing. */
            break;
    }
}
