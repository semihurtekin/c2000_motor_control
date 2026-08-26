/**
 * @file    mcal_xbar.c
 * @brief   F28379D X-BAR MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "mcal_xbar.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_XBAR_GPIO_MAX          (168U)
#define MCAL_XBAR_MUX_CFG_MASK      (0x3UL)
#define MCAL_XBAR_MUX_HIGH_OR_LOW   (1UL)

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

static Mcal_XbarStatusType IsEpwmTripValid(
    Mcal_XbarEpwmTripType trip);

static Mcal_XbarStatusType IsCmpssValid(
    Mcal_XbarCmpssType cmpss);

static Mcal_XbarStatusType IsCmpssOutputValid(
    Mcal_XbarCmpssOutputType output);

static void WriteInputSelect(
    Mcal_XbarInputType input,
    uint16_t gpioPin);

static void WriteEpwmTripMux(
    Mcal_XbarEpwmTripType trip,
    uint16_t mux,
    uint16_t muxConfig);

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

Mcal_XbarStatusType Mcal_Xbar_AddCmpssTripSource(
    Mcal_XbarEpwmTripType trip,
    Mcal_XbarCmpssType cmpss,
    Mcal_XbarCmpssOutputType output)
{
    Mcal_XbarStatusType status;
    uint16_t mux;
    uint16_t muxConfig;

    status = IsEpwmTripValid(trip);

    if(status == MCAL_XBAR_STATUS_OK)
    {
        status = IsCmpssValid(cmpss);

        if(status == MCAL_XBAR_STATUS_OK)
        {
            status = IsCmpssOutputValid(output);

            if(status == MCAL_XBAR_STATUS_OK)
            {
                mux = (uint16_t)(((uint16_t)cmpss - 1U) * 2U);
                muxConfig = 0U;

                if(output == MCAL_XBAR_CMPSS_LOW)
                {
                    mux++;
                }
                else if(output == MCAL_XBAR_CMPSS_HIGH_OR_LOW)
                {
                    muxConfig = (uint16_t)MCAL_XBAR_MUX_HIGH_OR_LOW;
                }
                else
                {
                    /* HIGH uses the default mux configuration. */
                }

                EALLOW;
                WriteEpwmTripMux(trip, mux, muxConfig);
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

static Mcal_XbarStatusType IsEpwmTripValid(
    Mcal_XbarEpwmTripType trip)
{
    Mcal_XbarStatusType status;

    if((trip == MCAL_XBAR_EPWM_TRIP_4) ||
       (trip == MCAL_XBAR_EPWM_TRIP_5) ||
       (trip == MCAL_XBAR_EPWM_TRIP_7) ||
       (trip == MCAL_XBAR_EPWM_TRIP_8) ||
       (trip == MCAL_XBAR_EPWM_TRIP_9) ||
       (trip == MCAL_XBAR_EPWM_TRIP_10) ||
       (trip == MCAL_XBAR_EPWM_TRIP_11) ||
       (trip == MCAL_XBAR_EPWM_TRIP_12))
    {
        status = MCAL_XBAR_STATUS_OK;
    }
    else
    {
        status = MCAL_XBAR_STATUS_INV_TRIP;
    }

    return status;
}

static Mcal_XbarStatusType IsCmpssValid(
    Mcal_XbarCmpssType cmpss)
{
    Mcal_XbarStatusType status;

    if(((uint16_t)cmpss >= (uint16_t)MCAL_XBAR_CMPSS_1) &&
       ((uint16_t)cmpss <= (uint16_t)MCAL_XBAR_CMPSS_8))
    {
        status = MCAL_XBAR_STATUS_OK;
    }
    else
    {
        status = MCAL_XBAR_STATUS_INV_CMPSS;
    }

    return status;
}

static Mcal_XbarStatusType IsCmpssOutputValid(
    Mcal_XbarCmpssOutputType output)
{
    Mcal_XbarStatusType status;

    if((output == MCAL_XBAR_CMPSS_HIGH) ||
       (output == MCAL_XBAR_CMPSS_LOW) ||
       (output == MCAL_XBAR_CMPSS_HIGH_OR_LOW))
    {
        status = MCAL_XBAR_STATUS_OK;
    }
    else
    {
        status = MCAL_XBAR_STATUS_INV_SOURCE;
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

static void WriteEpwmTripMux(
    Mcal_XbarEpwmTripType trip,
    uint16_t mux,
    uint16_t muxConfig)
{
    uint32_t shift;
    uint32_t mask;
    uint32_t value;
    uint32_t enableMask;

    shift = (uint32_t)mux * 2UL;
    mask = MCAL_XBAR_MUX_CFG_MASK << shift;
    value = (uint32_t)muxConfig << shift;
    enableMask = 1UL << (uint32_t)mux;

    switch(trip)
    {
        case MCAL_XBAR_EPWM_TRIP_4:
            EPwmXbarRegs.TRIP4MUX0TO15CFG.all =
                (EPwmXbarRegs.TRIP4MUX0TO15CFG.all & ~mask) | value;
            EPwmXbarRegs.TRIP4MUXENABLE.all |= enableMask;
            break;

        case MCAL_XBAR_EPWM_TRIP_5:
            EPwmXbarRegs.TRIP5MUX0TO15CFG.all =
                (EPwmXbarRegs.TRIP5MUX0TO15CFG.all & ~mask) | value;
            EPwmXbarRegs.TRIP5MUXENABLE.all |= enableMask;
            break;

        case MCAL_XBAR_EPWM_TRIP_7:
            EPwmXbarRegs.TRIP7MUX0TO15CFG.all =
                (EPwmXbarRegs.TRIP7MUX0TO15CFG.all & ~mask) | value;
            EPwmXbarRegs.TRIP7MUXENABLE.all |= enableMask;
            break;

        case MCAL_XBAR_EPWM_TRIP_8:
            EPwmXbarRegs.TRIP8MUX0TO15CFG.all =
                (EPwmXbarRegs.TRIP8MUX0TO15CFG.all & ~mask) | value;
            EPwmXbarRegs.TRIP8MUXENABLE.all |= enableMask;
            break;

        case MCAL_XBAR_EPWM_TRIP_9:
            EPwmXbarRegs.TRIP9MUX0TO15CFG.all =
                (EPwmXbarRegs.TRIP9MUX0TO15CFG.all & ~mask) | value;
            EPwmXbarRegs.TRIP9MUXENABLE.all |= enableMask;
            break;

        case MCAL_XBAR_EPWM_TRIP_10:
            EPwmXbarRegs.TRIP10MUX0TO15CFG.all =
                (EPwmXbarRegs.TRIP10MUX0TO15CFG.all & ~mask) | value;
            EPwmXbarRegs.TRIP10MUXENABLE.all |= enableMask;
            break;

        case MCAL_XBAR_EPWM_TRIP_11:
            EPwmXbarRegs.TRIP11MUX0TO15CFG.all =
                (EPwmXbarRegs.TRIP11MUX0TO15CFG.all & ~mask) | value;
            EPwmXbarRegs.TRIP11MUXENABLE.all |= enableMask;
            break;

        case MCAL_XBAR_EPWM_TRIP_12:
            EPwmXbarRegs.TRIP12MUX0TO15CFG.all =
                (EPwmXbarRegs.TRIP12MUX0TO15CFG.all & ~mask) | value;
            EPwmXbarRegs.TRIP12MUXENABLE.all |= enableMask;
            break;

        default:
            /* Do nothing. */
            break;
    }
}
