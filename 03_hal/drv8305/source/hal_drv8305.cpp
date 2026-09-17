/**
 * @file    hal_drv8305.cpp
 * @brief   DRV8305 gate-driver hardware abstraction implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "hal_drv8305.hpp"

#include "mcal_gpio.h"
#include "mcal_spi.h"
#include "platform_delay.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define DRV8305_SPI_BIT_RATE_HZ          (1000000UL)

#define DRV8305_CMD_WRITE                (0U)
#define DRV8305_CMD_READ                 (1U)

#define DRV8305_RW_SHIFT                 (15U)
#define DRV8305_ADDR_SHIFT               (11U)
#define DRV8305_ADDR_MASK                (0x000FU)
#define DRV8305_DATA_MASK                (0x07FFU)

#define DRV8305_COMM_OPTION_ACTIVE_FW    (0x0200U)
#define DRV8305_OTSD_ENABLE              (0x0400U)
#define DRV8305_SNS_CLAMP_ENABLE         (0x0080U)
#define DRV8305_WD_DELAY_20MS            (0x0020U)
#define DRV8305_SLEEP_DELAY_10US         (0x0008U)
#define DRV8305_VREG_UV_LEVEL_70PCT      (0x0002U)

#define DRV8305_HS_VERIFY_MASK           (0x03FFU)
#define DRV8305_LS_VERIFY_MASK           (0x03FFU)
#define DRV8305_GD_VERIFY_MASK           (0x03FFU)
#define DRV8305_IC_VERIFY_MASK           (0x07FFU)
#define DRV8305_CSA_VERIFY_MASK          (0x07FFU)
#define DRV8305_VREG_VERIFY_MASK         (0x0300U)
#define DRV8305_VDS_VERIFY_MASK          (0x00FFU)

/*==============================================================================
 * Private Types
 *============================================================================*/

struct Drv8305RegisterImages
{
    uint16_t hsGateDrive;
    uint16_t lsGateDrive;
    uint16_t gateControl;
    uint16_t icOperation;
    uint16_t shuntAmp;
    uint16_t voltageReg;
    uint16_t vdsSense;
};

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

namespace
{

static Hal::Drv8305Status ValidateConfig(
    const Hal::Drv8305Config& config);

static Hal::Drv8305Status InitializeBsp(
    const Bsp_Drv8305HwType *& hwConfig);

static Hal::Drv8305Status ConfigureDrvHw(
    const Hal::Drv8305Config& config,
    const Bsp_Drv8305HwType * hwConfig);

static Hal::Drv8305Status ForceSafeState(
    const Bsp_Drv8305HwType * hwConfig);

static void BuildRegisterImages(
    const Hal::Drv8305Config& config,
    Drv8305RegisterImages& images);

static uint16_t BuildHsGateDriveReg(
    const Hal::Drv8305GateDriveConfig& config);

static uint16_t BuildLsGateDriveReg(
    const Hal::Drv8305GateDriveConfig& config);

static uint16_t BuildGateControlReg(
    const Hal::Drv8305Config& config);

static uint16_t BuildIcOperationReg(
    const Hal::Drv8305CurrentSenseConfig& config);

static uint16_t BuildShuntAmpReg(
    const Hal::Drv8305CurrentSenseConfig& config);

static uint16_t BuildVoltageRegReg(
    const Hal::Drv8305CurrentSenseConfig& config);

static uint16_t BuildVdsSenseReg(
    const Hal::Drv8305ProtectionConfig& config);

static Hal::Drv8305Status WriteConfiguration(
    const Drv8305RegisterImages& images,
    const Bsp_Drv8305HwType * hwConfig);

static Hal::Drv8305Status VerifyConfiguration(
    const Drv8305RegisterImages& images,
    const Bsp_Drv8305HwType * hwConfig);

static Hal::Drv8305Status VerifyRegister(
    Hal::Drv8305Address address,
    uint16_t expectedData,
    uint16_t verifyMask,
    const Bsp_Drv8305HwType * hwConfig);

static Hal::Drv8305Status WriteRegister(
    Hal::Drv8305Address address,
    uint16_t data,
    const Bsp_Drv8305HwType * hwConfig);

static Hal::Drv8305Status ReadRegister(
    Hal::Drv8305Address address,
    uint16_t& data,
    const Bsp_Drv8305HwType * hwConfig);

static uint16_t BuildFrame(
    bool isRead,
    Hal::Drv8305Address address,
    uint16_t data);

static Hal::Drv8305Status TransferFrame(
    uint16_t txFrame,
    uint16_t& rxFrame,
    const Bsp_Drv8305HwType * hwConfig);
    
}

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

namespace Hal
{

Drv8305::Drv8305()
    : state_(DRV8305_STATE_UNINITIALIZED),
      hwConfig_(0)
{
}

Drv8305Status Drv8305::Init(
    const Drv8305Config& config)
{
    Drv8305Status status;
    const Bsp_Drv8305HwType * hwConfig;

    hwConfig = 0;

    if(state_ == DRV8305_STATE_UNINITIALIZED)
    {
        status = ValidateConfig(config);
    }
    else
    {
        status = DRV8305_STATUS_ALREADY_INITIALIZED;
    }

    if(status == DRV8305_STATUS_OK)
    {
        status = InitializeBsp(hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == DRV8305_STATUS_OK)
    {
        status = ConfigureDrvHw(config, hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == DRV8305_STATUS_OK)
    {
        hwConfig_ = hwConfig;
        config_ = config;
        state_ = DRV8305_STATE_CONFIGURED;
    }
    else
    {
        if(hwConfig != 0)
        {
            const Drv8305Status safeStatus =
                ForceSafeState(hwConfig);

            if(safeStatus != DRV8305_STATUS_OK)
            {
                status = DRV8305_STATUS_HW_ERROR;
            }
            else
            {
                /* Preserve the original initialization error. */
            }
        }
        else
        {
            /* No hardware binding was established. */
        }
    }

    return status;
}

} /* namespace Hal */

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

namespace
{

static Hal::Drv8305Status ValidateConfig(
    const Hal::Drv8305Config& config)
{
    Hal::Drv8305Status status;

    status = Hal::DRV8305_STATUS_OK;

    if(((uint16_t)config.gateDrive.hsSourceCurrent >
        (uint16_t)Hal::DRV8305_GATE_SOURCE_1000MA) ||
       ((uint16_t)config.gateDrive.lsSourceCurrent >
        (uint16_t)Hal::DRV8305_GATE_SOURCE_1000MA) ||
       ((uint16_t)config.gateDrive.hsSinkCurrent >
        (uint16_t)Hal::DRV8305_GATE_SINK_1250MA) ||
       ((uint16_t)config.gateDrive.lsSinkCurrent >
        (uint16_t)Hal::DRV8305_GATE_SINK_1250MA) ||
       ((uint16_t)config.gateDrive.hsDriveTime >
        (uint16_t)Hal::DRV8305_DRIVE_TIME_1780NS) ||
       ((uint16_t)config.gateDrive.lsDriveTime >
        (uint16_t)Hal::DRV8305_DRIVE_TIME_1780NS) ||
       ((uint16_t)config.gateDrive.deadTime >
        (uint16_t)Hal::DRV8305_DEAD_TIME_5280NS) ||
       ((uint16_t)config.currentSense.gain >
        (uint16_t)Hal::DRV8305_CSA_GAIN_80) ||
       ((uint16_t)config.currentSense.blanking >
        (uint16_t)Hal::DRV8305_CSA_BLANK_10000NS) ||
       ((config.currentSense.vrefScale !=
         Hal::DRV8305_VREF_SCALE_DIV_2) &&
        (config.currentSense.vrefScale !=
         Hal::DRV8305_VREF_SCALE_DIV_4)) ||
       ((uint16_t)config.protection.vdsThreshold >
        (uint16_t)Hal::DRV8305_VDS_THRESHOLD_2131MV) ||
       ((uint16_t)config.protection.vdsMode >
        (uint16_t)Hal::DRV8305_VDS_MODE_DISABLED) ||
       ((uint16_t)config.protection.vdsBlanking >
        (uint16_t)Hal::DRV8305_VDS_BLANK_7000NS) ||
       ((uint16_t)config.protection.vdsDeglitch >
        (uint16_t)Hal::DRV8305_VDS_DEGLITCH_7000NS))
    {
        status = Hal::DRV8305_STATUS_INVALID_ARG;
    }
    else
    {
        /* Configuration values are valid. */
    }

    return status;
}

static Hal::Drv8305Status InitializeBsp(
    const Bsp_Drv8305HwType *& hwConfig)
{
    Hal::Drv8305Status status;
    Bsp_Drv8305HwStatusType bspStatus;

    status = Hal::DRV8305_STATUS_HW_ERROR;
    hwConfig = 0;

    bspStatus = Bsp_Drv8305Hw_Init();

    if(bspStatus == BSP_DRV8305_HW_STATUS_OK)
    {
        hwConfig = Bsp_Drv8305Hw_GetHwConfig();

        if(hwConfig != 0)
        {
            status = ForceSafeState(hwConfig);
        }
        else
        {
            /* BSP returned no valid hardware configuration. */
        }
    }
    else
    {
        /* BSP initialization failed. */
    }

    return status;
}

static Hal::Drv8305Status ConfigureDrvHw(
    const Hal::Drv8305Config& config,
    const Bsp_Drv8305HwType * hwConfig)
{
    Hal::Drv8305Status status;
    Mcal_SpiStatusType spiStatus;
    Mcal_SpiConfigType spiConfig;
    Drv8305RegisterImages images;

    spiConfig.module = hwConfig->spiModule;
    spiConfig.mode = MCAL_SPI_MODE_1;
    spiConfig.sourceClockHz = Bsp_Drv8305_GetSpiClkHz();
    spiConfig.bitRateHz = DRV8305_SPI_BIT_RATE_HZ;
    spiConfig.dataWidth = MCAL_SPI_WIDTH_16;

    spiStatus = Mcal_Spi_Init(&spiConfig);

    if(spiStatus == MCAL_SPI_STATUS_OK)
    {
        BuildRegisterImages(config, images);
        status = WriteConfiguration(images, hwConfig);
    }
    else
    {
        status = Hal::DRV8305_STATUS_SPI_ERROR;
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = VerifyConfiguration(images, hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

static Hal::Drv8305Status ForceSafeState(
    const Bsp_Drv8305HwType * hwConfig)
{
    Hal::Drv8305Status status;
    Mcal_GpioStatusType gpioStatus;

    status = Hal::DRV8305_STATUS_OK;

    gpioStatus =
        Mcal_Gpio_Write(
            hwConfig->enableGatePin,
            MCAL_GPIO_LEVEL_LOW);

    if(gpioStatus != MCAL_GPIO_STATUS_OK)
    {
        status = Hal::DRV8305_STATUS_HW_ERROR;
    }
    else
    {
        /* Do nothing. */
    }

    gpioStatus =
        Mcal_Gpio_Write(
            hwConfig->nScsPin,
            MCAL_GPIO_LEVEL_HIGH);

    if(gpioStatus != MCAL_GPIO_STATUS_OK)
    {
        status = Hal::DRV8305_STATUS_HW_ERROR;
    }
    else
    {
        /* Do nothing. */
    }

    gpioStatus =
        Mcal_Gpio_Write(
            hwConfig->wakePin,
            MCAL_GPIO_LEVEL_LOW);

    if(gpioStatus != MCAL_GPIO_STATUS_OK)
    {
        status = Hal::DRV8305_STATUS_HW_ERROR;
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

static void BuildRegisterImages(
    const Hal::Drv8305Config& config,
    Drv8305RegisterImages& images)
{
    images.hsGateDrive = BuildHsGateDriveReg(config.gateDrive);
    images.lsGateDrive = BuildLsGateDriveReg(config.gateDrive);
    images.gateControl = BuildGateControlReg(config);
    images.icOperation = BuildIcOperationReg(config.currentSense);
    images.shuntAmp = BuildShuntAmpReg(config.currentSense);
    images.voltageReg = BuildVoltageRegReg(config.currentSense);
    images.vdsSense = BuildVdsSenseReg(config.protection);
}

static uint16_t BuildHsGateDriveReg(
    const Hal::Drv8305GateDriveConfig& config)
{
    uint16_t data;

    data =
        ((uint16_t)config.hsDriveTime << 8U) |
        ((uint16_t)config.hsSinkCurrent << 4U) |
        (uint16_t)config.hsSourceCurrent;

    return data;
}

static uint16_t BuildLsGateDriveReg(
    const Hal::Drv8305GateDriveConfig& config)
{
    uint16_t data;

    data =
        ((uint16_t)config.lsDriveTime << 8U) |
        ((uint16_t)config.lsSinkCurrent << 4U) |
        (uint16_t)config.lsSourceCurrent;

    return data;
}

static uint16_t BuildGateControlReg(
    const Hal::Drv8305Config& config)
{
    uint16_t data;

    /* PWM_MODE remains 00: six independent PWM inputs. */
    data =
        DRV8305_COMM_OPTION_ACTIVE_FW |
        ((uint16_t)config.gateDrive.deadTime << 4U) |
        ((uint16_t)config.protection.vdsBlanking << 2U) |
        (uint16_t)config.protection.vdsDeglitch;

    return data;
}

static uint16_t BuildIcOperationReg(
    const Hal::Drv8305CurrentSenseConfig& config)
{
    uint16_t data;

    /*
     * v0.1 policy:
     * OTSD, PVDD_UVLO2, gate-drive fault and SNS OCP protection enabled.
     * Watchdog disabled, device awake, CLR_FLTS not asserted.
     */
    data =
        DRV8305_OTSD_ENABLE |
        DRV8305_WD_DELAY_20MS;

    if(config.outputClampEnabled == true)
    {
        data |= DRV8305_SNS_CLAMP_ENABLE;
    }
    else
    {
        /* Sense-amplifier output clamp disabled. */
    }

    return data;
}

static uint16_t BuildShuntAmpReg(
    const Hal::Drv8305CurrentSenseConfig& config)
{
    uint16_t data;

    data =
        ((uint16_t)config.blanking << 6U) |
        ((uint16_t)config.gain << 4U) |
        ((uint16_t)config.gain << 2U) |
        (uint16_t)config.gain;

    return data;
}

static uint16_t BuildVoltageRegReg(
    const Hal::Drv8305CurrentSenseConfig& config)
{
    uint16_t data;

    data =
        ((uint16_t)config.vrefScale << 8U) |
        DRV8305_SLEEP_DELAY_10US |
        DRV8305_VREG_UV_LEVEL_70PCT;

    return data;
}

static uint16_t BuildVdsSenseReg(
    const Hal::Drv8305ProtectionConfig& config)
{
    uint16_t data;

    data =
        ((uint16_t)config.vdsThreshold << 3U) |
        (uint16_t)config.vdsMode;

    return data;
}

static Hal::Drv8305Status WriteConfiguration(
    const Drv8305RegisterImages& images,
    const Bsp_Drv8305HwType * hwConfig)
{
    Hal::Drv8305Status status;

    status = WriteRegister(Hal::DRV8305_ADDR_HS_GD_CTRL,
                           images.hsGateDrive,
                           hwConfig);

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = WriteRegister(Hal::DRV8305_ADDR_LS_GD_CTRL,
                               images.lsGateDrive,
                               hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = WriteRegister(Hal::DRV8305_ADDR_GD_CTRL,
                               images.gateControl,
                               hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = WriteRegister(Hal::DRV8305_ADDR_IC_OPERATION,
                               images.icOperation,
                               hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = WriteRegister(Hal::DRV8305_ADDR_SHUNT_AMP_CTRL,
                               images.shuntAmp,
                               hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = WriteRegister(Hal::DRV8305_ADDR_VOLTAGE_REG_CTRL,
                               images.voltageReg,
                               hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = WriteRegister(Hal::DRV8305_ADDR_VDS_SENSE_CTRL,
                               images.vdsSense,
                               hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

static Hal::Drv8305Status VerifyConfiguration(
    const Drv8305RegisterImages& images,
    const Bsp_Drv8305HwType * hwConfig)
{
    Hal::Drv8305Status status;

    status = VerifyRegister(Hal::DRV8305_ADDR_HS_GD_CTRL,
                            images.hsGateDrive,
                            DRV8305_HS_VERIFY_MASK,
                            hwConfig);

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = VerifyRegister(Hal::DRV8305_ADDR_LS_GD_CTRL,
                                images.lsGateDrive,
                                DRV8305_LS_VERIFY_MASK,
                                hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = VerifyRegister(Hal::DRV8305_ADDR_GD_CTRL,
                                images.gateControl,
                                DRV8305_GD_VERIFY_MASK,
                                hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = VerifyRegister(Hal::DRV8305_ADDR_IC_OPERATION,
                                images.icOperation,
                                DRV8305_IC_VERIFY_MASK,
                                hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = VerifyRegister(Hal::DRV8305_ADDR_SHUNT_AMP_CTRL,
                                images.shuntAmp,
                                DRV8305_CSA_VERIFY_MASK,
                                hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = VerifyRegister(Hal::DRV8305_ADDR_VOLTAGE_REG_CTRL,
                                images.voltageReg,
                                DRV8305_VREG_VERIFY_MASK,
                                hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    if(status == Hal::DRV8305_STATUS_OK)
    {
        status = VerifyRegister(Hal::DRV8305_ADDR_VDS_SENSE_CTRL,
                                images.vdsSense,
                                DRV8305_VDS_VERIFY_MASK,
                                hwConfig);
    }
    else
    {
        /* Do nothing. */
    }

    return status;
}

static Hal::Drv8305Status VerifyRegister(
    Hal::Drv8305Address address,
    uint16_t expectedData,
    uint16_t verifyMask,
    const Bsp_Drv8305HwType * hwConfig)
{
    Hal::Drv8305Status status;
    uint16_t actualData;

    actualData = 0U;
    status = ReadRegister(address, actualData, hwConfig);

    if(status == Hal::DRV8305_STATUS_OK)
    {
        if((actualData & verifyMask) !=
           (expectedData & verifyMask))
        {
            status = Hal::DRV8305_STATUS_CONFIG_VERIFY_FAILED;
        }
        else
        {
            /* Register configuration verified. */
        }
    }
    else
    {
        /* Preserve the register-read error. */
    }

    return status;
}

static Hal::Drv8305Status WriteRegister(
    Hal::Drv8305Address address,
    uint16_t data,
    const Bsp_Drv8305HwType * hwConfig)
{
    Hal::Drv8305Status status;
    uint16_t txFrame;
    uint16_t rxFrame;

    txFrame = BuildFrame(false, address, data);
    rxFrame = 0U;

    status = TransferFrame(txFrame, rxFrame, hwConfig);

    return status;
}

static Hal::Drv8305Status ReadRegister(
    Hal::Drv8305Address address,
    uint16_t& data,
    const Bsp_Drv8305HwType * hwConfig)
{
    Hal::Drv8305Status status;
    uint16_t txFrame;
    uint16_t rxFrame;

    txFrame = BuildFrame(true, address, 0U);
    rxFrame = 0U;

    status = TransferFrame(txFrame, rxFrame, hwConfig);

    if(status == Hal::DRV8305_STATUS_OK)
    {
        data = rxFrame & DRV8305_DATA_MASK;
    }
    else
    {
        // Do nothing.
    }

    return status;
}

static uint16_t BuildFrame(
    bool isRead,
    Hal::Drv8305Address address,
    uint16_t data)
{
    uint16_t frame;
    uint16_t command;

    if(isRead == true)
    {
        command = DRV8305_CMD_READ;
    }
    else
    {
        command = DRV8305_CMD_WRITE;
    }

    frame =
        (uint16_t)(
            (command << DRV8305_RW_SHIFT) |
            (((uint16_t)address & DRV8305_ADDR_MASK) <<
             DRV8305_ADDR_SHIFT) |
            (data & DRV8305_DATA_MASK));

    return frame;
}

static Hal::Drv8305Status TransferFrame(
    uint16_t txFrame,
    uint16_t& rxFrame,
    const Bsp_Drv8305HwType * hwConfig)
{
    Hal::Drv8305Status status;
    Mcal_GpioStatusType gpioStatus;
    Mcal_SpiStatusType spiStatus;

    status = Hal::DRV8305_STATUS_HW_ERROR;

    gpioStatus =
        Mcal_Gpio_Write(
            hwConfig->nScsPin,
            MCAL_GPIO_LEVEL_LOW);

    if(gpioStatus == MCAL_GPIO_STATUS_OK)
    {
        spiStatus =
            Mcal_Spi_TransferWord(
                hwConfig->spiModule,
                txFrame,
                &rxFrame);

        gpioStatus =
            Mcal_Gpio_Write(
                hwConfig->nScsPin,
                MCAL_GPIO_LEVEL_HIGH);

        if(gpioStatus == MCAL_GPIO_STATUS_OK)
        {
            Platform_DelayUs(1U);     // According to the drv8305 datasheet, We should wait at least 500ns between 2 high nSCSs.

            if(spiStatus == MCAL_SPI_STATUS_OK)
            {
                status = Hal::DRV8305_STATUS_OK;
            }
            else
            {
                status = Hal::DRV8305_STATUS_SPI_ERROR;
            }
        }
        else
        {
            status = Hal::DRV8305_STATUS_HW_ERROR;
        }
    }
    else
    {
        
        gpioStatus =
            Mcal_Gpio_Write(
                hwConfig->nScsPin,
                MCAL_GPIO_LEVEL_HIGH);

        if(gpioStatus == MCAL_GPIO_STATUS_OK)
        {
            Platform_DelayUs(1U);
        }
        else
        {
            /* Do nothing. */
        }

        status = Hal::DRV8305_STATUS_HW_ERROR;
    }

    return status;
}

}
