/**
 * @file    mcal_pie.c
 * @brief   C2000 PIE MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "mcal_pie.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_PIE_CHANNEL_MIN    (1U)
#define MCAL_PIE_CHANNEL_MAX    (16U)
#define MCAL_PIE_ACK_ALL_MASK   (0x0FFFU)

/*==============================================================================
 * Private Types
 *============================================================================*/

/*==============================================================================
 * Private Variables
 *============================================================================*/

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

/**
 * @brief Checks whether the given PIE group is valid.
 */
static Mcal_PieStatusType IsGroupValid(
    Mcal_PieGroupType group);

/**
 * @brief Checks whether the given PIE channel is valid.
 */
static Mcal_PieStatusType IsChannelValid(
    Mcal_PieChannelType channel);

/**
 * @brief Returns the bit mask associated with a PIE channel.
 */
static uint16_t GetChannelMask(
    Mcal_PieChannelType channel);

/**
 * @brief Reads the enable register of the selected PIE group.
 */
static uint16_t ReadIer(
    Mcal_PieGroupType group);

/**
 * @brief Writes the enable register of the selected PIE group.
 */
static void WriteIer(
    Mcal_PieGroupType group,
    uint16_t value);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_PieStatusType Mcal_Pie_Init(void)
{
    PieCtrlRegs.PIECTRL.bit.ENPIE = 0U;

    /* Disable every PIE interrupt channel. */
    PieCtrlRegs.PIEIER1.all = 0U;
    PieCtrlRegs.PIEIER2.all = 0U;
    PieCtrlRegs.PIEIER3.all = 0U;
    PieCtrlRegs.PIEIER4.all = 0U;
    PieCtrlRegs.PIEIER5.all = 0U;
    PieCtrlRegs.PIEIER6.all = 0U;
    PieCtrlRegs.PIEIER7.all = 0U;
    PieCtrlRegs.PIEIER8.all = 0U;
    PieCtrlRegs.PIEIER9.all = 0U;
    PieCtrlRegs.PIEIER10.all = 0U;
    PieCtrlRegs.PIEIER11.all = 0U;
    PieCtrlRegs.PIEIER12.all = 0U;

    /* Clear every pending PIE interrupt flag. */
    PieCtrlRegs.PIEIFR1.all = 0U;
    PieCtrlRegs.PIEIFR2.all = 0U;
    PieCtrlRegs.PIEIFR3.all = 0U;
    PieCtrlRegs.PIEIFR4.all = 0U;
    PieCtrlRegs.PIEIFR5.all = 0U;
    PieCtrlRegs.PIEIFR6.all = 0U;
    PieCtrlRegs.PIEIFR7.all = 0U;
    PieCtrlRegs.PIEIFR8.all = 0U;
    PieCtrlRegs.PIEIFR9.all = 0U;
    PieCtrlRegs.PIEIFR10.all = 0U;
    PieCtrlRegs.PIEIFR11.all = 0U;
    PieCtrlRegs.PIEIFR12.all = 0U;

    /* Release all PIE groups and enable the controller. */
    PieCtrlRegs.PIEACK.all = MCAL_PIE_ACK_ALL_MASK;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1U;

    return MCAL_PIE_STATUS_OK;
}

Mcal_PieStatusType Mcal_Pie_Enable(
    Mcal_PieGroupType group,
    Mcal_PieChannelType channel)
{
    Mcal_PieStatusType status;
    uint16_t ierValue;
    uint16_t channelMask;

    status = IsGroupValid(group);

    if(status == MCAL_PIE_STATUS_OK)
    {
        status = IsChannelValid(channel);

        if(status == MCAL_PIE_STATUS_OK)
        {
            channelMask = GetChannelMask(channel);
            ierValue = ReadIer(group);
            ierValue |= channelMask;
            WriteIer(group, ierValue);
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

Mcal_PieStatusType Mcal_Pie_Disable(
    Mcal_PieGroupType group,
    Mcal_PieChannelType channel)
{
    Mcal_PieStatusType status;
    uint16_t ierValue;
    uint16_t channelMask;

    status = IsGroupValid(group);

    if(status == MCAL_PIE_STATUS_OK)
    {
        status = IsChannelValid(channel);

        if(status == MCAL_PIE_STATUS_OK)
        {
            channelMask = GetChannelMask(channel);
            ierValue = ReadIer(group);
            ierValue &= (uint16_t)(~channelMask);
            WriteIer(group, ierValue);
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

Mcal_PieStatusType Mcal_Pie_Ack(
    Mcal_PieGroupType group)
{
    Mcal_PieStatusType status;
    uint16_t ackMask;

    status = IsGroupValid(group);

    if(status == MCAL_PIE_STATUS_OK)
    {
        ackMask = (uint16_t)((uint32_t)1U <<
                            ((uint16_t)group - 1U));
        PieCtrlRegs.PIEACK.all = ackMask;
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

static Mcal_PieStatusType IsGroupValid(
    Mcal_PieGroupType group)
{
    Mcal_PieStatusType status;

    if(((uint16_t)group >= (uint16_t)MCAL_PIE_GROUP_1) &&
       ((uint16_t)group <= (uint16_t)MCAL_PIE_GROUP_12))
    {
        status = MCAL_PIE_STATUS_OK;
    }
    else
    {
        status = MCAL_PIE_STATUS_INV_GROUP;
    }

    return status;
}

static Mcal_PieStatusType IsChannelValid(
    Mcal_PieChannelType channel)
{
    Mcal_PieStatusType status;

    if((channel >= MCAL_PIE_CHANNEL_MIN) &&
       (channel <= MCAL_PIE_CHANNEL_MAX))
    {
        status = MCAL_PIE_STATUS_OK;
    }
    else
    {
        status = MCAL_PIE_STATUS_INV_CHANNEL;
    }

    return status;
}

static uint16_t GetChannelMask(
    Mcal_PieChannelType channel)
{
    uint16_t channelMask;

    channelMask = (uint16_t)((uint32_t)1U <<
                            (channel - 1U));

    return channelMask;
}

static uint16_t ReadIer(
    Mcal_PieGroupType group)
{
    uint16_t value;

    value = 0U;

    switch(group)
    {
        case MCAL_PIE_GROUP_1:
            value = PieCtrlRegs.PIEIER1.all;
            break;

        case MCAL_PIE_GROUP_2:
            value = PieCtrlRegs.PIEIER2.all;
            break;

        case MCAL_PIE_GROUP_3:
            value = PieCtrlRegs.PIEIER3.all;
            break;

        case MCAL_PIE_GROUP_4:
            value = PieCtrlRegs.PIEIER4.all;
            break;

        case MCAL_PIE_GROUP_5:
            value = PieCtrlRegs.PIEIER5.all;
            break;

        case MCAL_PIE_GROUP_6:
            value = PieCtrlRegs.PIEIER6.all;
            break;

        case MCAL_PIE_GROUP_7:
            value = PieCtrlRegs.PIEIER7.all;
            break;

        case MCAL_PIE_GROUP_8:
            value = PieCtrlRegs.PIEIER8.all;
            break;

        case MCAL_PIE_GROUP_9:
            value = PieCtrlRegs.PIEIER9.all;
            break;

        case MCAL_PIE_GROUP_10:
            value = PieCtrlRegs.PIEIER10.all;
            break;

        case MCAL_PIE_GROUP_11:
            value = PieCtrlRegs.PIEIER11.all;
            break;

        case MCAL_PIE_GROUP_12:
            value = PieCtrlRegs.PIEIER12.all;
            break;

        default:
            /* Do nothing. */
            break;
    }

    return value;
}

static void WriteIer(
    Mcal_PieGroupType group,
    uint16_t value)
{
    switch(group)
    {
        case MCAL_PIE_GROUP_1:
            PieCtrlRegs.PIEIER1.all = value;
            break;

        case MCAL_PIE_GROUP_2:
            PieCtrlRegs.PIEIER2.all = value;
            break;

        case MCAL_PIE_GROUP_3:
            PieCtrlRegs.PIEIER3.all = value;
            break;

        case MCAL_PIE_GROUP_4:
            PieCtrlRegs.PIEIER4.all = value;
            break;

        case MCAL_PIE_GROUP_5:
            PieCtrlRegs.PIEIER5.all = value;
            break;

        case MCAL_PIE_GROUP_6:
            PieCtrlRegs.PIEIER6.all = value;
            break;

        case MCAL_PIE_GROUP_7:
            PieCtrlRegs.PIEIER7.all = value;
            break;

        case MCAL_PIE_GROUP_8:
            PieCtrlRegs.PIEIER8.all = value;
            break;

        case MCAL_PIE_GROUP_9:
            PieCtrlRegs.PIEIER9.all = value;
            break;

        case MCAL_PIE_GROUP_10:
            PieCtrlRegs.PIEIER10.all = value;
            break;

        case MCAL_PIE_GROUP_11:
            PieCtrlRegs.PIEIER11.all = value;
            break;

        case MCAL_PIE_GROUP_12:
            PieCtrlRegs.PIEIER12.all = value;
            break;

        default:
            /* Do nothing. */
            break;
    }
}
