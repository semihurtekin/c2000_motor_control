/**
 * @file    mcal_adc.h
 * @brief   F28379D ADC MCAL driver interface.
 */

#ifndef MCAL_ADC_H
#define MCAL_ADC_H

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * Public Macros
 *============================================================================*/

/*==============================================================================
 * Public Types
 *============================================================================*/

typedef enum
{
    MCAL_ADC_A = 0U,
    MCAL_ADC_B = 1U,
    MCAL_ADC_C = 2U,
    MCAL_ADC_D = 3U
} Mcal_AdcIdType;

typedef enum
{
    MCAL_ADC_SOC_0 = 0U,
    MCAL_ADC_SOC_1 = 1U,
    MCAL_ADC_SOC_2 = 2U,
    MCAL_ADC_SOC_3 = 3U,
    MCAL_ADC_SOC_4 = 4U,
    MCAL_ADC_SOC_5 = 5U,
    MCAL_ADC_SOC_6 = 6U,
    MCAL_ADC_SOC_7 = 7U,
    MCAL_ADC_SOC_8 = 8U,
    MCAL_ADC_SOC_9 = 9U,
    MCAL_ADC_SOC_10 = 10U,
    MCAL_ADC_SOC_11 = 11U,
    MCAL_ADC_SOC_12 = 12U,
    MCAL_ADC_SOC_13 = 13U,
    MCAL_ADC_SOC_14 = 14U,
    MCAL_ADC_SOC_15 = 15U
} Mcal_AdcSocType;

typedef enum
{
    MCAL_ADC_CHANNEL_0 = 0U,
    MCAL_ADC_CHANNEL_1 = 1U,
    MCAL_ADC_CHANNEL_2 = 2U,
    MCAL_ADC_CHANNEL_3 = 3U,
    MCAL_ADC_CHANNEL_4 = 4U,
    MCAL_ADC_CHANNEL_5 = 5U,
    MCAL_ADC_CHANNEL_6 = 6U,
    MCAL_ADC_CHANNEL_7 = 7U,
    MCAL_ADC_CHANNEL_8 = 8U,
    MCAL_ADC_CHANNEL_9 = 9U,
    MCAL_ADC_CHANNEL_10 = 10U,
    MCAL_ADC_CHANNEL_11 = 11U,
    MCAL_ADC_CHANNEL_12 = 12U,
    MCAL_ADC_CHANNEL_13 = 13U,
    MCAL_ADC_CHANNEL_14 = 14U,
    MCAL_ADC_CHANNEL_15 = 15U
} Mcal_AdcChannelType;

typedef enum
{
    MCAL_ADC_TRIG_SW_ONLY = 0U,
    MCAL_ADC_TRIG_EPWM1_SOCA = 5U,
    MCAL_ADC_TRIG_EPWM1_SOCB = 6U
} Mcal_AdcTriggerType;

typedef enum
{
    MCAL_ADC_STATUS_OK = 0U,
    MCAL_ADC_STATUS_INV_ID = 1U,
    MCAL_ADC_STATUS_INV_ARG = 2U
} Mcal_AdcStatusType;


typedef enum
{
    MCAL_ADC_INT_1 = 1U,
    MCAL_ADC_INT_2 = 2U,
    MCAL_ADC_INT_3 = 3U,
    MCAL_ADC_INT_4 = 4U
} Mcal_AdcIntType;

typedef struct
{
    Mcal_AdcIdType adc;
    Mcal_AdcIntType adcInt;
    Mcal_AdcSocType sourceEoc;
} Mcal_AdcIntConfigType;

typedef struct
{
    Mcal_AdcIdType adc;
    Mcal_AdcSocType soc;
    Mcal_AdcChannelType channel;
    Mcal_AdcTriggerType trigger;
    uint16_t acquisitionCycles;
} Mcal_AdcSocConfigType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes and powers up the selected ADC module.
 *
 * ADC v0.1 uses 12-bit single-ended operation and ADCCLK = SYSCLK / 4.
 * TI AdcSetMode() is used as the vendor boundary for device factory trim.
 *
 * The caller shall wait at least 500 us after this function before allowing
 * the first ADC conversion trigger.
 *
 * @param adc Selected ADC module.
 *
 * @return Driver status.
 */
Mcal_AdcStatusType Mcal_Adc_Init(
    Mcal_AdcIdType adc);

/**
 * @brief Configures one ADC SOC conversion slot.
 *
 * acquisitionCycles is expressed in SYSCLK cycles. The MCAL converts the
 * physical cycle count to the hardware ACQPS encoding internally.
 *
 * @param config SOC configuration.
 *
 * @return Driver status.
 */
Mcal_AdcStatusType Mcal_Adc_InitSoc(
    const Mcal_AdcSocConfigType * config);

/**
 * @brief Reads the result register associated with one SOC.
 *
 * This function does not wait for conversion completion. The caller shall
 * ensure that the corresponding conversion has completed when a coherent
 * sample is required.
 *
 * @param adc Selected ADC module.
 * @param soc SOC/result index.
 * @param result Receives the ADC conversion result.
 *
 * @return Driver status.
 */
Mcal_AdcStatusType Mcal_Adc_GetResult(
    Mcal_AdcIdType adc,
    Mcal_AdcSocType soc,
    uint16_t * result);


/**
 * @brief Configures and enables one ADC interrupt generator.
 *
 * The selected ADCINTx is sourced from the selected EOC event.
 * Continuous interrupt mode is enabled as required by the F2837xD ADC
 * interrupt silicon erratum workaround. Interrupt overflow shall still be
 * monitored by software.
 *
 * @param config ADC interrupt configuration.
 *
 * @return Driver status.
 */
Mcal_AdcStatusType Mcal_Adc_EnableInterrupt(
    const Mcal_AdcIntConfigType * config);

/**
 * @brief Clears one ADC interrupt flag.
 *
 * @param adc Selected ADC module.
 * @param adcInt ADC interrupt generator.
 *
 * @return Driver status.
 */
Mcal_AdcStatusType Mcal_Adc_ClearIntFlag(
    Mcal_AdcIdType adc,
    Mcal_AdcIntType adcInt);

/**
 * @brief Reads one ADC interrupt overflow flag.
 *
 * @param adc Selected ADC module.
 * @param adcInt ADC interrupt generator.
 * @param overflow Receives 1U when overflow is set, otherwise 0U.
 *
 * @return Driver status.
 */
Mcal_AdcStatusType Mcal_Adc_IsIntOverflow(
    Mcal_AdcIdType adc,
    Mcal_AdcIntType adcInt,
    uint16_t * overflow);

/**
 * @brief Clears one ADC interrupt overflow flag.
 *
 * @param adc Selected ADC module.
 * @param adcInt ADC interrupt generator.
 *
 * @return Driver status.
 */
Mcal_AdcStatusType Mcal_Adc_ClearIntOverflow(
    Mcal_AdcIdType adc,
    Mcal_AdcIntType adcInt);

Mcal_AdcStatusType Mcal_Adc_SetSocPriority(
    Mcal_AdcIdType adc,
    uint16_t highPrioritySocCount);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_ADC_H */
