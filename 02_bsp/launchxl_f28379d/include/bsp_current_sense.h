/**
 * @file    bsp_current_sense.h
 * @brief   Current-sense hardware mapping for LAUNCHXL-F28379D.
 */

#ifndef BSP_CURRENT_SENSE_H
#define BSP_CURRENT_SENSE_H

/*==============================================================================
 * Includes
 *============================================================================*/

#include "mcal_adc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * Public Types
 *============================================================================*/

typedef enum
{
    BSP_CURRENT_SENSE_STATUS_OK = 0U,
    BSP_CURRENT_SENSE_STATUS_INIT_FAILED
} Bsp_CurrentSenseStatusType;

typedef struct
{
    Mcal_AdcIdType adc;
    Mcal_AdcSocType soc;
    Mcal_AdcChannelType channel;
} Bsp_CurrentSenseInputType;

typedef struct
{
    Bsp_CurrentSenseInputType phaseU;
    Bsp_CurrentSenseInputType phaseV;
} Bsp_CurrentSenseHwType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

Bsp_CurrentSenseStatusType Bsp_CurrentSense_Init(void);

const Bsp_CurrentSenseHwType * Bsp_CurrentSense_GetHwConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CURRENT_SENSE_H */

