/**
 * @file    hal_drv8305.hpp
 * @brief   DRV8305 gate driver hardware abstraction interface.
 */

#ifndef HAL_DRV8305_HPP
#define HAL_DRV8305_HPP

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

#include "bsp_drv8305_hw.h"

/*==============================================================================
 * Namespace
 *============================================================================*/

namespace Hal
{

/*==============================================================================
 * Public Types
 *============================================================================*/

enum Drv8305State
{
    DRV8305_STATE_UNINITIALIZED = 0U,
    DRV8305_STATE_CONFIGURED
};

enum Drv8305Status
{
    DRV8305_STATUS_OK = 0U,
    DRV8305_STATUS_ALREADY_INITIALIZED,
    DRV8305_STATUS_NOT_INITIALIZED,
    DRV8305_STATUS_INVALID_ARG,
    DRV8305_STATUS_SPI_ERROR,
    DRV8305_STATUS_HW_ERROR,
    DRV8305_STATUS_CONFIG_VERIFY_FAILED,
    DRV8305_STATUS_FAULT_ACTIVE
};

enum Drv8305Address
{
    DRV8305_ADDR_WARNINGS_WD_RESET = 0x1U,
    DRV8305_ADDR_OV_VDS_FAULTS = 0x2U,
    DRV8305_ADDR_IC_FAULTS = 0x3U,
    DRV8305_ADDR_VGS_FAULTS = 0x4U,
    DRV8305_ADDR_HS_GD_CTRL = 0x5U,
    DRV8305_ADDR_LS_GD_CTRL = 0x6U,
    DRV8305_ADDR_GD_CTRL = 0x7U,
    DRV8305_ADDR_RSRVD = 0x8U,
    DRV8305_ADDR_IC_OPERATION = 0x9U,
    DRV8305_ADDR_SHUNT_AMP_CTRL = 0xAU,
    DRV8305_ADDR_VOLTAGE_REG_CTRL = 0xBU,
    DRV8305_ADDR_VDS_SENSE_CTRL = 0xCU
};

enum Drv8305GateSourceCurrent
{
    DRV8305_GATE_SOURCE_10MA = 0x0U,
    DRV8305_GATE_SOURCE_20MA = 0x1U,
    DRV8305_GATE_SOURCE_30MA = 0x2U,
    DRV8305_GATE_SOURCE_40MA = 0x3U,
    DRV8305_GATE_SOURCE_50MA = 0x4U,
    DRV8305_GATE_SOURCE_60MA = 0x5U,
    DRV8305_GATE_SOURCE_70MA = 0x6U,
    DRV8305_GATE_SOURCE_125MA = 0x7U,
    DRV8305_GATE_SOURCE_250MA = 0x8U,
    DRV8305_GATE_SOURCE_500MA = 0x9U,
    DRV8305_GATE_SOURCE_750MA = 0xAU,
    DRV8305_GATE_SOURCE_1000MA = 0xBU
};

enum Drv8305GateSinkCurrent
{
    DRV8305_GATE_SINK_20MA = 0x0U,
    DRV8305_GATE_SINK_30MA = 0x1U,
    DRV8305_GATE_SINK_40MA = 0x2U,
    DRV8305_GATE_SINK_50MA = 0x3U,
    DRV8305_GATE_SINK_60MA = 0x4U,
    DRV8305_GATE_SINK_70MA = 0x5U,
    DRV8305_GATE_SINK_80MA = 0x6U,
    DRV8305_GATE_SINK_250MA = 0x7U,
    DRV8305_GATE_SINK_500MA = 0x8U,
    DRV8305_GATE_SINK_750MA = 0x9U,
    DRV8305_GATE_SINK_1000MA = 0xAU,
    DRV8305_GATE_SINK_1250MA = 0xBU
};

enum Drv8305DriveTime
{
    DRV8305_DRIVE_TIME_220NS = 0x0U,
    DRV8305_DRIVE_TIME_440NS = 0x1U,
    DRV8305_DRIVE_TIME_880NS = 0x2U,
    DRV8305_DRIVE_TIME_1780NS = 0x3U
};

enum Drv8305DeadTime
{
    DRV8305_DEAD_TIME_35NS = 0x0U,
    DRV8305_DEAD_TIME_52NS = 0x1U,
    DRV8305_DEAD_TIME_88NS = 0x2U,
    DRV8305_DEAD_TIME_440NS = 0x3U,
    DRV8305_DEAD_TIME_880NS = 0x4U,
    DRV8305_DEAD_TIME_1760NS = 0x5U,
    DRV8305_DEAD_TIME_3520NS = 0x6U,
    DRV8305_DEAD_TIME_5280NS = 0x7U
};

enum Drv8305CsaGain
{
    DRV8305_CSA_GAIN_10 = 0x0U,
    DRV8305_CSA_GAIN_20 = 0x1U,
    DRV8305_CSA_GAIN_40 = 0x2U,
    DRV8305_CSA_GAIN_80 = 0x3U
};

enum Drv8305CsaBlanking
{
    DRV8305_CSA_BLANK_0NS = 0x0U,
    DRV8305_CSA_BLANK_500NS = 0x1U,
    DRV8305_CSA_BLANK_2500NS = 0x2U,
    DRV8305_CSA_BLANK_10000NS = 0x3U
};

enum Drv8305VrefScale
{
    DRV8305_VREF_SCALE_DIV_2 = 0x1U,
    DRV8305_VREF_SCALE_DIV_4 = 0x2U
};

enum Drv8305VdsBlanking
{
    DRV8305_VDS_BLANK_0NS = 0x0U,
    DRV8305_VDS_BLANK_1750NS = 0x1U,
    DRV8305_VDS_BLANK_3500NS = 0x2U,
    DRV8305_VDS_BLANK_7000NS = 0x3U
};

enum Drv8305VdsDeglitch
{
    DRV8305_VDS_DEGLITCH_0NS = 0x0U,
    DRV8305_VDS_DEGLITCH_1750NS = 0x1U,
    DRV8305_VDS_DEGLITCH_3500NS = 0x2U,
    DRV8305_VDS_DEGLITCH_7000NS = 0x3U
};

enum Drv8305VdsThreshold
{
    DRV8305_VDS_THRESHOLD_60MV = 0x00U,
    DRV8305_VDS_THRESHOLD_68MV = 0x01U,
    DRV8305_VDS_THRESHOLD_76MV = 0x02U,
    DRV8305_VDS_THRESHOLD_86MV = 0x03U,
    DRV8305_VDS_THRESHOLD_97MV = 0x04U,
    DRV8305_VDS_THRESHOLD_109MV = 0x05U,
    DRV8305_VDS_THRESHOLD_123MV = 0x06U,
    DRV8305_VDS_THRESHOLD_138MV = 0x07U,
    DRV8305_VDS_THRESHOLD_155MV = 0x08U,
    DRV8305_VDS_THRESHOLD_175MV = 0x09U,
    DRV8305_VDS_THRESHOLD_197MV = 0x0AU,
    DRV8305_VDS_THRESHOLD_222MV = 0x0BU,
    DRV8305_VDS_THRESHOLD_250MV = 0x0CU,
    DRV8305_VDS_THRESHOLD_282MV = 0x0DU,
    DRV8305_VDS_THRESHOLD_317MV = 0x0EU,
    DRV8305_VDS_THRESHOLD_358MV = 0x0FU,
    DRV8305_VDS_THRESHOLD_403MV = 0x10U,
    DRV8305_VDS_THRESHOLD_454MV = 0x11U,
    DRV8305_VDS_THRESHOLD_511MV = 0x12U,
    DRV8305_VDS_THRESHOLD_576MV = 0x13U,
    DRV8305_VDS_THRESHOLD_648MV = 0x14U,
    DRV8305_VDS_THRESHOLD_730MV = 0x15U,
    DRV8305_VDS_THRESHOLD_822MV = 0x16U,
    DRV8305_VDS_THRESHOLD_926MV = 0x17U,
    DRV8305_VDS_THRESHOLD_1043MV = 0x18U,
    DRV8305_VDS_THRESHOLD_1175MV = 0x19U,
    DRV8305_VDS_THRESHOLD_1324MV = 0x1AU,
    DRV8305_VDS_THRESHOLD_1491MV = 0x1BU,
    DRV8305_VDS_THRESHOLD_1679MV = 0x1CU,
    DRV8305_VDS_THRESHOLD_1892MV = 0x1DU,
    DRV8305_VDS_THRESHOLD_2131MV = 0x1EU
};

enum Drv8305VdsMode
{
    DRV8305_VDS_MODE_LATCHED_SHUTDOWN = 0x0U,
    DRV8305_VDS_MODE_REPORT_ONLY = 0x1U,
    DRV8305_VDS_MODE_DISABLED = 0x2U
};

struct Drv8305GateDriveConfig
{
    Drv8305GateSourceCurrent hsSourceCurrent;
    Drv8305GateSinkCurrent hsSinkCurrent;
    Drv8305DriveTime hsDriveTime;

    Drv8305GateSourceCurrent lsSourceCurrent;
    Drv8305GateSinkCurrent lsSinkCurrent;
    Drv8305DriveTime lsDriveTime;

    Drv8305DeadTime deadTime;
};

struct Drv8305CurrentSenseConfig
{
    Drv8305CsaGain gain;
    Drv8305CsaBlanking blanking;
    Drv8305VrefScale vrefScale;
    bool outputClampEnabled;
};

struct Drv8305ProtectionConfig
{
    Drv8305VdsThreshold vdsThreshold;
    Drv8305VdsMode vdsMode;
    Drv8305VdsBlanking vdsBlanking;
    Drv8305VdsDeglitch vdsDeglitch;
};

struct Drv8305Config
{
    Drv8305GateDriveConfig gateDrive;
    Drv8305CurrentSenseConfig currentSense;
    Drv8305ProtectionConfig protection;
};


/*==============================================================================
 * Public Classes
 *============================================================================*/

class Drv8305
{
public:

    Drv8305();

    Drv8305Status Init(
        const Drv8305Config& config);

    Drv8305Status Enable(void);

    Drv8305Status Disable(void);


private:
 
    Drv8305(const Drv8305&);
    Drv8305& operator=(const Drv8305&);

    
    Drv8305State state_;
    const Bsp_Drv8305HwType * hwConfig_;
    Drv8305Config config_;
};

} /* namespace Hal */

#endif /* HAL_DRV8305_HPP */
