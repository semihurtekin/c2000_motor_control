/**
 * @file    mcal_xbar.c
 * @brief   F28379D Input X-BAR MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "mcal_xbar.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_XBAR_GPIO_MAX    (168U)

/*==============================================================================
 * Private Types
 *============================================================================*/

/*==============================================================================
 * Private Variables
 *============================================================================*/

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static Mcal_XbarStatusType IsInputValid(
    Mcal_XbarInputType input);

static Mcal_XbarStatusType IsPinValid(
    uint16_t gpioPin);

static void WriteInputSelect(
    Mcal_XbarInputType input,
    uint16_t gpioPin);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_XbarStatusType Mcal_Xbar_SetInput(
    Mcal_XbarInputType input,
    uint16_t gpioPin)
{
    Mcal_XbarStatusType status;

    status = IsInputValid(input);

    if(status == MCAL_XBAR_STATUS_OK)
    {
        status = IsPinValid(gpioPin);

        if(status == MCAL_XBAR_STATUS_OK)
        {
            EALLOW;
            WriteInputSelect(input, gpioPin);
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

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

static Mcal_XbarStatusType IsInputValid(
    Mcal_XbarInputType input)
{
    Mcal_XbarStatusType status;

    if(((uint16_t)input >= (uint16_t)MCAL_XBAR_INPUT_1) &&
       ((uint16_t)input <= (uint16_t)MCAL_XBAR_INPUT_14))
    {
        status = MCAL_XBAR_STATUS_OK;
    }
    else
    {
        status = MCAL_XBAR_STATUS_INV_INPUT;
    }

    return status;
}

static Mcal_XbarStatusType IsPinValid(
    uint16_t gpioPin)
{
    Mcal_XbarStatusType status;

    if(gpioPin <= MCAL_XBAR_GPIO_MAX)
    {
        status = MCAL_XBAR_STATUS_OK;
    }
    else
    {
        status = MCAL_XBAR_STATUS_INV_PIN;
    }

    return status;
}

static void WriteInputSelect(
    Mcal_XbarInputType input,
    uint16_t gpioPin)
{
    switch(input)
    {
        case MCAL_XBAR_INPUT_1:
            InputXbarRegs.INPUT1SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_2:
            InputXbarRegs.INPUT2SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_3:
            InputXbarRegs.INPUT3SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_4:
            InputXbarRegs.INPUT4SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_5:
            InputXbarRegs.INPUT5SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_6:
            InputXbarRegs.INPUT6SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_7:
            InputXbarRegs.INPUT7SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_8:
            InputXbarRegs.INPUT8SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_9:
            InputXbarRegs.INPUT9SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_10:
            InputXbarRegs.INPUT10SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_11:
            InputXbarRegs.INPUT11SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_12:
            InputXbarRegs.INPUT12SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_13:
            InputXbarRegs.INPUT13SELECT = gpioPin;
            break;

        case MCAL_XBAR_INPUT_14:
            InputXbarRegs.INPUT14SELECT = gpioPin;
            break;

        default:
            /* Do nothing. */
            break;
    }
}
