/**
 * @file    bsp_drv8305_hw.c
 * @brief   DRV8305 gate-driver hardware mapping for LAUNCHXL-F28379D.
 */

#include "bsp_drv8305_hw.h"

#include "platform_clock.h"

#define BSP_DRV8305_MOSI_PIN          (58U)
#define BSP_DRV8305_MISO_PIN          (59U)
#define BSP_DRV8305_SCLK_PIN          (60U)

#define BSP_DRV8305_SPI_MUX           (15U)

#define BSP_DRV8305_N_SCS_PIN         (61U)
#define BSP_DRV8305_N_FAULT_PIN       (19U)
#define BSP_DRV8305_PWRGD_PIN         (123U)
#define BSP_DRV8305_EN_GATE_PIN       (124U)
#define BSP_DRV8305_WAKE_PIN          (125U)

static const Bsp_Drv8305HwType Drv8305Hw =
{
    MCAL_SPI_A,
    BSP_DRV8305_N_SCS_PIN,
    BSP_DRV8305_EN_GATE_PIN,
    BSP_DRV8305_WAKE_PIN,
    BSP_DRV8305_N_FAULT_PIN,
    BSP_DRV8305_PWRGD_PIN
};

static Bsp_Drv8305HwStatusType InitSafeControlPins(
    const Bsp_Drv8305HwType * config);

static Bsp_Drv8305HwStatusType InitStatusPins(
    const Bsp_Drv8305HwType * config);

static Bsp_Drv8305HwStatusType EnableSpiClock(
    Mcal_SpiIdType module);

static Bsp_Drv8305HwStatusType InitSpiPins(void);

static Mcal_GpioStatusType InitOutputPin(
    Mcal_GpioPinType pin,
    Mcal_GpioLevelType initLevel);

static Mcal_GpioStatusType InitInputPin(
    Mcal_GpioPinType pin);

Bsp_Drv8305HwStatusType Bsp_Drv8305Hw_Init(void)
{
    Bsp_Drv8305HwStatusType status;

    status = InitSafeControlPins(&Drv8305Hw);

    if(status == BSP_DRV8305_HW_STATUS_OK)
    {
        status = InitStatusPins(&Drv8305Hw);
    }
    else
    {
        /* Keep the already-established safe states unchanged. */
    }

    if(status == BSP_DRV8305_HW_STATUS_OK)
    {
        status = EnableSpiClock(Drv8305Hw.spiModule);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == BSP_DRV8305_HW_STATUS_OK)
    {
        status = InitSpiPins();
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

const Bsp_Drv8305HwType * Bsp_Drv8305Hw_GetHwConfig(void)
{
    return &Drv8305Hw;
}

uint32_t Bsp_Drv8305_GetSpiClkHz(void)
{
    return Platform_ClockGetLspClkHz();
}

static Bsp_Drv8305HwStatusType InitSafeControlPins(
    const Bsp_Drv8305HwType * config)
{
    Bsp_Drv8305HwStatusType status;
    Mcal_GpioStatusType gpioStatus;

    status = BSP_DRV8305_HW_STATUS_OK;

    gpioStatus =
        InitOutputPin(
            config->nScsPin,
            MCAL_GPIO_LEVEL_HIGH);

    if(gpioStatus != MCAL_GPIO_STATUS_OK)
    {
        status = BSP_DRV8305_HW_STATUS_INIT_FAILED;
    }
    else
    {
        /* Do nothing. */
    }

    gpioStatus =
        InitOutputPin(
            config->enableGatePin,
            MCAL_GPIO_LEVEL_LOW);

    if(gpioStatus != MCAL_GPIO_STATUS_OK)
    {
        status = BSP_DRV8305_HW_STATUS_INIT_FAILED;
    }
    else
    {
        /* Do nothing. */
    }

    gpioStatus =
        InitOutputPin(
            config->wakePin,
            MCAL_GPIO_LEVEL_LOW);

    if(gpioStatus != MCAL_GPIO_STATUS_OK)
    {
        status = BSP_DRV8305_HW_STATUS_INIT_FAILED;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

static Bsp_Drv8305HwStatusType InitStatusPins(
    const Bsp_Drv8305HwType * config)
{
    Bsp_Drv8305HwStatusType status;
    Mcal_GpioStatusType gpioStatus;

    status = BSP_DRV8305_HW_STATUS_INIT_FAILED;

    gpioStatus = InitInputPin(config->faultPin);

    if(gpioStatus == MCAL_GPIO_STATUS_OK)
    {
        gpioStatus = InitInputPin(config->powerGoodPin);
    }
    else
    {
        /* Do nothing. */
    }

    if(gpioStatus == MCAL_GPIO_STATUS_OK)
    {
        status = BSP_DRV8305_HW_STATUS_OK;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

static Bsp_Drv8305HwStatusType EnableSpiClock(
    Mcal_SpiIdType module)
{
    Bsp_Drv8305HwStatusType status;
    Platform_ClockStatusType clockStatus;

    status = BSP_DRV8305_HW_STATUS_INIT_FAILED;
    clockStatus = PLATFORM_CLOCK_STATUS_INVALID_PARAM;

    switch(module)
    {
        case MCAL_SPI_A:
            clockStatus =
                Platform_ClockEnableSpi(
                    PLATFORM_SPI_MODULE_A);
            break;

        case MCAL_SPI_B:
            clockStatus =
                Platform_ClockEnableSpi(
                    PLATFORM_SPI_MODULE_B);
            break;

        case MCAL_SPI_C:
            clockStatus =
                Platform_ClockEnableSpi(
                    PLATFORM_SPI_MODULE_C);
            break;

        default:
            /* Invalid SPI mapping. */
            break;
    }

    if(clockStatus == PLATFORM_CLOCK_STATUS_OK)
    {
        status = BSP_DRV8305_HW_STATUS_OK;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

static Bsp_Drv8305HwStatusType InitSpiPins(void)
{
    Bsp_Drv8305HwStatusType status;
    Mcal_GpioStatusType gpioStatus;

    status = BSP_DRV8305_HW_STATUS_INIT_FAILED;

    gpioStatus =
        Mcal_Gpio_SetMux(
            BSP_DRV8305_MOSI_PIN,
            BSP_DRV8305_SPI_MUX);

    if(gpioStatus == MCAL_GPIO_STATUS_OK)
    {
        gpioStatus =
            Mcal_Gpio_SetMux(
                BSP_DRV8305_MISO_PIN,
                BSP_DRV8305_SPI_MUX);
    }
    else
    {
        /* Do nothing. */
    }

    if(gpioStatus == MCAL_GPIO_STATUS_OK)
    {
        gpioStatus =
            Mcal_Gpio_SetQualMode(
                BSP_DRV8305_MISO_PIN,
                MCAL_GPIO_QUAL_ASYNC);
    }
    else
    {
        /* Do nothing. */
    }

    if(gpioStatus == MCAL_GPIO_STATUS_OK)
    {
        gpioStatus =
            Mcal_Gpio_SetMux(
                BSP_DRV8305_SCLK_PIN,
                BSP_DRV8305_SPI_MUX);
    }
    else
    {
        /* Do nothing. */
    }

    if(gpioStatus == MCAL_GPIO_STATUS_OK)
    {
        status = BSP_DRV8305_HW_STATUS_OK;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

static Mcal_GpioStatusType InitOutputPin(
    Mcal_GpioPinType pin,
    Mcal_GpioLevelType initLevel)
{
    Mcal_GpioConfigType gpioConfig;

    gpioConfig.pin = pin;
    gpioConfig.dir = MCAL_GPIO_DIR_OUTPUT;
    gpioConfig.pull = MCAL_GPIO_PULL_DISABLE;
    gpioConfig.odr = MCAL_GPIO_ODR_DISABLE;
    gpioConfig.inv = MCAL_GPIO_INV_DISABLE;
    gpioConfig.qual = MCAL_GPIO_QUAL_SYNC;
    gpioConfig.owner = MCAL_GPIO_OWNER_CPU1;
    gpioConfig.initLevel = initLevel;

    return Mcal_Gpio_InitPin(&gpioConfig);
}

static Mcal_GpioStatusType InitInputPin(
    Mcal_GpioPinType pin)
{
    Mcal_GpioConfigType gpioConfig;

    gpioConfig.pin = pin;
    gpioConfig.dir = MCAL_GPIO_DIR_INPUT;
    gpioConfig.pull = MCAL_GPIO_PULL_DISABLE;
    gpioConfig.odr = MCAL_GPIO_ODR_DISABLE;
    gpioConfig.inv = MCAL_GPIO_INV_DISABLE;
    gpioConfig.qual = MCAL_GPIO_QUAL_SYNC;
    gpioConfig.owner = MCAL_GPIO_OWNER_CPU1;
    gpioConfig.initLevel = MCAL_GPIO_LEVEL_LOW;

    return Mcal_Gpio_InitPin(&gpioConfig);
}
