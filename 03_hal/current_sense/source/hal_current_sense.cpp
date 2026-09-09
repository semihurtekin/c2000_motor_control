/**
 * @file    hal_current_sense.cpp
 * @brief   Motor phase-current sensing hardware abstraction implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "hal_current_sense.hpp"

#include "bsp_current_sense.h"
#include "mcal_adc.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define HAL_CURRENT_SENSE_ACQ_CYCLES    (15U)

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

namespace
{

Hal::CurrentSenseStatus ConfigureAdcHardware(
    const Bsp_CurrentSenseHwType& hwConfig);

Hal::CurrentSenseStatus ConfigureAdcSoc(
    const Bsp_CurrentSenseInputType& input);

}

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

namespace Hal
{

CurrentSense::CurrentSense()
    : initialized_(false),
      calibrated_(false),
      calibrationSampleCount_(0U),
      offsetU_(0.0F),
      offsetV_(0.0F),
      scaleU_(0.0F),
      scaleV_(0.0F)
{
}

CurrentSenseStatus CurrentSense::Init(
    const CurrentSenseConfig& config)
{
    CurrentSenseStatus status;
    const Bsp_CurrentSenseHwType * hwConfig;
    float calculatedScale;

    status = ValidateConfig(config);

    if(status == CURRENT_SENSE_STATUS_OK)
    {
        calculatedScale = CalculateScale(config);

        hwConfig = Bsp_CurrentSense_GetHwConfig();

        if(hwConfig == 0)
        {
            status = CURRENT_SENSE_STATUS_HW_ERROR;
        }
        else
        {
            status = ConfigureAdcHardware(*hwConfig);
        }
    }
    else
    {
        /* Do nothing. */
    }

    if(status == CURRENT_SENSE_STATUS_OK)
    {
        calibrationSampleCount_ =
            config.calibrationSampleCount;

        scaleU_ = calculatedScale;
        scaleV_ = calculatedScale;

        offsetU_ = 0.0F;
        offsetV_ = 0.0F;

        calibrated_ = false;
        initialized_ = true;
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

CurrentSenseStatus CurrentSense::ValidateConfig(
    const CurrentSenseConfig& config) const
{
    CurrentSenseStatus status;

    status = CURRENT_SENSE_STATUS_OK;

    if(initialized_ == true)
    {
        status =
            CURRENT_SENSE_STATUS_ALREADY_INITIALIZED;
    }
    else
    {
        if((config.shuntResistanceOhm <= 0.0F) ||
           (config.amplifierGain <= 0.0F) ||
           (config.adcReferenceVoltage <= 0.0F) ||
           (config.adcResolutionCounts == 0U) ||
           (config.calibrationSampleCount == 0U))
        {
            status =
                CURRENT_SENSE_STATUS_INVALID_CONFIG;
        }
        else
        {
            /* Configuration is valid. */
        }
    }

    return status;
}

float CurrentSense::CalculateScale(
    const CurrentSenseConfig& config) const
{
    float scale;

    scale =
        config.adcReferenceVoltage /
        ((float)config.adcResolutionCounts *
         config.amplifierGain *
         config.shuntResistanceOhm);

    return scale;
}

} /* namespace Hal */

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

namespace
{

Hal::CurrentSenseStatus ConfigureAdcHardware(
    const Bsp_CurrentSenseHwType& hwConfig)
{
    Hal::CurrentSenseStatus status;
    Mcal_AdcStatusType adcStatus;

    status = Hal::CURRENT_SENSE_STATUS_OK;

    adcStatus =
        Mcal_Adc_Init(
            hwConfig.phaseU.adc);

    if((adcStatus == MCAL_ADC_STATUS_OK) &&
       (hwConfig.phaseV.adc != hwConfig.phaseU.adc))
    {
        adcStatus =
            Mcal_Adc_Init(
                hwConfig.phaseV.adc);
    }
    else
    {
        /* Do nothing. */
    }

    if(adcStatus != MCAL_ADC_STATUS_OK)
    {
        status =
            Hal::CURRENT_SENSE_STATUS_HW_ERROR;
    }
    else
    {
        /*
         * Mcal_Adc_Init() powers up the ADC module. The required power-up
         * settling time shall elapse before ePWM SOCA generation is enabled.
         */
        status =
            ConfigureAdcSoc(
                hwConfig.phaseU);
    }

    if(status == Hal::CURRENT_SENSE_STATUS_OK)
    {
        status =
            ConfigureAdcSoc(
                hwConfig.phaseV);
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Hal::CurrentSenseStatus ConfigureAdcSoc(
    const Bsp_CurrentSenseInputType& input)
{
    Hal::CurrentSenseStatus status;
    Mcal_AdcStatusType adcStatus;
    Mcal_AdcSocConfigType socConfig;

    socConfig.adc = input.adc;
    socConfig.soc = input.soc;
    socConfig.channel = input.channel;
    socConfig.trigger =
        MCAL_ADC_TRIG_EPWM1_SOCA;
    socConfig.acquisitionCycles =
        HAL_CURRENT_SENSE_ACQ_CYCLES;

    adcStatus =
        Mcal_Adc_InitSoc(
            &socConfig);

    if(adcStatus == MCAL_ADC_STATUS_OK)
    {
        status =
            Hal::CURRENT_SENSE_STATUS_OK;
    }
    else
    {
        status =
            Hal::CURRENT_SENSE_STATUS_HW_ERROR;
    }

    return status;
}

}

