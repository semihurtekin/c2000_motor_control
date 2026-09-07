/**
 * @file    bsp_current_sense.c
 * @brief   Current-sense hardware mapping for LAUNCHXL-F28379D.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "bsp_current_sense.h"

#include "platform_clock.h"

/*==============================================================================
 * Private Variables
 *============================================================================*/

static const Bsp_CurrentSenseHwType CurrentSenseHw =
{
    {
        MCAL_ADC_A,
        MCAL_ADC_SOC_0,
        MCAL_ADC_CHANNEL_2
    },
    {
        MCAL_ADC_B,
        MCAL_ADC_SOC_0,
        MCAL_ADC_CHANNEL_2
    }
};

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static Bsp_CurrentSenseStatusType EnableAdcClock(
    Mcal_AdcIdType adc);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Bsp_CurrentSenseStatusType Bsp_CurrentSense_Init(void)
{
    Bsp_CurrentSenseStatusType status;

    status = EnableAdcClock(
        CurrentSenseHw.phaseU.adc);

    if(status == BSP_CURRENT_SENSE_STATUS_OK)
    {
        status = EnableAdcClock(
            CurrentSenseHw.phaseV.adc);
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

const Bsp_CurrentSenseHwType * Bsp_CurrentSense_GetHwConfig(void)
{
    return &CurrentSenseHw;
}

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

static Bsp_CurrentSenseStatusType EnableAdcClock(
    Mcal_AdcIdType adc)
{
    Bsp_CurrentSenseStatusType status;
    Platform_ClockStatusType clockStatus;

    status = BSP_CURRENT_SENSE_STATUS_INIT_FAILED;
    clockStatus = PLATFORM_CLOCK_STATUS_INVALID_PARAM;

    switch(adc)
    {
        case MCAL_ADC_A:
            clockStatus =
                Platform_ClockEnableAdc(
                    PLATFORM_ADC_MODULE_A);
            break;

        case MCAL_ADC_B:
            clockStatus =
                Platform_ClockEnableAdc(
                    PLATFORM_ADC_MODULE_B);
            break;

        case MCAL_ADC_C:
            clockStatus =
                Platform_ClockEnableAdc(
                    PLATFORM_ADC_MODULE_C);
            break;

        case MCAL_ADC_D:
            clockStatus =
                Platform_ClockEnableAdc(
                    PLATFORM_ADC_MODULE_D);
            break;

        default:
            /* Invalid ADC mapping. */
            break;
    }

    if(clockStatus == PLATFORM_CLOCK_STATUS_OK)
    {
        status = BSP_CURRENT_SENSE_STATUS_OK;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}
