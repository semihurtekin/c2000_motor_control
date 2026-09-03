/**
 * @file    hal_motor_pwm.cpp
 * @brief   Three-phase motor PWM hardware abstraction implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "hal_motor_pwm.hpp"

#include "bsp_motor_hw.h"
#include "mcal_epwm.h"
#include "platform_clock.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define HAL_MOTOR_PWM_NS_PER_SEC    (1000000000ULL)
#define HAL_MOTOR_PWM_PERIOD_MAX    (65535UL)
#define HAL_MOTOR_PWM_DB_MAX        (0x3FFFUL)

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

namespace
{

Hal::MotorPwmStatus InitPhase(
    Mcal_EpwmIdType module,
    uint16_t period,
    uint16_t deadBand);

Hal::MotorPwmStatus WriteCompareValues(
    const Bsp_MotorPwmHwType& hwConfig,
    uint16_t phaseUCompare,
    uint16_t phaseVCompare,
    uint16_t phaseWCompare);

Hal::MotorPwmStatus ClearAllTrips(
    const Bsp_MotorPwmHwType& hwConfig);

Hal::MotorPwmStatus ForceAllTrips(
    const Bsp_MotorPwmHwType& hwConfig);

Hal::MotorPwmStatus AreAllTripsInactive(
    const Bsp_MotorPwmHwType& hwConfig,
    bool& allInactive);

}

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

namespace Hal
{

MotorPwm::MotorPwm()
    : initialized_(false),
      enabled_(false),
      period_(0U)
{
}

MotorPwmStatus MotorPwm::Init(
    const MotorPwmConfig& config)
{
    MotorPwmStatus status;
    MotorPwmTiming timing;

    status = ValidateConfig(config);

    if(status == MOTOR_PWM_STATUS_OK)
    {
        status = CalculateTiming(
            config,
            timing);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == MOTOR_PWM_STATUS_OK)
    {
        status = ConfigureHardware(timing);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == MOTOR_PWM_STATUS_OK)
    {
        period_ = timing.period;
        initialized_ = true;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

MotorPwmStatus MotorPwm::SetDuty(
    const MotorPwmDuty& dutyCycle)
{
    MotorPwmStatus status;
    uint16_t phaseUCompare;
    uint16_t phaseVCompare;
    uint16_t phaseWCompare;
    const Bsp_MotorPwmHwType * hwConfig;

    status = MOTOR_PWM_STATUS_OK;

    if(initialized_ == false)
    {
        status = MOTOR_PWM_STATUS_NOT_INITIALIZED;
    }
    else
    {
        status = ValidateDuty(dutyCycle);
    }

    if(status == MOTOR_PWM_STATUS_OK)
    {
        phaseUCompare =
            CalculateCompare(dutyCycle.phaseU);

        phaseVCompare =
            CalculateCompare(dutyCycle.phaseV);

        phaseWCompare =
            CalculateCompare(dutyCycle.phaseW);

        hwConfig = Bsp_MotorHw_GetPwmConfig();

        if(hwConfig == 0)
        {
            status = MOTOR_PWM_STATUS_HW_ERROR;
        }
        else
        {
            status = WriteCompareValues(
                *hwConfig,
                phaseUCompare,
                phaseVCompare,
                phaseWCompare);
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

MotorPwmStatus MotorPwm::Enable(void)
{
    MotorPwmStatus status;

    if(initialized_ == false)
    {
        status = MOTOR_PWM_STATUS_NOT_INITIALIZED;
    }
    else
    {
        if(enabled_ == true)
        {
            status = MOTOR_PWM_STATUS_OK;
        }
        else
        {
            status = OutputPwmEnable();
        }
    }

    return status;
}

MotorPwmStatus MotorPwm::Disable(void)
{
    MotorPwmStatus status;
    
    if(initialized_ == false)
    {
        status = MOTOR_PWM_STATUS_NOT_INITIALIZED;
    }
    else
    {
        status = OutputPwmDisable();
    }

    return status;
}

bool MotorPwm::IsEnabled(void) const
{
    return enabled_;
}

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

MotorPwmStatus MotorPwm::ValidateConfig(
    const MotorPwmConfig& config) const
{
    MotorPwmStatus status;

    status = MOTOR_PWM_STATUS_OK;

    if(initialized_ == true)
    {
        status = MOTOR_PWM_STATUS_ALREADY_INITIALIZED;
    }
    else
    {
        if(config.frequencyHz == 0UL)
        {
            status = MOTOR_PWM_STATUS_INVALID_CONFIG;
        }
        else
        {
            /* Additional physical limits are validated during timing conversion. */
        }
    }

    return status;
}

MotorPwmStatus MotorPwm::CalculateTiming(
    const MotorPwmConfig& config,
    MotorPwmTiming& timing) const
{
    MotorPwmStatus status;
    uint32_t epwmClkHz;
    uint32_t calculatedPeriod;
    uint32_t calculatedDeadBand;
    uint64_t deadBandNumerator;

    status = MOTOR_PWM_STATUS_OK;

    epwmClkHz = Platform_ClockGetEpwmClkHz();

    calculatedPeriod =
        (epwmClkHz / 2UL) /
        config.frequencyHz;

    if((calculatedPeriod == 0UL) ||
       (calculatedPeriod > HAL_MOTOR_PWM_PERIOD_MAX))
    {
        status = MOTOR_PWM_STATUS_INVALID_CONFIG;
    }
    else
    {
        deadBandNumerator =
            ((uint64_t)config.deadTimeNs *
             (uint64_t)epwmClkHz);

        calculatedDeadBand =
            (uint32_t)(
                (deadBandNumerator +
                 (HAL_MOTOR_PWM_NS_PER_SEC - 1ULL)) /
                HAL_MOTOR_PWM_NS_PER_SEC);

        if(calculatedDeadBand > HAL_MOTOR_PWM_DB_MAX)
        {
            status = MOTOR_PWM_STATUS_INVALID_CONFIG;
        }
        else
        {
            timing.period =
                (uint16_t)calculatedPeriod;

            timing.deadBand =
                (uint16_t)calculatedDeadBand;
        }
    }

    return status;
}

MotorPwmStatus MotorPwm::ConfigureHardware(
    const MotorPwmTiming& timing)
{
    MotorPwmStatus status;
    const Bsp_MotorPwmHwType * hwConfig;
    Mcal_EpwmStatusType mcalStatus;

    status = MOTOR_PWM_STATUS_OK;

    hwConfig = Bsp_MotorHw_GetPwmConfig();

    if(hwConfig == 0)
    {
        status = MOTOR_PWM_STATUS_HW_ERROR;
    }
    else
    {
        mcalStatus =
            Mcal_Epwm_SetTbClkSync(
                MCAL_EPWM_TBCLK_SYNC_DISABLE);

        if(mcalStatus != MCAL_EPWM_STATUS_OK)
        {
            status = MOTOR_PWM_STATUS_HW_ERROR;
        }
        else
        {
            status = InitPhase(
                hwConfig->phaseU.module,
                timing.period,
                timing.deadBand);

            if(status == MOTOR_PWM_STATUS_OK)
            {
                status = InitPhase(
                    hwConfig->phaseV.module,
                    timing.period,
                    timing.deadBand);
            }
            else
            {
                /* Do nothing. */
            }

            if(status == MOTOR_PWM_STATUS_OK)
            {
                status = InitPhase(
                    hwConfig->phaseW.module,
                    timing.period,
                    timing.deadBand);
            }
            else
            {
                /* Do nothing. */
            }

            if(status == MOTOR_PWM_STATUS_OK)
            {
                mcalStatus =
                    Mcal_Epwm_SetTbClkSync(
                        MCAL_EPWM_TBCLK_SYNC_ENABLE);

                if(mcalStatus != MCAL_EPWM_STATUS_OK)
                {
                    status =
                        MOTOR_PWM_STATUS_HW_ERROR;
                }
                else
                {
                    /* Initialization completed successfully. */
                }
            }
            else
            {
                /*
                 * Keep TBCLKSYNC disabled when initialization is incomplete.
                 */
            }
        }
    }

    return status;
}

MotorPwmStatus MotorPwm::ValidateDuty(
    const MotorPwmDuty& dutyCycle) const
{
    MotorPwmStatus status;

    status = MOTOR_PWM_STATUS_INVALID_DUTY;

    if(((dutyCycle.phaseU >= 0.0F) &&
        (dutyCycle.phaseU <= 1.0F)) &&
       ((dutyCycle.phaseV >= 0.0F) &&
        (dutyCycle.phaseV <= 1.0F)) &&
       ((dutyCycle.phaseW >= 0.0F) &&
        (dutyCycle.phaseW <= 1.0F)))
    {
        status = MOTOR_PWM_STATUS_OK;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

uint16_t MotorPwm::CalculateCompare(
    float duty) const
{
    uint16_t compareResult;

    compareResult =
        (uint16_t)((duty * (float)period_) + 0.5F);

    return compareResult;
}

MotorPwmStatus MotorPwm::OutputPwmEnable(void)
{
    MotorPwmStatus status;
    MotorPwmStatus rollbackStatus;
    const Bsp_MotorPwmHwType * hwConfig;
    bool allTripsInactive;

    status = MOTOR_PWM_STATUS_HW_ERROR;
    allTripsInactive = false;

    hwConfig = Bsp_MotorHw_GetPwmConfig();

    if(hwConfig != 0)
    {
        status = ClearAllTrips(*hwConfig);

        if(status == MOTOR_PWM_STATUS_OK)
        {
            status = AreAllTripsInactive(
                *hwConfig,
                allTripsInactive);
        }
        else
        {
            /* Do nothing. */
        }

        if((status == MOTOR_PWM_STATUS_OK) &&
           (allTripsInactive == true))
        {
            enabled_ = true;
        }
        else
        {
            /*
             * Restore the safe hardware state if the enable sequence
             * did not complete successfully.
             */
            rollbackStatus = ForceAllTrips(*hwConfig);

            enabled_ = false;

            if(rollbackStatus != MOTOR_PWM_STATUS_OK)
            {
                status = MOTOR_PWM_STATUS_HW_ERROR;
            }
            else
            {
                /*
                 * Preserve the original enable failure status.
                 */
                if(status == MOTOR_PWM_STATUS_OK)
                {
                    status = MOTOR_PWM_STATUS_HW_ERROR;
                }
                else
                {
                    /* Do nothing. */
                }
            }
        }
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

MotorPwmStatus MotorPwm::OutputPwmDisable(void)
{
    MotorPwmStatus status;
    const Bsp_MotorPwmHwType * hwConfig;

    hwConfig = Bsp_MotorHw_GetPwmConfig();

    if(hwConfig != 0)
    {
        status = ForceAllTrips(*hwConfig);
        enabled_ = false;
    }
    else 
    {
        status = MOTOR_PWM_STATUS_HW_ERROR;
        enabled_ = false;
    }

    return status;
}

} /* namespace Hal */

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

namespace
{

Hal::MotorPwmStatus InitPhase(
    Mcal_EpwmIdType module,
    uint16_t period,
    uint16_t deadBand)
{
    Hal::MotorPwmStatus status;
    Mcal_EpwmStatusType mcalStatus;
    Mcal_EpwmTbConfigType tbConfig;
    Mcal_EpwmCompareConfigType compareConfig;
    Mcal_EpwmDeadBandConfigType deadBandConfig;

    status = Hal::MOTOR_PWM_STATUS_OK;

    tbConfig.module = module;
    tbConfig.period = period;
    tbConfig.mode = MCAL_EPWM_COUNT_UP_DOWN;
    tbConfig.clkDiv = MCAL_EPWM_CLKDIV_1;
    tbConfig.hsClkDiv = MCAL_EPWM_HSCLKDIV_1;

    mcalStatus =
        Mcal_Epwm_InitTimeBase(&tbConfig);

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        compareConfig.module = module;
        compareConfig.compareA = period / 2U;

        mcalStatus =
            Mcal_Epwm_InitCompareA(
                &compareConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        deadBandConfig.module = module;
        deadBandConfig.risingDelay = deadBand;
        deadBandConfig.fallingDelay = deadBand;

        mcalStatus =
            Mcal_Epwm_InitDeadBand(
                &deadBandConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        mcalStatus =
            Mcal_Epwm_InitTrip(module);
    }
    else
    {
        /* Do nothing. */
    }

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        mcalStatus =
            Mcal_Epwm_ForceTrip(module);
    }
    else
    {
        /* Do nothing. */
    }

    if(mcalStatus != MCAL_EPWM_STATUS_OK)
    {
        status =
            Hal::MOTOR_PWM_STATUS_HW_ERROR;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Hal::MotorPwmStatus WriteCompareValues(
    const Bsp_MotorPwmHwType& hwConfig,
    uint16_t phaseUCompare,
    uint16_t phaseVCompare,
    uint16_t phaseWCompare)
{
    Hal::MotorPwmStatus status;
    Mcal_EpwmStatusType mcalStatus;

    status = Hal::MOTOR_PWM_STATUS_OK;

    mcalStatus =
        Mcal_Epwm_SetCompareA(
            hwConfig.phaseU.module,
            phaseUCompare);

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        mcalStatus =
            Mcal_Epwm_SetCompareA(
                hwConfig.phaseV.module,
                phaseVCompare);
    }
    else
    {
        /* Do nothing. */
    }

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        mcalStatus =
            Mcal_Epwm_SetCompareA(
                hwConfig.phaseW.module,
                phaseWCompare);
    }
    else
    {
        /* Do nothing. */
    }

    if(mcalStatus != MCAL_EPWM_STATUS_OK)
    {
        status =
            Hal::MOTOR_PWM_STATUS_HW_ERROR;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Hal::MotorPwmStatus ClearAllTrips(
    const Bsp_MotorPwmHwType& hwConfig)
{
    Hal::MotorPwmStatus status;
    Mcal_EpwmStatusType mcalStatus;

    status = Hal::MOTOR_PWM_STATUS_OK;

    mcalStatus =
        Mcal_Epwm_ClearTrip(
            hwConfig.phaseU.module);

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        mcalStatus =
            Mcal_Epwm_ClearTrip(
                hwConfig.phaseV.module);
    }
    else
    {
        /* Do nothing. */
    }

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        mcalStatus =
            Mcal_Epwm_ClearTrip(
                hwConfig.phaseW.module);
    }
    else
    {
        /* Do nothing. */
    }

    if(mcalStatus != MCAL_EPWM_STATUS_OK)
    {
        status =
            Hal::MOTOR_PWM_STATUS_HW_ERROR;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Hal::MotorPwmStatus AreAllTripsInactive(
    const Bsp_MotorPwmHwType& hwConfig,
    bool& allInactive)
{
    Hal::MotorPwmStatus status;
    Mcal_EpwmStatusType mcalStatus;
    uint16_t phaseUTrip;
    uint16_t phaseVTrip;
    uint16_t phaseWTrip;

    status = Hal::MOTOR_PWM_STATUS_OK;
    allInactive = false;

    mcalStatus =
        Mcal_Epwm_IsTripActive(
            hwConfig.phaseU.module,
            &phaseUTrip);

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        mcalStatus =
            Mcal_Epwm_IsTripActive(
                hwConfig.phaseV.module,
                &phaseVTrip);
    }
    else
    {
        /* Do nothing. */
    }

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        mcalStatus =
            Mcal_Epwm_IsTripActive(
                hwConfig.phaseW.module,
                &phaseWTrip);
    }
    else
    {
        /* Do nothing. */
    }

    if(mcalStatus == MCAL_EPWM_STATUS_OK)
    {
        if((phaseUTrip == 0U) &&
           (phaseVTrip == 0U) &&
           (phaseWTrip == 0U))
        {
            allInactive = true;
        }
        else
        {
            /* At least one output remains tripped. */
        }
    }
    else
    {
        status =
            Hal::MOTOR_PWM_STATUS_HW_ERROR;
    }

    return status;
}

Hal::MotorPwmStatus ForceAllTrips(
    const Bsp_MotorPwmHwType& hwConfig)
{
    Hal::MotorPwmStatus status;
    Mcal_EpwmStatusType phaseUStatus;
    Mcal_EpwmStatusType phaseVStatus;
    Mcal_EpwmStatusType phaseWStatus;

    /*
     * Attempt every safety action even if one of them fails.
     */
    phaseUStatus =
        Mcal_Epwm_ForceTrip(
            hwConfig.phaseU.module);

    phaseVStatus =
        Mcal_Epwm_ForceTrip(
            hwConfig.phaseV.module);

    phaseWStatus =
        Mcal_Epwm_ForceTrip(
            hwConfig.phaseW.module);

    if((phaseUStatus == MCAL_EPWM_STATUS_OK) &&
       (phaseVStatus == MCAL_EPWM_STATUS_OK) &&
       (phaseWStatus == MCAL_EPWM_STATUS_OK))
    {
        status = Hal::MOTOR_PWM_STATUS_OK;
    }
    else
    {
        status =
            Hal::MOTOR_PWM_STATUS_HW_ERROR;
    }

    return status;
}

}

