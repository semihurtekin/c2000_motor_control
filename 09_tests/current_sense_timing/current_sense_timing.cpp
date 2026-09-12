/**
 * @file    current_sense_timing_test.cpp
 * @brief   Scope validation of the CurrentSense ADC sampling instant.
 *
 * WARNING:
 *   This lab enables the motor PWM outputs. Run it with NO inverter or
 *   gate-driver power stage connected.
 *
 * Scope:
 *   CH1 -> J4-1 / GPIO0 / EPWM1A
 *   CH2 -> J8-1 / GPIO6 / EPWM4A timing marker
 *   GND -> LaunchPad GND
 *
 * Marker:
 *   EPWM4 runs synchronized with EPWM1 at 20 kHz center-aligned.
 *   CMPA = TBPRD - 50 counts.
 *   EPWMCLK = 100 MHz -> 10 ns/count.
 *   Therefore EPWM4A is LOW for 1 us centered on TBCTR = TBPRD.
 *
 *   The midpoint of the marker LOW pulse is the EPWM1 PERIOD event, which is
 *   also the CurrentSense SOCA trigger point.
 *
 *   With the temporary bench setting of 100 SYSCLK acquisition cycles:
 *   SYSCLK = 200 MHz -> 5 ns/cycle -> acquisition window = 500 ns.
 */

#include <stdint.h>

#include "platform_clock.h"
#include "bsp_motor_hw.h"
#include "mcal_epwm.h"
#include "mcal_gpio.h"
#include "hal_motor_pwm.hpp"
#include "hal_current_sense.hpp"

#define TEST_PWM_FREQUENCY_HZ         (20000UL)
#define TEST_PWM_DEAD_TIME_NS         (500UL)
#define TEST_EPWM_CLK_HZ              (100000000UL)

#define TEST_MARKER_GPIO              (6U)
#define TEST_MARKER_MUX_EPWM4A        (1U)
#define TEST_MARKER_COUNTS_FROM_PRD   (50U)

#define TEST_ADC_REFERENCE_V          (3.0F)
#define TEST_ADC_COUNTS               (4096U)
#define TEST_CALIBRATION_SAMPLES      (128U)
#define TEST_SHUNT_RESISTANCE_OHM     (1.0F)
#define TEST_AMPLIFIER_GAIN           (1.0F)

volatile Platform_ClockStatusType G_ClockStatus =
    PLATFORM_CLOCK_STATUS_INVALID_PARAM;

volatile Bsp_MotorHwStatusType G_BspStatus =
    BSP_MOTOR_HW_STATUS_INIT_FAILED;

volatile Mcal_EpwmStatusType G_MarkerEpwmStatus =
    MCAL_EPWM_STATUS_INV_ARG;

volatile Mcal_GpioStatusType G_MarkerGpioStatus =
    MCAL_GPIO_STATUS_INV_ARG;

volatile Hal::MotorPwmStatus G_PwmStatus =
    Hal::MOTOR_PWM_STATUS_NOT_INITIALIZED;

volatile Hal::CurrentSenseStatus G_CurrentSenseStatus =
    Hal::CURRENT_SENSE_STATUS_NOT_INITIALIZED;

volatile uint16_t G_TestPassed = 0U;

static Mcal_EpwmStatusType InitSamplingMarker(void);
static void StayHere(void);

int main(void)
{
    Hal::MotorPwm motorPwm;
    Hal::CurrentSense currentSense;

    Hal::MotorPwmConfig pwmConfig;
    Hal::MotorPwmDuty pwmDuty;
    Hal::CurrentSenseConfig currentSenseConfig;

    uint16_t testOk;

    testOk = 1U;

    G_ClockStatus = Platform_ClockInit();

    if(G_ClockStatus != PLATFORM_CLOCK_STATUS_OK)
    {
        testOk = 0U;
    }
    else
    {
        /* Do nothing. */
    }

    if(testOk != 0U)
    {
        G_BspStatus = Bsp_MotorHw_Init();

        if(G_BspStatus != BSP_MOTOR_HW_STATUS_OK)
        {
            testOk = 0U;
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
     * Configure the synchronized EPWM4A timing marker before MotorPwm::Init()
     * releases TBCLKSYNC for EPWM1-3.
     */
    if(testOk != 0U)
    {
        G_MarkerEpwmStatus =
            Mcal_Epwm_SetTbClkSync(
                MCAL_EPWM_TBCLK_SYNC_DISABLE);

        if(G_MarkerEpwmStatus == MCAL_EPWM_STATUS_OK)
        {
            G_MarkerEpwmStatus = InitSamplingMarker();
        }
        else
        {
            /* Do nothing. */
        }

        if(G_MarkerEpwmStatus != MCAL_EPWM_STATUS_OK)
        {
            testOk = 0U;
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

    if(testOk != 0U)
    {
        pwmConfig.frequencyHz = TEST_PWM_FREQUENCY_HZ;
        pwmConfig.deadTimeNs = TEST_PWM_DEAD_TIME_NS;

        G_PwmStatus = motorPwm.Init(pwmConfig);

        if(G_PwmStatus == Hal::MOTOR_PWM_STATUS_OK)
        {
            pwmDuty.phaseU = 0.50F;
            pwmDuty.phaseV = 0.50F;
            pwmDuty.phaseW = 0.50F;

            G_PwmStatus = motorPwm.SetDuty(pwmDuty);
        }
        else
        {
            /* Do nothing. */
        }

        if(G_PwmStatus != Hal::MOTOR_PWM_STATUS_OK)
        {
            testOk = 0U;
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
     * Keep the analog input at approximately 1.50 V for calibration.
     */
    if(testOk != 0U)
    {
        currentSenseConfig.shuntResistanceOhm =
            TEST_SHUNT_RESISTANCE_OHM;
        currentSenseConfig.amplifierGain =
            TEST_AMPLIFIER_GAIN;
        currentSenseConfig.adcReferenceVoltage =
            TEST_ADC_REFERENCE_V;
        currentSenseConfig.adcResolutionCounts =
            TEST_ADC_COUNTS;
        currentSenseConfig.calibrationSampleCount =
            TEST_CALIBRATION_SAMPLES;

        G_CurrentSenseStatus =
            currentSense.Init(currentSenseConfig);

        if(G_CurrentSenseStatus == Hal::CURRENT_SENSE_STATUS_OK)
        {
            G_CurrentSenseStatus =
                currentSense.Calibrate();
        }
        else
        {
            /* Do nothing. */
        }

        if(G_CurrentSenseStatus == Hal::CURRENT_SENSE_STATUS_OK)
        {
            G_CurrentSenseStatus =
                currentSense.StartSampling();
        }
        else
        {
            /* Do nothing. */
        }

        if(G_CurrentSenseStatus != Hal::CURRENT_SENSE_STATUS_OK)
        {
            testOk = 0U;
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
     * Enable EPWM1A only for scope observation.
     * NO inverter / gate-driver power stage shall be connected.
     */
    if(testOk != 0U)
    {
        G_PwmStatus = motorPwm.Enable();

        if(G_PwmStatus != Hal::MOTOR_PWM_STATUS_OK)
        {
            testOk = 0U;
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

    if(testOk != 0U)
    {
        G_TestPassed = 1U;
    }
    else
    {
        G_TestPassed = 0U;
    }

    StayHere();

    return 0;
}

static Mcal_EpwmStatusType InitSamplingMarker(void)
{
    Mcal_EpwmStatusType status;
    Platform_ClockStatusType clockStatus;
    Mcal_EpwmTbConfigType tbConfig;
    Mcal_EpwmCompareConfigType compareConfig;
    uint32_t period;
    uint16_t markerCompare;

    status = MCAL_EPWM_STATUS_INV_ARG;

    clockStatus =
        Platform_ClockEnableEpwm(
            PLATFORM_EPWM_MODULE_4);

    if(clockStatus == PLATFORM_CLOCK_STATUS_OK)
    {
        G_MarkerGpioStatus =
            Mcal_Gpio_SetMux(
                TEST_MARKER_GPIO,
                TEST_MARKER_MUX_EPWM4A);

        if(G_MarkerGpioStatus == MCAL_GPIO_STATUS_OK)
        {
            period =
                (TEST_EPWM_CLK_HZ / 2UL) /
                TEST_PWM_FREQUENCY_HZ;

            markerCompare =
                (uint16_t)(
                    period -
                    TEST_MARKER_COUNTS_FROM_PRD);

            tbConfig.module = MCAL_EPWM_4;
            tbConfig.period = (uint16_t)period;
            tbConfig.mode = MCAL_EPWM_COUNT_UP_DOWN;
            tbConfig.clkDiv = MCAL_EPWM_CLKDIV_1;
            tbConfig.hsClkDiv = MCAL_EPWM_HSCLKDIV_1;

            status = Mcal_Epwm_InitTimeBase(&tbConfig);

            if(status == MCAL_EPWM_STATUS_OK)
            {
                compareConfig.module = MCAL_EPWM_4;
                compareConfig.compareA = markerCompare;

                status =
                    Mcal_Epwm_InitCompareA(
                        &compareConfig);
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

static void StayHere(void)
{
    for(;;)
    {
        /* Keep hardware running for oscilloscope observation. */
    }
}
