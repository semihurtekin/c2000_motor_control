/**
 * @file    mcal_dma.h
 * @brief   DMA MCAL interface for TMS320F28379D.
 */

#ifndef MCAL_DMA_H
#define MCAL_DMA_H

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

/*==============================================================================
 * Public Types
 *============================================================================*/

typedef enum
{
    MCAL_DMA_STATUS_OK = 0U,
    MCAL_DMA_STATUS_INV_CH,
    MCAL_DMA_STATUS_INV_ARG
} Mcal_DmaStatusType;

typedef enum
{
    MCAL_DMA_CHANNEL_1 = 0U,
    MCAL_DMA_CHANNEL_2,
    MCAL_DMA_CHANNEL_3,
    MCAL_DMA_CHANNEL_4,
    MCAL_DMA_CHANNEL_5,
    MCAL_DMA_CHANNEL_6
} Mcal_DmaChannelType;

/*
 * Values intentionally match the F28379D DMA trigger-source encoding.
 * Only triggers required by the current ADC/DMA vertical slice are exposed.
 */
typedef enum
{
    MCAL_DMA_TRIG_ADCA1 = 1U,
    MCAL_DMA_TRIG_ADCB1 = 6U
} Mcal_DmaTriggerType;

typedef enum
{
    MCAL_DMA_ONESHOT_DISABLE = 0U,
    MCAL_DMA_ONESHOT_ENABLE
} Mcal_DmaOneShotType;

typedef enum
{
    MCAL_DMA_CONT_DISABLE = 0U,
    MCAL_DMA_CONT_ENABLE
} Mcal_DmaContinuousType;


typedef enum
{
    MCAL_DMA_INT_AT_BEGINNING = 0U,
    MCAL_DMA_INT_AT_END = 1U
} Mcal_DmaIntModeType;

typedef struct
{
    Mcal_DmaChannelType channel;
    Mcal_DmaTriggerType trigger;

    const volatile uint16_t * sourceAddress;
    volatile uint16_t * destinationAddress;

    uint16_t burstWords;
    int16_t sourceBurstStep;
    int16_t destinationBurstStep;

    uint32_t transferBursts;
    int16_t sourceTransferStep;
    int16_t destinationTransferStep;

    Mcal_DmaOneShotType oneShot;
    Mcal_DmaContinuousType continuous;
} Mcal_DmaChannelConfigType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes the DMA controller to a known state.
 *
 * The DMA peripheral clock shall be enabled by the platform before this
 * function is called.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_Init(void);

/**
 * @brief Configures one DMA channel.
 *
 * The channel is soft-reset before configuration and remains disarmed after
 * this function returns. DMA v0.1 supports only 16-bit word transfers and
 * disables address wrapping.
 *
 * @param config DMA channel configuration.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_InitChannel(
    const Mcal_DmaChannelConfigType * config);

/**
 * @brief Arms and starts one DMA channel.
 *
 * The configured peripheral trigger is enabled and the channel waits for the
 * first trigger event.
 *
 * @param channel DMA channel.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_Start(
    Mcal_DmaChannelType channel);

/**
 * @brief Stops one DMA channel.
 *
 * Peripheral triggering is disabled before the channel is halted.
 *
 * @param channel DMA channel.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_Stop(
    Mcal_DmaChannelType channel);

/**
 * @brief Reads the DMA channel running state.
 *
 * @param channel DMA channel.
 * @param running Receives 1U when the channel is enabled, otherwise 0U.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_IsRunning(
    Mcal_DmaChannelType channel,
    uint16_t * running);

/**
 * @brief Reads whether a non-continuous transfer has completed.
 *
 * This result is meaningful after Mcal_Dma_Start() and provided the channel
 * has not been manually stopped or reinitialized.
 *
 * @param channel DMA channel.
 * @param done Receives 1U when the transfer is complete, otherwise 0U.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_IsTransferDone(
    Mcal_DmaChannelType channel,
    uint16_t * done);

/**
 * @brief Reads the DMA peripheral-trigger overrun state.
 *
 * @param channel DMA channel.
 * @param overrun Receives 1U when an overrun is detected, otherwise 0U.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_IsOverrun(
    Mcal_DmaChannelType channel,
    uint16_t * overrun);

/**
 * @brief Clears the DMA channel error indication.
 *
 * F28379D ERRCLR clears both the overrun and synchronization error flags.
 *
 * @param channel DMA channel.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_ClearOverrun(
    Mcal_DmaChannelType channel);


/**
 * @brief Enables one DMA channel interrupt.
 *
 * The interrupt can be generated at the beginning or at the end of a DMA
 * transfer. Beginning-of-transfer mode is useful for continuous ping-pong
 * buffering because the active address has already been loaded from the
 * shadow address when the ISR executes.
 *
 * @param channel DMA channel.
 * @param mode    DMA channel interrupt timing.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_EnableInterrupt(
    Mcal_DmaChannelType channel,
    Mcal_DmaIntModeType mode);

/**
 * @brief Disables one DMA channel interrupt.
 *
 * @param channel DMA channel.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_DisableInterrupt(
    Mcal_DmaChannelType channel);

/**
 * @brief Updates the destination shadow address for the next DMA transfer.
 *
 * The active destination address of the transfer currently in progress is not
 * modified. DMA v0.1 keeps destination wrapping disabled.
 *
 * @param channel            DMA channel.
 * @param destinationAddress Destination address for the next transfer.
 *
 * @return Driver status.
 */
Mcal_DmaStatusType Mcal_Dma_SetDstStartAddress(
    Mcal_DmaChannelType channel,
    volatile uint16_t * destinationAddress);

#endif /* MCAL_DMA_H */
