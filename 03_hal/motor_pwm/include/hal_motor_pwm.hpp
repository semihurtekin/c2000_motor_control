/**
 * @file    hal_motor_pwm.hpp
 * @brief   Three-phase motor PWM hardware abstraction interface.
 */

#ifndef HAL_MOTOR_PWM_HPP
#define HAL_MOTOR_PWM_HPP

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

/*==============================================================================
 * Namespace
 *============================================================================*/

namespace Hal
{

/*==============================================================================
 * Public Types
 *============================================================================*/

enum MotorPwmStatus
{
    MOTOR_PWM_STATUS_OK = 0U,
    MOTOR_PWM_STATUS_NOT_INITIALIZED,
    MOTOR_PWM_STATUS_ALREADY_INITIALIZED,
    MOTOR_PWM_STATUS_INVALID_CONFIG,
    MOTOR_PWM_STATUS_INVALID_DUTY,
    MOTOR_PWM_STATUS_HW_ERROR
};

struct MotorPwmConfig
{
    uint32_t frequencyHz;
    uint32_t deadTimeNs;
};

struct MotorPwmDuty
{
    float phaseU;
    float phaseV;
    float phaseW;
};

/*==============================================================================
 * Public Classes
 *============================================================================*/

/**
 * @brief Provides synchronized three-phase motor PWM control.
 */
class MotorPwm
{
public:
    /**
     * @brief Constructs the motor PWM software object.
     *
     * @note No hardware initialization is performed by the constructor.
     */
    MotorPwm();

    /**
     * @brief Initializes the three-phase motor PWM hardware.
     *
     * @param config Motor PWM timing configuration.
     *
     * @return HAL motor PWM status.
     */
    MotorPwmStatus Init(
        const MotorPwmConfig& config);

private:
    struct MotorPwmTiming
    {
        uint16_t period;
        uint16_t deadBand;
    };

    MotorPwm(const MotorPwm&);
    MotorPwm& operator=(const MotorPwm&);

    MotorPwmStatus ValidateConfig(
        const MotorPwmConfig& config) const;

    MotorPwmStatus CalculateTiming(
        const MotorPwmConfig& config,
        MotorPwmTiming& timing) const;

    MotorPwmStatus ConfigureHardware(
        const MotorPwmTiming& timing);

    bool initialized_;
    bool enabled_;
    uint16_t period_;
};

} /* namespace Hal */

#endif /* HAL_MOTOR_PWM_HPP */

