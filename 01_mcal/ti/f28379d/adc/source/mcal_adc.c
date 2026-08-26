/**
 * @file    mcal_adc.c
 * @brief   F28379D ADC MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stddef.h>

#include "mcal_adc.h"
#include "F2837xD_device.h"
#include "F2837xD_Examples.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_ADC_PRESCALE_DIV_4         (6U)
#define MCAL_ADC_INT_PULSE_EOC          (1U)
#define MCAL_ADC_POWER_UP               (1U)

/*
 * ADC v0.1 uses the current 200 MHz SYSCLK platform configuration.
 * 12-bit mode requires at least 75 ns acquisition time:
 * 15 SYSCLK cycles x 5 ns = 75 ns.
 */
#define MCAL_ADC_ACQ_CYCLES_MIN         (15U)
#define MCAL_ADC_ACQ_CYCLES_MAX         (512U)


#define MCAL_ADC_INT_DISABLE            (0U)
#define MCAL_ADC_INT_ENABLE             (1U)
#define MCAL_ADC_INT_CONT_ENABLE        (1U)
#define MCAL_ADC_FLAG_CLEAR             (1U)
#define MCAL_ADC_FLAG_RESET             (0U)
#define MCAL_ADC_FLAG_SET               (1U)

/*==============================================================================
 * Private Types
 *============================================================================*/

/*==============================================================================
 * Private Variables
 *============================================================================*/

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static volatile struct ADC_REGS * GetAdcRegs(
    Mcal_AdcIdType adc);

static volatile struct ADC_RESULT_REGS * GetResultRegs(
    Mcal_AdcIdType adc);

static Mcal_AdcStatusType IsAdcValid(
    Mcal_AdcIdType adc);

static Mcal_AdcStatusType IsSocValid(
    Mcal_AdcSocType soc);

static Mcal_AdcStatusType IsChannelValid(
    Mcal_AdcChannelType channel);

static Mcal_AdcStatusType IsTriggerValid(
    Mcal_AdcTriggerType trigger);

static Mcal_AdcStatusType IsSocConfigValid(
    const Mcal_AdcSocConfigType * config);


static Mcal_AdcStatusType IsIntValid(
    Mcal_AdcIntType adcInt);

static Mcal_AdcStatusType IsIntConfigValid(
    const Mcal_AdcIntConfigType * config);

static void ConfigureInterrupt(
    volatile struct ADC_REGS * adcRegs,
    Mcal_AdcIntType adcInt,
    Mcal_AdcSocType sourceEoc);

static void ClearIntFlag(
    volatile struct ADC_REGS * adcRegs,
    Mcal_AdcIntType adcInt);

static uint16_t ReadIntOverflow(
    volatile struct ADC_REGS * adcRegs,
    Mcal_AdcIntType adcInt);

static void ClearIntOverflow(
    volatile struct ADC_REGS * adcRegs,
    Mcal_AdcIntType adcInt);

static uint16_t GetTiAdcId(
    Mcal_AdcIdType adc);

static void ConfigureSoc(
    volatile struct ADC_REGS * adcRegs,
    Mcal_AdcSocType soc,
    uint16_t channel,
    uint16_t trigger,
    uint16_t acqps);

static uint16_t ReadResult(
    volatile struct ADC_RESULT_REGS * resultRegs,
    Mcal_AdcSocType soc);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_AdcStatusType Mcal_Adc_Init(
    Mcal_AdcIdType adc)
{
    Mcal_AdcStatusType status;
    volatile struct ADC_REGS * adcRegs;
    uint16_t tiAdcId;

    status = IsAdcValid(adc);

    if(status == MCAL_ADC_STATUS_OK)
    {
        adcRegs = GetAdcRegs(adc);
        tiAdcId = GetTiAdcId(adc);

        EALLOW;

        /*
         * Current platform:
         * SYSCLK = 200 MHz
         * ADCCLK = 200 MHz / 4 = 50 MHz
         */
        adcRegs->ADCCTL2.bit.PRESCALE =
            MCAL_ADC_PRESCALE_DIV_4;

        /*
         * Vendor boundary. AdcSetMode() selects the conversion mode and
         * applies the device-specific ADC trim values stored in OTP.
         */
        AdcSetMode(
            tiAdcId,
            ADC_RESOLUTION_12BIT,
            ADC_SIGNALMODE_SINGLE);

        /*
         * Generate the ADC interrupt pulse at end-of-conversion timing.
         * The interrupt itself is not enabled by ADC v0.1 yet.
         */
        adcRegs->ADCCTL1.bit.INTPULSEPOS =
            MCAL_ADC_INT_PULSE_EOC;

        adcRegs->ADCCTL1.bit.ADCPWDNZ =
            MCAL_ADC_POWER_UP;

        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_AdcStatusType Mcal_Adc_InitSoc(
    const Mcal_AdcSocConfigType * config)
{
    Mcal_AdcStatusType status;
    volatile struct ADC_REGS * adcRegs;
    uint16_t acqps;

    status = IsSocConfigValid(config);

    if(status == MCAL_ADC_STATUS_OK)
    {
        adcRegs = GetAdcRegs(config->adc);

        /*
         * Hardware encoding:
         * acquisitionCycles = ACQPS + 1.
         */
        acqps = config->acquisitionCycles - 1U;

        EALLOW;

        ConfigureSoc(
            adcRegs,
            config->soc,
            (uint16_t)config->channel,
            (uint16_t)config->trigger,
            acqps);

        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_AdcStatusType Mcal_Adc_GetResult(
    Mcal_AdcIdType adc,
    Mcal_AdcSocType soc,
    uint16_t * result)
{
    Mcal_AdcStatusType status;
    volatile struct ADC_RESULT_REGS * resultRegs;

    status = IsAdcValid(adc);

    if(status == MCAL_ADC_STATUS_OK)
    {
        status = IsSocValid(soc);

        if(status == MCAL_ADC_STATUS_OK)
        {
            if(result != NULL)
            {
                resultRegs = GetResultRegs(adc);
                *result = ReadResult(resultRegs, soc);
            }
            else
            {
                status = MCAL_ADC_STATUS_INV_ARG;
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


Mcal_AdcStatusType Mcal_Adc_EnableInterrupt(
    const Mcal_AdcIntConfigType * config)
{
    Mcal_AdcStatusType status;
    volatile struct ADC_REGS * adcRegs;

    status = IsIntConfigValid(config);

    if(status == MCAL_ADC_STATUS_OK)
    {
        adcRegs = GetAdcRegs(config->adc);

        EALLOW;

        ConfigureInterrupt(
            adcRegs,
            config->adcInt,
            config->sourceEoc);

        EDIS;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

Mcal_AdcStatusType Mcal_Adc_ClearIntFlag(
    Mcal_AdcIdType adc,
    Mcal_AdcIntType adcInt)
{
    Mcal_AdcStatusType status;
    volatile struct ADC_REGS * adcRegs;

    status = IsAdcValid(adc);

    if(status == MCAL_ADC_STATUS_OK)
    {
        status = IsIntValid(adcInt);

        if(status == MCAL_ADC_STATUS_OK)
        {
            adcRegs = GetAdcRegs(adc);
            ClearIntFlag(adcRegs, adcInt);
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

Mcal_AdcStatusType Mcal_Adc_IsIntOverflow(
    Mcal_AdcIdType adc,
    Mcal_AdcIntType adcInt,
    uint16_t * overflow)
{
    Mcal_AdcStatusType status;
    volatile struct ADC_REGS * adcRegs;

    status = IsAdcValid(adc);

    if(status == MCAL_ADC_STATUS_OK)
    {
        status = IsIntValid(adcInt);

        if(status == MCAL_ADC_STATUS_OK)
        {
            if(overflow != NULL)
            {
                adcRegs = GetAdcRegs(adc);

                *overflow = ReadIntOverflow(
                    adcRegs,
                    adcInt);
            }
            else
            {
                status = MCAL_ADC_STATUS_INV_ARG;
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

Mcal_AdcStatusType Mcal_Adc_ClearIntOverflow(
    Mcal_AdcIdType adc,
    Mcal_AdcIntType adcInt)
{
    Mcal_AdcStatusType status;
    volatile struct ADC_REGS * adcRegs;

    status = IsAdcValid(adc);

    if(status == MCAL_ADC_STATUS_OK)
    {
        status = IsIntValid(adcInt);

        if(status == MCAL_ADC_STATUS_OK)
        {
            adcRegs = GetAdcRegs(adc);
            ClearIntOverflow(adcRegs, adcInt);
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

Mcal_AdcStatusType Mcal_Adc_SetSocPriority(
    Mcal_AdcIdType adc,
    uint16_t highPrioritySocCount)
{
    volatile struct ADC_REGS * adcRegs;
    Mcal_AdcStatusType status;
    status = IsAdcValid(adc);

    if(status == MCAL_ADC_STATUS_OK)
    {
        if(highPrioritySocCount <= 16)
        {
            adcRegs = GetAdcRegs(adc);
            adcRegs->ADCSOCPRICTL.bit.SOCPRIORITY = highPrioritySocCount;
        }
        else 
        {
            status = MCAL_ADC_STATUS_INV_ARG;        
        }
    }

    return status;
}

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

static volatile struct ADC_REGS * GetAdcRegs(
    Mcal_AdcIdType adc)
{
    volatile struct ADC_REGS * adcRegs;

    adcRegs = NULL;

    switch(adc)
    {
        case MCAL_ADC_A:
            adcRegs = &AdcaRegs;
            break;

        case MCAL_ADC_B:
            adcRegs = &AdcbRegs;
            break;

        case MCAL_ADC_C:
            adcRegs = &AdccRegs;
            break;

        case MCAL_ADC_D:
            adcRegs = &AdcdRegs;
            break;

        default:
            /* Do nothing. */
            break;
    }

    return adcRegs;
}

static volatile struct ADC_RESULT_REGS * GetResultRegs(
    Mcal_AdcIdType adc)
{
    volatile struct ADC_RESULT_REGS * resultRegs;

    resultRegs = NULL;

    switch(adc)
    {
        case MCAL_ADC_A:
            resultRegs = &AdcaResultRegs;
            break;

        case MCAL_ADC_B:
            resultRegs = &AdcbResultRegs;
            break;

        case MCAL_ADC_C:
            resultRegs = &AdccResultRegs;
            break;

        case MCAL_ADC_D:
            resultRegs = &AdcdResultRegs;
            break;

        default:
            /* Do nothing. */
            break;
    }

    return resultRegs;
}

static Mcal_AdcStatusType IsAdcValid(
    Mcal_AdcIdType adc)
{
    Mcal_AdcStatusType status;

    if((uint16_t)adc <= (uint16_t)MCAL_ADC_D)
    {
        status = MCAL_ADC_STATUS_OK;
    }
    else
    {
        status = MCAL_ADC_STATUS_INV_ID;
    }

    return status;
}

static Mcal_AdcStatusType IsSocValid(
    Mcal_AdcSocType soc)
{
    Mcal_AdcStatusType status;

    if((uint16_t)soc <= (uint16_t)MCAL_ADC_SOC_15)
    {
        status = MCAL_ADC_STATUS_OK;
    }
    else
    {
        status = MCAL_ADC_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_AdcStatusType IsChannelValid(
    Mcal_AdcChannelType channel)
{
    Mcal_AdcStatusType status;

    if((uint16_t)channel <=
       (uint16_t)MCAL_ADC_CHANNEL_15)
    {
        status = MCAL_ADC_STATUS_OK;
    }
    else
    {
        status = MCAL_ADC_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_AdcStatusType IsTriggerValid(
    Mcal_AdcTriggerType trigger)
{
    Mcal_AdcStatusType status;

    if((trigger == MCAL_ADC_TRIG_SW_ONLY) ||
       (trigger == MCAL_ADC_TRIG_EPWM1_SOCA) ||
       (trigger == MCAL_ADC_TRIG_EPWM1_SOCB))
    {
        status = MCAL_ADC_STATUS_OK;
    }
    else
    {
        status = MCAL_ADC_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_AdcStatusType IsSocConfigValid(
    const Mcal_AdcSocConfigType * config)
{
    Mcal_AdcStatusType status;

    if(config != NULL)
    {
        status = IsAdcValid(config->adc);

        if(status == MCAL_ADC_STATUS_OK)
        {
            status = IsSocValid(config->soc);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_ADC_STATUS_OK)
        {
            status = IsChannelValid(config->channel);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_ADC_STATUS_OK)
        {
            status = IsTriggerValid(config->trigger);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_ADC_STATUS_OK)
        {
            if((config->acquisitionCycles >=
                MCAL_ADC_ACQ_CYCLES_MIN) &&
               (config->acquisitionCycles <=
                MCAL_ADC_ACQ_CYCLES_MAX))
            {
                /* Do nothing. */
            }
            else
            {
                status = MCAL_ADC_STATUS_INV_ARG;
            }
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        status = MCAL_ADC_STATUS_INV_ARG;
    }

    return status;
}


static Mcal_AdcStatusType IsIntValid(
    Mcal_AdcIntType adcInt)
{
    Mcal_AdcStatusType status;

    if(((uint16_t)adcInt >= (uint16_t)MCAL_ADC_INT_1) &&
       ((uint16_t)adcInt <= (uint16_t)MCAL_ADC_INT_4))
    {
        status = MCAL_ADC_STATUS_OK;
    }
    else
    {
        status = MCAL_ADC_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_AdcStatusType IsIntConfigValid(
    const Mcal_AdcIntConfigType * config)
{
    Mcal_AdcStatusType status;

    if(config != NULL)
    {
        status = IsAdcValid(config->adc);

        if(status == MCAL_ADC_STATUS_OK)
        {
            status = IsIntValid(config->adcInt);
        }
        else
        {
            /* Do nothing. */
        }

        if(status == MCAL_ADC_STATUS_OK)
        {
            status = IsSocValid(config->sourceEoc);
        }
        else
        {
            /* Do nothing. */
        }
    }
    else
    {
        status = MCAL_ADC_STATUS_INV_ARG;
    }

    return status;
}

static void ConfigureInterrupt(
    volatile struct ADC_REGS * adcRegs,
    Mcal_AdcIntType adcInt,
    Mcal_AdcSocType sourceEoc)
{
    switch(adcInt)
    {
        case MCAL_ADC_INT_1:
            adcRegs->ADCINTSEL1N2.bit.INT1E =
                MCAL_ADC_INT_DISABLE;
            adcRegs->ADCINTSEL1N2.bit.INT1SEL =
                (uint16_t)sourceEoc;
            adcRegs->ADCINTSEL1N2.bit.INT1CONT =
                MCAL_ADC_INT_CONT_ENABLE;
            adcRegs->ADCINTFLGCLR.bit.ADCINT1 =
                MCAL_ADC_FLAG_CLEAR;
            adcRegs->ADCINTOVFCLR.bit.ADCINT1 =
                MCAL_ADC_FLAG_CLEAR;
            adcRegs->ADCINTSEL1N2.bit.INT1E =
                MCAL_ADC_INT_ENABLE;
            break;

        case MCAL_ADC_INT_2:
            adcRegs->ADCINTSEL1N2.bit.INT2E =
                MCAL_ADC_INT_DISABLE;
            adcRegs->ADCINTSEL1N2.bit.INT2SEL =
                (uint16_t)sourceEoc;
            adcRegs->ADCINTSEL1N2.bit.INT2CONT =
                MCAL_ADC_INT_CONT_ENABLE;
            adcRegs->ADCINTFLGCLR.bit.ADCINT2 =
                MCAL_ADC_FLAG_CLEAR;
            adcRegs->ADCINTOVFCLR.bit.ADCINT2 =
                MCAL_ADC_FLAG_CLEAR;
            adcRegs->ADCINTSEL1N2.bit.INT2E =
                MCAL_ADC_INT_ENABLE;
            break;

        case MCAL_ADC_INT_3:
            adcRegs->ADCINTSEL3N4.bit.INT3E =
                MCAL_ADC_INT_DISABLE;
            adcRegs->ADCINTSEL3N4.bit.INT3SEL =
                (uint16_t)sourceEoc;
            adcRegs->ADCINTSEL3N4.bit.INT3CONT =
                MCAL_ADC_INT_CONT_ENABLE;
            adcRegs->ADCINTFLGCLR.bit.ADCINT3 =
                MCAL_ADC_FLAG_CLEAR;
            adcRegs->ADCINTOVFCLR.bit.ADCINT3 =
                MCAL_ADC_FLAG_CLEAR;
            adcRegs->ADCINTSEL3N4.bit.INT3E =
                MCAL_ADC_INT_ENABLE;
            break;

        case MCAL_ADC_INT_4:
            adcRegs->ADCINTSEL3N4.bit.INT4E =
                MCAL_ADC_INT_DISABLE;
            adcRegs->ADCINTSEL3N4.bit.INT4SEL =
                (uint16_t)sourceEoc;
            adcRegs->ADCINTSEL3N4.bit.INT4CONT =
                MCAL_ADC_INT_CONT_ENABLE;
            adcRegs->ADCINTFLGCLR.bit.ADCINT4 =
                MCAL_ADC_FLAG_CLEAR;
            adcRegs->ADCINTOVFCLR.bit.ADCINT4 =
                MCAL_ADC_FLAG_CLEAR;
            adcRegs->ADCINTSEL3N4.bit.INT4E =
                MCAL_ADC_INT_ENABLE;
            break;

        default:
            /* Do nothing. */
            break;
    }
}

static void ClearIntFlag(
    volatile struct ADC_REGS * adcRegs,
    Mcal_AdcIntType adcInt)
{
    switch(adcInt)
    {
        case MCAL_ADC_INT_1:
            adcRegs->ADCINTFLGCLR.bit.ADCINT1 =
                MCAL_ADC_FLAG_CLEAR;
            break;

        case MCAL_ADC_INT_2:
            adcRegs->ADCINTFLGCLR.bit.ADCINT2 =
                MCAL_ADC_FLAG_CLEAR;
            break;

        case MCAL_ADC_INT_3:
            adcRegs->ADCINTFLGCLR.bit.ADCINT3 =
                MCAL_ADC_FLAG_CLEAR;
            break;

        case MCAL_ADC_INT_4:
            adcRegs->ADCINTFLGCLR.bit.ADCINT4 =
                MCAL_ADC_FLAG_CLEAR;
            break;

        default:
            /* Do nothing. */
            break;
    }
}

static uint16_t ReadIntOverflow(
    volatile struct ADC_REGS * adcRegs,
    Mcal_AdcIntType adcInt)
{
    uint16_t overflow;

    overflow = MCAL_ADC_FLAG_RESET;

    switch(adcInt)
    {
        case MCAL_ADC_INT_1:
            if(adcRegs->ADCINTOVF.bit.ADCINT1 != 0U)
            {
                overflow = MCAL_ADC_FLAG_SET;
            }
            else
            {
                /* Do nothing. */
            }
            break;

        case MCAL_ADC_INT_2:
            if(adcRegs->ADCINTOVF.bit.ADCINT2 != 0U)
            {
                overflow = MCAL_ADC_FLAG_SET;
            }
            else
            {
                /* Do nothing. */
            }
            break;

        case MCAL_ADC_INT_3:
            if(adcRegs->ADCINTOVF.bit.ADCINT3 != 0U)
            {
                overflow = MCAL_ADC_FLAG_SET;
            }
            else
            {
                /* Do nothing. */
            }
            break;

        case MCAL_ADC_INT_4:
            if(adcRegs->ADCINTOVF.bit.ADCINT4 != 0U)
            {
                overflow = MCAL_ADC_FLAG_SET;
            }
            else
            {
                /* Do nothing. */
            }
            break;

        default:
            /* Do nothing. */
            break;
    }

    return overflow;
}

static void ClearIntOverflow(
    volatile struct ADC_REGS * adcRegs,
    Mcal_AdcIntType adcInt)
{
    switch(adcInt)
    {
        case MCAL_ADC_INT_1:
            adcRegs->ADCINTOVFCLR.bit.ADCINT1 =
                MCAL_ADC_FLAG_CLEAR;
            break;

        case MCAL_ADC_INT_2:
            adcRegs->ADCINTOVFCLR.bit.ADCINT2 =
                MCAL_ADC_FLAG_CLEAR;
            break;

        case MCAL_ADC_INT_3:
            adcRegs->ADCINTOVFCLR.bit.ADCINT3 =
                MCAL_ADC_FLAG_CLEAR;
            break;

        case MCAL_ADC_INT_4:
            adcRegs->ADCINTOVFCLR.bit.ADCINT4 =
                MCAL_ADC_FLAG_CLEAR;
            break;

        default:
            /* Do nothing. */
            break;
    }
}

static uint16_t GetTiAdcId(
    Mcal_AdcIdType adc)
{
    uint16_t tiAdcId;

    tiAdcId = ADC_ADCA;

    switch(adc)
    {
        case MCAL_ADC_A:
            tiAdcId = ADC_ADCA;
            break;

        case MCAL_ADC_B:
            tiAdcId = ADC_ADCB;
            break;

        case MCAL_ADC_C:
            tiAdcId = ADC_ADCC;
            break;

        case MCAL_ADC_D:
            tiAdcId = ADC_ADCD;
            break;

        default:
            /* Do nothing. */
            break;
    }

    return tiAdcId;
}

static void ConfigureSoc(
    volatile struct ADC_REGS * adcRegs,
    Mcal_AdcSocType soc,
    uint16_t channel,
    uint16_t trigger,
    uint16_t acqps)
{
    switch(soc)
    {
        case MCAL_ADC_SOC_0:
            adcRegs->ADCSOC0CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC0CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC0CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_1:
            adcRegs->ADCSOC1CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC1CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC1CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_2:
            adcRegs->ADCSOC2CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC2CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC2CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_3:
            adcRegs->ADCSOC3CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC3CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC3CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_4:
            adcRegs->ADCSOC4CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC4CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC4CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_5:
            adcRegs->ADCSOC5CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC5CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC5CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_6:
            adcRegs->ADCSOC6CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC6CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC6CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_7:
            adcRegs->ADCSOC7CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC7CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC7CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_8:
            adcRegs->ADCSOC8CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC8CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC8CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_9:
            adcRegs->ADCSOC9CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC9CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC9CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_10:
            adcRegs->ADCSOC10CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC10CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC10CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_11:
            adcRegs->ADCSOC11CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC11CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC11CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_12:
            adcRegs->ADCSOC12CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC12CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC12CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_13:
            adcRegs->ADCSOC13CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC13CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC13CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_14:
            adcRegs->ADCSOC14CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC14CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC14CTL.bit.ACQPS = acqps;
            break;

        case MCAL_ADC_SOC_15:
            adcRegs->ADCSOC15CTL.bit.CHSEL = channel;
            adcRegs->ADCSOC15CTL.bit.TRIGSEL = trigger;
            adcRegs->ADCSOC15CTL.bit.ACQPS = acqps;
            break;

        default:
            /* Do nothing. */
            break;
    }
}

static uint16_t ReadResult(
    volatile struct ADC_RESULT_REGS * resultRegs,
    Mcal_AdcSocType soc)
{
    uint16_t result;

    result = 0U;

    switch(soc)
    {
        case MCAL_ADC_SOC_0:
            result = resultRegs->ADCRESULT0;
            break;

        case MCAL_ADC_SOC_1:
            result = resultRegs->ADCRESULT1;
            break;

        case MCAL_ADC_SOC_2:
            result = resultRegs->ADCRESULT2;
            break;

        case MCAL_ADC_SOC_3:
            result = resultRegs->ADCRESULT3;
            break;

        case MCAL_ADC_SOC_4:
            result = resultRegs->ADCRESULT4;
            break;

        case MCAL_ADC_SOC_5:
            result = resultRegs->ADCRESULT5;
            break;

        case MCAL_ADC_SOC_6:
            result = resultRegs->ADCRESULT6;
            break;

        case MCAL_ADC_SOC_7:
            result = resultRegs->ADCRESULT7;
            break;

        case MCAL_ADC_SOC_8:
            result = resultRegs->ADCRESULT8;
            break;

        case MCAL_ADC_SOC_9:
            result = resultRegs->ADCRESULT9;
            break;

        case MCAL_ADC_SOC_10:
            result = resultRegs->ADCRESULT10;
            break;

        case MCAL_ADC_SOC_11:
            result = resultRegs->ADCRESULT11;
            break;

        case MCAL_ADC_SOC_12:
            result = resultRegs->ADCRESULT12;
            break;

        case MCAL_ADC_SOC_13:
            result = resultRegs->ADCRESULT13;
            break;

        case MCAL_ADC_SOC_14:
            result = resultRegs->ADCRESULT14;
            break;

        case MCAL_ADC_SOC_15:
            result = resultRegs->ADCRESULT15;
            break;

        default:
            /* Do nothing. */
            break;
    }

    return result;
}
