/**
 * @file    bsp_motor_hw.c
 * @brief   Motor-control hardware mapping for LAUNCHXL-F28379D.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "bsp_motor_hw.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define BSP_PWM_MUX_EPWM    (1U)

/*==============================================================================
 * Private Variables
 *============================================================================*/

/*==============================================================================
* Private Function Declarations
*============================================================================*/

static Bsp_MotorHwStatusType InitPwmPhase(
    const Bsp_MotorPwmPhaseType * phase);
    
/*==============================================================================
* Public Function Definitions
*============================================================================*/

static const Bsp_MotorPwmHwType MotorPwmHw =
{
    {
        MCAL_EPWM_1,
        0U,
        1U,
        BSP_PWM_MUX_EPWM,
        BSP_PWM_MUX_EPWM
    },
    {
        MCAL_EPWM_2,
        2U,
        3U,
        BSP_PWM_MUX_EPWM,
        BSP_PWM_MUX_EPWM
    },
    {
        MCAL_EPWM_3,
        4U,
        5U,
        BSP_PWM_MUX_EPWM,
        BSP_PWM_MUX_EPWM
    }
};

const Bsp_MotorPwmHwType * Bsp_MotorHw_GetPwmConfig(void)
{
    return &MotorPwmHw;
}

Bsp_MotorHwStatusType Bsp_MotorHw_Init(void)
{
    Bsp_MotorHwStatusType status;

    status = InitPwmPhase(&MotorPwmHw.phaseU);

    if(status == BSP_MOTOR_HW_STATUS_OK)
    {
        status = InitPwmPhase(&MotorPwmHw.phaseV);

        if(status == BSP_MOTOR_HW_STATUS_OK)
        {
            status = InitPwmPhase(&MotorPwmHw.phaseW);
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


/*==============================================================================
 * Private Function Definitions
 *============================================================================*/
 
static Bsp_MotorHwStatusType InitPwmPhase(
    const Bsp_MotorPwmPhaseType * phase)
{
    Bsp_MotorHwStatusType status;
    Mcal_GpioStatusType gpioStatus;

    status = BSP_MOTOR_HW_STATUS_INIT_FAILED;

    gpioStatus = Mcal_Gpio_SetMux(
        phase->highPin,
        phase->highMux);

    if(gpioStatus == MCAL_GPIO_STATUS_OK)
    {
        gpioStatus = Mcal_Gpio_SetMux(
            phase->lowPin,
            phase->lowMux);

        if(gpioStatus == MCAL_GPIO_STATUS_OK)
        {
            status = BSP_MOTOR_HW_STATUS_OK;
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
