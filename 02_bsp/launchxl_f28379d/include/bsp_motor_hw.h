/**
 * @file    bsp_motor_hw.h
 * @brief   Motor-control hardware mapping for LAUNCHXL-F28379D.
 */

#ifndef BSP_MOTOR_HW_H
#define BSP_MOTOR_HW_H

/*==============================================================================
 * Includes
 *============================================================================*/

#include "mcal_epwm.h"
#include "mcal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * Public Types
 *============================================================================*/

typedef enum
{
    BSP_MOTOR_HW_STATUS_OK = 0U,
    BSP_MOTOR_HW_STATUS_INIT_FAILED = 1U
} Bsp_MotorHwStatusType;

typedef struct
{
    Mcal_EpwmIdType module;
    Mcal_GpioPinType highPin;
    Mcal_GpioPinType lowPin;
    Mcal_GpioMuxType highMux;
    Mcal_GpioMuxType lowMux;
} Bsp_MotorPwmPhaseType;

typedef struct
{
    Bsp_MotorPwmPhaseType phaseU;
    Bsp_MotorPwmPhaseType phaseV;
    Bsp_MotorPwmPhaseType phaseW;
} Bsp_MotorPwmHwType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

Bsp_MotorHwStatusType Bsp_MotorHw_Init(void);

const Bsp_MotorPwmHwType * Bsp_MotorHw_GetPwmConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_MOTOR_HW_H */
