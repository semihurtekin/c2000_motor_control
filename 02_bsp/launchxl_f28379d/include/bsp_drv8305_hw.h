/**
 * @file    bsp_drv8305_hw.h
 * @brief   DRV8305 gate-driver hardware mapping for LAUNCHXL-F28379D.
 */

#ifndef BSP_DRV8305_HW_H
#define BSP_DRV8305_HW_H

#include <stdint.h>

#include "mcal_gpio.h"
#include "mcal_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BSP_DRV8305_HW_STATUS_OK = 0U,
    BSP_DRV8305_HW_STATUS_INIT_FAILED
} Bsp_Drv8305HwStatusType;

typedef struct
{
    Mcal_SpiIdType spiModule;
    Mcal_GpioPinType nScsPin;
    Mcal_GpioPinType enableGatePin;
    Mcal_GpioPinType wakePin;
    Mcal_GpioPinType faultPin;
    Mcal_GpioPinType powerGoodPin;
} Bsp_Drv8305HwType;

Bsp_Drv8305HwStatusType Bsp_Drv8305Hw_Init(void);

const Bsp_Drv8305HwType * Bsp_Drv8305Hw_GetHwConfig(void);

uint32_t Bsp_Drv8305_GetSpiClkHz(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DRV8305_HW_H */

