/**
 * @file    current_sense_test.cpp
 * @brief   Bench validation for the CurrentSense HAL on LAUNCHXL-F28379D.
 *
 * Hardware:
 *   J3-29 = ADCINA2 = phase U input
 *   J3-28 = ADCINB2 = phase V input
 *
 * Test setup:
 *   Apply the same adjustable 0 V to 3.0 V signal to both ADC inputs.
 *   Set the input to approximately 1.50 V before starting the program so that
 *   zero-current calibration occurs near mid-scale.
 *
 * Notes:
 *   - Motor PWM outputs are kept disabled/tripped for the whole test.
 *   - This is bench/lab code. It intentionally reads MCAL/BSP data directly
 *     after HAL initialization to observe the measurement chain.
 */

#include <stdint.h>

#include "platform_clock.h"

#include "bsp_motor_hw.h"
#include "bsp_current_sense.h"

#include "mcal_adc.h"

#include "hal_motor_pwm.hpp"
#include "hal_current_sense.hpp"
#include "hal_current_sense_fast.h"

#define TEST_PWM_FREQUENCY_HZ          (20000UL)
#define TEST_PWM_DEAD_TIME_NS          (500UL)

#define TEST_ADC_REFERENCE_V           (3.0F)
#define TEST_ADC_COUNTS                (4096U)
#define TEST_CALIBRATION_SAMPLES       (128U)

#define TEST_SHUNT_RESISTANCE_OHM      (1.0F)
#define TEST_AMPLIFIER_GAIN            (1.0F)

#define TEST_SAMPLE_POLL_LIMIT         (100000UL)

typedef enum
{
    TEST_STAGE_RESET = 0U,
    TEST_STAGE_CLOCK_READY,
    TEST_STAGE_PWM_READY,
    TEST_STAGE_CURRENT_SENSE_READY,
    TEST_STAGE_CALIBRATED,
    TEST_STAGE_SAMPLING,
    TEST_STAGE_RUNNING,
    TEST_STAGE_ERROR
} CurrentSenseTestStageType;

volatile CurrentSenseTestStageType G_TestStage = TEST_STAGE_RESET;

volatile Platform_ClockStatusType G_ClockStatus =
    PLATFORM_CLOCK_STATUS_INVALID_PARAM;

volatile Bsp_MotorHwStatusType G_BspPwmStatus =
    BSP_MOTOR_HW_STATUS_INIT_FAILED;

volatile Hal::MotorPwmStatus G_PwmStatus =
    Hal::MOTOR_PWM_STATUS_NOT_INITIALIZED;

volatile Hal::CurrentSenseStatus G_CurrentSenseStatus =
    Hal::CURRENT_SENSE_STATUS_NOT_INITIALIZED;

volatile Mcal_AdcStatusType G_AdcStatusU =
    MCAL_ADC_STATUS_INV_ARG;

volatile Mcal_AdcStatusType G_AdcStatusV =
    MCAL_ADC_STATUS_INV_ARG;

volatile uint16_t G_RawU = 0U;
volatile uint16_t G_RawV = 0U;

volatile float G_InputVoltageU = 0.0F;
volatile float G_InputVoltageV = 0.0F;

CurrentSenseFastConfigType G_FastConfig =
{
    0.0F,
    0.0F,
    0.0F,
    0.0F
};

PhaseCurrentType G_PhaseCurrent =
{
    0.0F,
    0.0F,
    0.0F
};

static Mcal_AdcStatusType ReadFreshSamplePair(
    const Bsp_CurrentSenseHwType * hwConfig,
    uint16_t * rawU,
    uint16_t * rawV);

static Mcal_AdcStatusType WaitForAdcFlag(
    Mcal_AdcIdType adc);

static void EnterErrorState(void);

int main(void)
{
    Hal::MotorPwm motorPwm;
    Hal::CurrentSense currentSense;

    Hal::MotorPwmConfig pwmConfig;
    Hal::CurrentSenseConfig currentSenseConfig;

    const Bsp_CurrentSenseHwType * currentSenseHw;

    uint16_t rawU;
    uint16_t rawV;

    rawU = 0U;
    rawV = 0U;
    currentSenseHw = 0;

    G_ClockStatus = Platform_ClockInit();

    if(G_ClockStatus == PLATFORM_CLOCK_STATUS_OK)
    {
        G_TestStage = TEST_STAGE_CLOCK_READY;
    }
    else
    {
        EnterErrorState();
    }

    if(G_TestStage != TEST_STAGE_ERROR)
    {
        G_BspPwmStatus = Bsp_MotorHw_Init();

        if(G_BspPwmStatus == BSP_MOTOR_HW_STATUS_OK)
        {
            pwmConfig.frequencyHz = TEST_PWM_FREQUENCY_HZ;
            pwmConfig.deadTimeNs = TEST_PWM_DEAD_TIME_NS;

            G_PwmStatus = motorPwm.Init(pwmConfig);

            if(G_PwmStatus == Hal::MOTOR_PWM_STATUS_OK)
            {
                G_PwmStatus = motorPwm.Disable();

                if(G_PwmStatus == Hal::MOTOR_PWM_STATUS_OK)
                {
                    G_TestStage = TEST_STAGE_PWM_READY;
                }
                else
                {
                    EnterErrorState();
                }
            }
            else
            {
                EnterErrorState();
            }
        }
        else
        {
            EnterErrorState();
        }
    }
    else
    {
        /* Do nothing. */
    }

    if(G_TestStage != TEST_STAGE_ERROR)
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
            currentSense.Init(
                currentSenseConfig);

        if(G_CurrentSenseStatus == Hal::CURRENT_SENSE_STATUS_OK)
        {
            G_TestStage =
                TEST_STAGE_CURRENT_SENSE_READY;
        }
        else
        {
            EnterErrorState();
        }
    }
    else
    {
        /* Do nothing. */
    }

    /*
     * Set the external analog input to approximately 1.50 V before running.
     */
    if(G_TestStage != TEST_STAGE_ERROR)
    {
        G_CurrentSenseStatus =
            currentSense.Calibrate();

        if(G_CurrentSenseStatus == Hal::CURRENT_SENSE_STATUS_OK)
        {
            G_CurrentSenseStatus =
                currentSense.GetFastConfig(
                    G_FastConfig);
        }
        else
        {
            /* Do nothing. */
        }

        if(G_CurrentSenseStatus == Hal::CURRENT_SENSE_STATUS_OK)
        {
            G_TestStage =
                TEST_STAGE_CALIBRATED;
        }
        else
        {
            EnterErrorState();
        }
    }
    else
    {
        /* Do nothing. */
    }

    if(G_TestStage != TEST_STAGE_ERROR)
    {
        currentSenseHw =
            Bsp_CurrentSense_GetHwConfig();

        if(currentSenseHw != 0)
        {
            G_CurrentSenseStatus =
                currentSense.StartSampling();

            if(G_CurrentSenseStatus == Hal::CURRENT_SENSE_STATUS_OK)
            {
                G_TestStage =
                    TEST_STAGE_SAMPLING;
            }
            else
            {
                EnterErrorState();
            }
        }
        else
        {
            EnterErrorState();
        }
    }
    else
    {
        /* Do nothing. */
    }

    while(G_TestStage != TEST_STAGE_ERROR)
    {
        G_AdcStatusU =
            ReadFreshSamplePair(
                currentSenseHw,
                &rawU,
                &rawV);

        G_AdcStatusV = G_AdcStatusU;

        if(G_AdcStatusU == MCAL_ADC_STATUS_OK)
        {
            G_RawU = rawU;
            G_RawV = rawV;

            G_InputVoltageU =
                ((float)rawU *
                 TEST_ADC_REFERENCE_V) /
                (float)TEST_ADC_COUNTS;

            G_InputVoltageV =
                ((float)rawV *
                 TEST_ADC_REFERENCE_V) /
                (float)TEST_ADC_COUNTS;

            CurrentSenseFast_Convert(
                rawU,
                rawV,
                &G_FastConfig,
                &G_PhaseCurrent);

            G_TestStage =
                TEST_STAGE_RUNNING;
        }
        else
        {
            EnterErrorState();
        }
    }

    for(;;)
    {
        /* Keep debugger state available after an error. */
    }
}

static Mcal_AdcStatusType ReadFreshSamplePair(
    const Bsp_CurrentSenseHwType * hwConfig,
    uint16_t * rawU,
    uint16_t * rawV)
{
    Mcal_AdcStatusType status;

    status = MCAL_ADC_STATUS_INV_ARG;

    if((hwConfig != 0) &&
       (rawU != 0) &&
       (rawV != 0))
    {
        status =
            Mcal_Adc_ClearIntOverflow(
                hwConfig->phaseU.adc,
                MCAL_ADC_INT_1);

        if(status == MCAL_ADC_STATUS_OK)
        {
            status =
                Mcal_Adc_ClearIntOverflow(
                    hwConfig->phaseV.adc,
                    MCAL_ADC_INT_1);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_ADC_STATUS_OK)
        {
            status =
                Mcal_Adc_ClearIntFlag(
                    hwConfig->phaseU.adc,
                    MCAL_ADC_INT_1);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_ADC_STATUS_OK)
        {
            status =
                Mcal_Adc_ClearIntFlag(
                    hwConfig->phaseV.adc,
                    MCAL_ADC_INT_1);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_ADC_STATUS_OK)
        {
            status =
                WaitForAdcFlag(
                    hwConfig->phaseU.adc);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_ADC_STATUS_OK)
        {
            status =
                WaitForAdcFlag(
                    hwConfig->phaseV.adc);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_ADC_STATUS_OK)
        {
            status =
                Mcal_Adc_GetResult(
                    hwConfig->phaseU.adc,
                    hwConfig->phaseU.soc,
                    rawU);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_ADC_STATUS_OK)
        {
            status =
                Mcal_Adc_GetResult(
                    hwConfig->phaseV.adc,
                    hwConfig->phaseV.soc,
                    rawV);
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        /* Invalid test argument. */
    }

    return status;
}

static Mcal_AdcStatusType WaitForAdcFlag(
    Mcal_AdcIdType adc)
{
    Mcal_AdcStatusType status;
    uint16_t flagSet;
    uint32_t pollCount;

    status = MCAL_ADC_STATUS_OK;
    flagSet = 0U;
    pollCount = 0UL;

    while((status == MCAL_ADC_STATUS_OK) &&
          (flagSet == 0U) &&
          (pollCount < TEST_SAMPLE_POLL_LIMIT))
    {
        status =
            Mcal_Adc_IsIntFlagSet(
                adc,
                MCAL_ADC_INT_1,
                &flagSet);

        pollCount++;
    }

    if((status == MCAL_ADC_STATUS_OK) &&
       (flagSet == 0U))
    {
        /*
         * Temporary bench-test timeout indication.
         */
        status = MCAL_ADC_STATUS_INV_ARG;
    }
    else
    {
        /* Preserve the current status. */
    }

    return status;
}

static void EnterErrorState(void)
{
    G_TestStage = TEST_STAGE_ERROR;
}
