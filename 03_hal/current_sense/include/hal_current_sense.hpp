/**
 * @file    hal_current_sense.hpp
 * @brief   Motor phase-current sensing hardware abstraction interface.
 */

#ifndef HAL_CURRENT_SENSE_HPP
#define HAL_CURRENT_SENSE_HPP

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

#include "hal_current_sense_fast.h"

/*==============================================================================
 * Namespace
 *============================================================================*/

namespace Hal
{

/*==============================================================================
 * Public Types
 *============================================================================*/

enum CurrentSenseStatus
{
    CURRENT_SENSE_STATUS_OK = 0U,
    CURRENT_SENSE_STATUS_NOT_INITIALIZED,
    CURRENT_SENSE_STATUS_ALREADY_INITIALIZED,
    CURRENT_SENSE_STATUS_NOT_CALIBRATED,
    CURRENT_SENSE_STATUS_INVALID_CONFIG,
    CURRENT_SENSE_STATUS_CALIBRATION_FAILED,
    CURRENT_SENSE_STATUS_HW_ERROR
};

struct CurrentSenseConfig
{
    float shuntResistanceOhm;
    float amplifierGain;
    float adcReferenceVoltage;
    uint16_t adcResolutionCounts;
    uint16_t calibrationSampleCount;
};

/*==============================================================================
 * Public Classes
 *============================================================================*/

class CurrentSense
{
public:
    /**
     * @brief Constructs the current-sense software object.
     *
     * @note No hardware initialization is performed by the constructor.
     */
    CurrentSense();

    /**
     * @brief Initializes current-sense hardware and scaling parameters.
     *
     * @pre The motor ePWM time-base resource used as the ADC sampling trigger
     *      has been initialized and its peripheral clock is available.
     *
     * @param config Current-sense configuration.
     *
     * @return Current-sense status.
     */
    CurrentSenseStatus Init(
        const CurrentSenseConfig& config);

    /**
     * @brief Performs zero-current offset calibration.
     *
     * The motor power stage shall be in a zero-current safe state before this
     * function is called. Calibration samples are acquired synchronously from
     * phase U and phase V using the configured ePWM-triggered ADC path.
     *
     * @return Current-sense status.
     */
    CurrentSenseStatus Calibrate(void);

    /**
     * @brief Returns whether a valid calibration is available.
     *
     * @return true when calibrated, otherwise false.
     */
    bool IsCalibrated(void) const;

    /**
     * @brief Returns the validated fast-path current-sense configuration.
     *
     * The output parameter is modified only when initialization and
     * calibration are both valid.
     *
     * @param config Receives the CLA-compatible fast-path configuration.
     *
     * @return Current-sense status.
     */
    CurrentSenseStatus GetFastConfig(
        CurrentSenseFastConfigType& config) const;

private:
    CurrentSense(const CurrentSense&);
    CurrentSense& operator=(const CurrentSense&);

    CurrentSenseStatus ValidateConfig(
        const CurrentSenseConfig& config) const;

    float CalculateScale(
        const CurrentSenseConfig& config) const;

    bool initialized_;
    bool calibrated_;

    uint16_t calibrationSampleCount_;

    float offsetU_;
    float offsetV_;

    float scaleU_;
    float scaleV_;
};

} /* namespace Hal */

#endif /* HAL_CURRENT_SENSE_HPP */
