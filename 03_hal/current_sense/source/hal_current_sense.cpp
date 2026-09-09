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
#include "mcal_epwm.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define HAL_CURRENT_SENSE_ACQ_CYCLES        (15U)
#define HAL_CURRENT_SENSE_TRIGGER_PRESCALE  (1U)
#define HAL_CURRENT_SENSE_POLL_LIMIT        (100000UL)

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

namespace
{

Hal::CurrentSenseStatus ConfigureAdcHardware(
    const Bsp_CurrentSenseHwType& hwConfig);

Hal::CurrentSenseStatus ConfigureAdcSoc(
    const Bsp_CurrentSenseInputType& input);

Hal::CurrentSenseStatus ConfigurePwmSocTrigger(void);

Hal::CurrentSenseStatus SetSamplingTriggerState(
    Mcal_EpwmAdcTrigStateType state);

Hal::CurrentSenseStatus WaitForAdcFlag(
    Mcal_AdcIdType adc,
    Mcal_AdcIntType adcInt);

Hal::CurrentSenseStatus CheckAdcOverflow(
    Mcal_AdcIdType adc,
    Mcal_AdcIntType adcInt);

Hal::CurrentSenseStatus ClearCalibrationStatus(
    const Bsp_CurrentSenseHwType& hwConfig);

Hal::CurrentSenseStatus ReadCalibrationSample(
    const Bsp_CurrentSenseHwType& hwConfig,
    uint16_t& rawU,
    uint16_t& rawV);

void CalculateAverageOffset(
    uint32_t sumU,
    uint32_t sumV,
    uint16_t sampleCount,
    float& offsetU,
    float& offsetV);

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

    hwConfig = 0;
    calculatedScale = 0.0F;

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

CurrentSenseStatus CurrentSense::Calibrate(void)
{
    CurrentSenseStatus status;
    CurrentSenseStatus cleanupStatus;
    const Bsp_CurrentSenseHwType * hwConfig;
    uint32_t sumU;
    uint32_t sumV;
    uint16_t rawU;
    uint16_t rawV;
    uint16_t sampleIndex;
    float calculatedOffsetU;
    float calculatedOffsetV;

    status = CURRENT_SENSE_STATUS_OK;
    cleanupStatus = CURRENT_SENSE_STATUS_OK;
    hwConfig = 0;
    sumU = 0UL;
    sumV = 0UL;
    rawU = 0U;
    rawV = 0U;
    sampleIndex = 0U;
    calculatedOffsetU = 0.0F;
    calculatedOffsetV = 0.0F;

    if(initialized_ == false)
    {
        status = CURRENT_SENSE_STATUS_NOT_INITIALIZED;
    }
    else
    {
        /*
         * Any recalibration attempt invalidates the previously published
         * calibration until the complete new sequence succeeds.
         */
        calibrated_ = false;

        hwConfig = Bsp_CurrentSense_GetHwConfig();

        if(hwConfig == 0)
        {
            status = CURRENT_SENSE_STATUS_HW_ERROR;
        }
        else
        {
            status = ClearCalibrationStatus(*hwConfig);
        }
    }

    if(status == CURRENT_SENSE_STATUS_OK)
    {
        status =
            SetSamplingTriggerState(
                MCAL_EPWM_ADC_TRIG_STATE_ENABLE);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == CURRENT_SENSE_STATUS_OK)
    {
        for(sampleIndex = 0U;
            (sampleIndex < calibrationSampleCount_) &&
            (status == CURRENT_SENSE_STATUS_OK);
            sampleIndex++)
        {
            status =
                ReadCalibrationSample(
                    *hwConfig,
                    rawU,
                    rawV);

            if(status == CURRENT_SENSE_STATUS_OK)
            {
                sumU += (uint32_t)rawU;
                sumV += (uint32_t)rawV;
            }
            else
            {
                /* Do nothing. */
            }
        }
    }
    else
    {
        /* Do nothing. */
    }

    if(hwConfig != 0)
    {
        /*
         * Sampling is always disabled after a calibration attempt, including
         * timeout or lower-layer failure paths.
         */
        cleanupStatus =
            SetSamplingTriggerState(
                MCAL_EPWM_ADC_TRIG_STATE_DISABLE);

        if(cleanupStatus != CURRENT_SENSE_STATUS_OK)
        {
            status = CURRENT_SENSE_STATUS_HW_ERROR;
        }
        else
        {
            /* Do nothing. */
        }

        cleanupStatus =
            ClearCalibrationStatus(*hwConfig);

        if(cleanupStatus != CURRENT_SENSE_STATUS_OK)
        {
            status = CURRENT_SENSE_STATUS_HW_ERROR;
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        /* No hardware configuration was available to clean up. */
    }

    if(status == CURRENT_SENSE_STATUS_OK)
    {
        CalculateAverageOffset(
            sumU,
            sumV,
            calibrationSampleCount_,
            calculatedOffsetU,
            calculatedOffsetV);

        offsetU_ = calculatedOffsetU;
        offsetV_ = calculatedOffsetV;

        calibrated_ = true;
    }
    else
    {
        calibrated_ = false;
    }

    return status;
}

bool CurrentSense::IsCalibrated(void) const
{
    return calibrated_;
}

CurrentSenseStatus CurrentSense::GetFastConfig(
    CurrentSenseFastConfigType& config) const
{
    CurrentSenseStatus status;

    if(initialized_ == false)
    {
        status = CURRENT_SENSE_STATUS_NOT_INITIALIZED;
    }
    else
    {
        if(calibrated_ == false)
        {
            status = CURRENT_SENSE_STATUS_NOT_CALIBRATED;
        }
        else
        {
            config.offsetU = offsetU_;
            config.offsetV = offsetV_;
            config.scaleU = scaleU_;
            config.scaleV = scaleV_;

            status = CURRENT_SENSE_STATUS_OK;
        }
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
    Bsp_CurrentSenseStatusType bspStatus;
    Mcal_AdcStatusType adcStatus;

    status = Hal::CURRENT_SENSE_STATUS_OK;

    /*
     * Ensure the board-level ADC resources, including peripheral clocks,
     * are enabled before peripheral configuration is attempted.
     */
    bspStatus = Bsp_CurrentSense_Init();

    if(bspStatus != BSP_CURRENT_SENSE_STATUS_OK)
    {
        status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
    }
    else
    {
        adcStatus =
            Mcal_Adc_Init(
                hwConfig.phaseU.adc);

        if(adcStatus != MCAL_ADC_STATUS_OK)
        {
            status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
        }
        else
        {
            if(hwConfig.phaseV.adc != hwConfig.phaseU.adc)
            {
                adcStatus =
                    Mcal_Adc_Init(
                        hwConfig.phaseV.adc);

                if(adcStatus != MCAL_ADC_STATUS_OK)
                {
                    status =
                        Hal::CURRENT_SENSE_STATUS_HW_ERROR;
                }
                else
                {
                    /* ADC modules initialized successfully. */
                }
            }
            else
            {
                /* The second input uses the already initialized ADC module. */
            }
        }
    }

    if(status == Hal::CURRENT_SENSE_STATUS_OK)
    {
        status =
            ConfigureAdcSoc(
                hwConfig.phaseU);
    }
    else
    {
        /* Do nothing. */
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

    if(status == Hal::CURRENT_SENSE_STATUS_OK)
    {
        status = ConfigurePwmSocTrigger();
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
    Mcal_AdcIntConfigType intConfig;

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
        intConfig.adc = input.adc;
        intConfig.adcInt = MCAL_ADC_INT_1;
        intConfig.sourceEoc = input.soc;

        adcStatus =
            Mcal_Adc_EnableInterrupt(
                &intConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(adcStatus == MCAL_ADC_STATUS_OK)
    {
        status = Hal::CURRENT_SENSE_STATUS_OK;
    }
    else
    {
        status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
    }

    return status;
}

Hal::CurrentSenseStatus ConfigurePwmSocTrigger(void)
{
    Hal::CurrentSenseStatus status;
    Mcal_EpwmStatusType epwmStatus;
    Mcal_EpwmAdcTrigConfigType triggerConfig;

    triggerConfig.module = MCAL_EPWM_1;
    triggerConfig.soc = MCAL_EPWM_ADC_SOCA;
    triggerConfig.source =
        MCAL_EPWM_ADC_TRIG_PERIOD;
    triggerConfig.eventPrescale =
        HAL_CURRENT_SENSE_TRIGGER_PRESCALE;

    epwmStatus =
        Mcal_Epwm_InitAdcTrigger(
            &triggerConfig);

    if(epwmStatus == MCAL_EPWM_STATUS_OK)
    {
        status = Hal::CURRENT_SENSE_STATUS_OK;
    }
    else
    {
        status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
    }

    return status;
}

Hal::CurrentSenseStatus SetSamplingTriggerState(
    Mcal_EpwmAdcTrigStateType state)
{
    Hal::CurrentSenseStatus status;
    Mcal_EpwmStatusType epwmStatus;

    epwmStatus =
        Mcal_Epwm_SetAdcTriggerState(
            MCAL_EPWM_1,
            MCAL_EPWM_ADC_SOCA,
            state);

    if(epwmStatus == MCAL_EPWM_STATUS_OK)
    {
        status = Hal::CURRENT_SENSE_STATUS_OK;
    }
    else
    {
        status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
    }

    return status;
}

Hal::CurrentSenseStatus WaitForAdcFlag(
    Mcal_AdcIdType adc,
    Mcal_AdcIntType adcInt)
{
    Hal::CurrentSenseStatus status;
    Mcal_AdcStatusType adcStatus;
    uint32_t pollCount;
    uint16_t flagSet;

    status = Hal::CURRENT_SENSE_STATUS_OK;
    pollCount = 0UL;
    flagSet = 0U;

    while((flagSet == 0U) &&
          (pollCount < HAL_CURRENT_SENSE_POLL_LIMIT) &&
          (status == Hal::CURRENT_SENSE_STATUS_OK))
    {
        adcStatus =
            Mcal_Adc_IsIntFlagSet(
                adc,
                adcInt,
                &flagSet);

        if(adcStatus != MCAL_ADC_STATUS_OK)
        {
            status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
        }
        else
        {
            pollCount++;
        }
    }

    if((status == Hal::CURRENT_SENSE_STATUS_OK) &&
       (flagSet == 0U))
    {
        status =
            Hal::CURRENT_SENSE_STATUS_CALIBRATION_FAILED;
    }
    else
    {
        /* Preserve the current status. */
    }

    return status;
}

Hal::CurrentSenseStatus CheckAdcOverflow(
    Mcal_AdcIdType adc,
    Mcal_AdcIntType adcInt)
{
    Hal::CurrentSenseStatus status;
    Mcal_AdcStatusType adcStatus;
    uint16_t overflow;

    overflow = 0U;

    adcStatus =
        Mcal_Adc_IsIntOverflow(
            adc,
            adcInt,
            &overflow);

    if(adcStatus != MCAL_ADC_STATUS_OK)
    {
        status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
    }
    else
    {
        if(overflow != 0U)
        {
            status =
                Hal::CURRENT_SENSE_STATUS_CALIBRATION_FAILED;
        }
        else
        {
            status = Hal::CURRENT_SENSE_STATUS_OK;
        }
    }

    return status;
}

Hal::CurrentSenseStatus ClearCalibrationStatus(
    const Bsp_CurrentSenseHwType& hwConfig)
{
    Hal::CurrentSenseStatus status;
    Mcal_AdcStatusType adcStatus;
    bool clearFailed;

    clearFailed = false;

    adcStatus =
        Mcal_Adc_ClearIntFlag(
            hwConfig.phaseU.adc,
            MCAL_ADC_INT_1);

    if(adcStatus != MCAL_ADC_STATUS_OK)
    {
        clearFailed = true;
    }
    else
    {
        /* Do nothing. */
    }

    adcStatus =
        Mcal_Adc_ClearIntFlag(
            hwConfig.phaseV.adc,
            MCAL_ADC_INT_1);

    if(adcStatus != MCAL_ADC_STATUS_OK)
    {
        clearFailed = true;
    }
    else
    {
        /* Do nothing. */
    }

    adcStatus =
        Mcal_Adc_ClearIntOverflow(
            hwConfig.phaseU.adc,
            MCAL_ADC_INT_1);

    if(adcStatus != MCAL_ADC_STATUS_OK)
    {
        clearFailed = true;
    }
    else
    {
        /* Do nothing. */
    }

    adcStatus =
        Mcal_Adc_ClearIntOverflow(
            hwConfig.phaseV.adc,
            MCAL_ADC_INT_1);

    if(adcStatus != MCAL_ADC_STATUS_OK)
    {
        clearFailed = true;
    }
    else
    {
        /* Do nothing. */
    }

    if(clearFailed == false)
    {
        status = Hal::CURRENT_SENSE_STATUS_OK;
    }
    else
    {
        status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
    }

    return status;
}

Hal::CurrentSenseStatus ReadCalibrationSample(
    const Bsp_CurrentSenseHwType& hwConfig,
    uint16_t& rawU,
    uint16_t& rawV)
{
    Hal::CurrentSenseStatus status;
    Hal::CurrentSenseStatus clearStatus;
    Mcal_AdcStatusType adcStatus;

    status =
        WaitForAdcFlag(
            hwConfig.phaseU.adc,
            MCAL_ADC_INT_1);

    if(status == Hal::CURRENT_SENSE_STATUS_OK)
    {
        status =
            WaitForAdcFlag(
                hwConfig.phaseV.adc,
                MCAL_ADC_INT_1);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::CURRENT_SENSE_STATUS_OK)
    {
        status =
            CheckAdcOverflow(
                hwConfig.phaseU.adc,
                MCAL_ADC_INT_1);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::CURRENT_SENSE_STATUS_OK)
    {
        status =
            CheckAdcOverflow(
                hwConfig.phaseV.adc,
                MCAL_ADC_INT_1);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::CURRENT_SENSE_STATUS_OK)
    {
        adcStatus =
            Mcal_Adc_GetResult(
                hwConfig.phaseU.adc,
                hwConfig.phaseU.soc,
                &rawU);

        if(adcStatus != MCAL_ADC_STATUS_OK)
        {
            status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
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

    if(status == Hal::CURRENT_SENSE_STATUS_OK)
    {
        adcStatus =
            Mcal_Adc_GetResult(
                hwConfig.phaseV.adc,
                hwConfig.phaseV.soc,
                &rawV);

        if(adcStatus != MCAL_ADC_STATUS_OK)
        {
            status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
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

    /*
     * Clear completion state after every sample attempt so that the next
     * iteration waits for a new conversion event.
     */
    clearStatus =
        ClearCalibrationStatus(
            hwConfig);

    if(clearStatus != Hal::CURRENT_SENSE_STATUS_OK)
    {
        status = Hal::CURRENT_SENSE_STATUS_HW_ERROR;
    }
    else
    {
        /* Preserve the sample status. */
    }

    return status;
}

void CalculateAverageOffset(
    uint32_t sumU,
    uint32_t sumV,
    uint16_t sampleCount,
    float& offsetU,
    float& offsetV)
{
    offsetU =
        (float)sumU /
        (float)sampleCount;

    offsetV =
        (float)sumV /
        (float)sampleCount;
}

}
