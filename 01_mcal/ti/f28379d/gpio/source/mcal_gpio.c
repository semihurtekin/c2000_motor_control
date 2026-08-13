/**
 * @file    mcal_gpio.c
 * @brief   F28379D GPIO MCAL driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "mcal_gpio.h"

#include <stddef.h>

#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define MCAL_GPIO_PORT_SIZE    (32U)
#define MCAL_GPIO_MUX_SIZE     (16U)
#define MCAL_GPIO_CSEL_SIZE    (8U)
#define MCAL_GPIO_QUAL_SIZE    (8U)
#define MCAL_GPIO_MAX_PIN      ((Mcal_GpioPinType)168U)
#define MCAL_GPIO_MAX_MUX      ((Mcal_GpioMuxType)15U)
#define MCAL_GPIO_MAX_DIV      (510U)
#define MCAL_GPIO_2BIT_MASK    ((uint32_t)0x3U)
#define MCAL_GPIO_4BIT_MASK    ((uint32_t)0xFU)
#define MCAL_GPIO_8BIT_MASK    ((uint32_t)0xFFU)

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static Mcal_GpioStatusType IsPinValid(
    Mcal_GpioPinType pin);

static Mcal_GpioStatusType IsLevelValid(
    Mcal_GpioLevelType level);

static Mcal_GpioStatusType IsDividerValid(
    uint16_t divider);

static Mcal_GpioStatusType CheckPinAccess(
    Mcal_GpioPinType pin);

static Mcal_GpioStatusType CheckQualAccess(
    Mcal_GpioPinType pin);

static Mcal_GpioStatusType ValidateConfig(
    const Mcal_GpioConfigType * config);

static uint16_t GetPortIndex(
    Mcal_GpioPinType pin);

static uint16_t GetBitIndex(
    Mcal_GpioPinType pin);

static uint32_t GetPinMask(
    Mcal_GpioPinType pin);

static volatile uint32_t * GetDirReg(
    uint16_t portIndex);

static volatile uint32_t * GetPudReg(
    uint16_t portIndex);

static volatile uint32_t * GetInvReg(
    uint16_t portIndex);

static volatile uint32_t * GetOdrReg(
    uint16_t portIndex);

static volatile uint32_t * GetCtrlReg(
    uint16_t portIndex);

static volatile uint32_t * GetQselReg(
    uint16_t portIndex,
    uint16_t regIndex);

static volatile uint32_t * GetMuxReg(
    uint16_t portIndex,
    uint16_t regIndex);

static volatile uint32_t * GetGmuxReg(
    uint16_t portIndex,
    uint16_t regIndex);

static volatile uint32_t * GetCselReg(
    uint16_t portIndex,
    uint16_t regIndex);

static volatile uint32_t * GetLockReg(
    uint16_t portIndex);

static volatile uint32_t * GetCommitReg(
    uint16_t portIndex);

static void WriteBit(
    volatile uint32_t * regPtr,
    uint16_t bitIndex,
    uint16_t bitValue);

static void Write2Bit(
    volatile uint32_t * regPtr,
    uint16_t fieldIndex,
    uint16_t fieldValue);

static void Write4Bit(
    volatile uint32_t * regPtr,
    uint16_t fieldIndex,
    uint16_t fieldValue);

static void Write8Bit(
    volatile uint32_t * regPtr,
    uint16_t fieldIndex,
    uint16_t fieldValue);

static Mcal_GpioStatusType SetDirection(
    Mcal_GpioPinType pin,
    Mcal_GpioDirType dir);

static Mcal_GpioStatusType SetPull(
    Mcal_GpioPinType pin,
    Mcal_GpioPullType pull);

static Mcal_GpioStatusType SetOpenDrain(
    Mcal_GpioPinType pin,
    Mcal_GpioOdrType odr);

static Mcal_GpioStatusType SetInvert(
    Mcal_GpioPinType pin,
    Mcal_GpioInvType inv);

static Mcal_GpioStatusType SetQual(
    Mcal_GpioPinType pin,
    Mcal_GpioQualType qual);

static Mcal_GpioStatusType SetOwner(
    Mcal_GpioPinType pin,
    Mcal_GpioOwnerType owner);

static Mcal_GpioStatusType SetPinMux(
    Mcal_GpioPinType pin,
    Mcal_GpioMuxType mux);

static Mcal_GpioStatusType SetQualPeriod(
    Mcal_GpioPinType pin,
    uint16_t divider);

static void WriteSet(
    uint16_t portIndex,
    uint32_t pinMask);

static void WriteClear(
    uint16_t portIndex,
    uint32_t pinMask);

static void WriteToggle(
    uint16_t portIndex,
    uint32_t pinMask);

static uint32_t ReadPort(
    uint16_t portIndex);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Mcal_GpioStatusType Mcal_Gpio_InitPin(
    const Mcal_GpioConfigType * config)
{
    Mcal_GpioStatusType status;
    uint16_t portIndex;
    uint32_t pinMask;

    status = ValidateConfig(config);

    if (status == MCAL_GPIO_STATUS_OK)
    {
        status = CheckPinAccess(config->pin);
    }

    if (status == MCAL_GPIO_STATUS_OK)
    {
        portIndex = GetPortIndex(config->pin);
        pinMask = GetPinMask(config->pin);

        EALLOW;

        status = SetDirection(config->pin, MCAL_GPIO_DIR_INPUT);

        if (status == MCAL_GPIO_STATUS_OK)
        {
            status = SetPinMux(config->pin, (Mcal_GpioMuxType)0U);
        }

        if (status == MCAL_GPIO_STATUS_OK)
        {
            status = SetPull(config->pin, config->pull);
        }

        if (status == MCAL_GPIO_STATUS_OK)
        {
            status = SetOpenDrain(config->pin, config->odr);
        }

        if (status == MCAL_GPIO_STATUS_OK)
        {
            status = SetInvert(config->pin, config->inv);
        }

        if (status == MCAL_GPIO_STATUS_OK)
        {
            status = SetQual(config->pin, config->qual);
        }

        if (status == MCAL_GPIO_STATUS_OK)
        {
            status = SetOwner(config->pin, config->owner);
        }

        EDIS;

        if ((status == MCAL_GPIO_STATUS_OK) &&
            (config->dir == MCAL_GPIO_DIR_OUTPUT))
        {
            if (config->initLevel == MCAL_GPIO_LEVEL_HIGH)
            {
                WriteSet(portIndex, pinMask);
            }
            else
            {
                WriteClear(portIndex, pinMask);
            }

            EALLOW;
            status = SetDirection(config->pin, MCAL_GPIO_DIR_OUTPUT);
            EDIS;
        }
    }

    return status;
}

Mcal_GpioStatusType Mcal_Gpio_SetQualPeriod(
    Mcal_GpioPinType pin,
    uint16_t divider)
{
    Mcal_GpioStatusType status;

    status = IsPinValid(pin);

    if (status == MCAL_GPIO_STATUS_OK)
    {
        status = IsDividerValid(divider);
    }

    if (status == MCAL_GPIO_STATUS_OK)
    {
        status = CheckQualAccess(pin);
    }

    if (status == MCAL_GPIO_STATUS_OK)
    {
        EALLOW;
        status = SetQualPeriod(pin, divider);
        EDIS;
    }

    return status;
}

Mcal_GpioStatusType Mcal_Gpio_SetMux(
    Mcal_GpioPinType pin,
    Mcal_GpioMuxType mux)
{
    Mcal_GpioStatusType status;

    status = IsPinValid(pin);

    if ((status == MCAL_GPIO_STATUS_OK) &&
        (mux > MCAL_GPIO_MAX_MUX))
    {
        status = MCAL_GPIO_STATUS_INV_ARG;
    }

    if (status == MCAL_GPIO_STATUS_OK)
    {
        status = CheckPinAccess(pin);
    }

    if (status == MCAL_GPIO_STATUS_OK)
    {
        EALLOW;
        status = SetPinMux(pin, mux);
        EDIS;
    }

    return status;
}

Mcal_GpioStatusType Mcal_Gpio_Lock(
    Mcal_GpioPinType pin)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * lockReg;
    volatile uint32_t * commitReg;
    uint16_t portIndex;
    uint32_t pinMask;

    status = IsPinValid(pin);

    if (status == MCAL_GPIO_STATUS_OK)
    {
        portIndex = GetPortIndex(pin);
        pinMask = GetPinMask(pin);
        lockReg = GetLockReg(portIndex);
        commitReg = GetCommitReg(portIndex);

        if ((lockReg == NULL) || (commitReg == NULL))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }
        else if ((*commitReg & pinMask) != 0U)
        {
            status = MCAL_GPIO_STATUS_COMMITTED;
        }
        else
        {
            EALLOW;
            *lockReg |= pinMask;
            EDIS;
        }
    }

    return status;
}

Mcal_GpioStatusType Mcal_Gpio_Unlock(
    Mcal_GpioPinType pin)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * lockReg;
    volatile uint32_t * commitReg;
    uint16_t portIndex;
    uint32_t pinMask;

    status = IsPinValid(pin);

    if (status == MCAL_GPIO_STATUS_OK)
    {
        portIndex = GetPortIndex(pin);
        pinMask = GetPinMask(pin);
        lockReg = GetLockReg(portIndex);
        commitReg = GetCommitReg(portIndex);

        if ((lockReg == NULL) || (commitReg == NULL))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }
        else if ((*commitReg & pinMask) != 0U)
        {
            status = MCAL_GPIO_STATUS_COMMITTED;
        }
        else
        {
            EALLOW;
            *lockReg &= ~pinMask;
            EDIS;
        }
    }

    return status;
}

Mcal_GpioStatusType Mcal_Gpio_CommitLock(
    Mcal_GpioPinType pin)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * lockReg;
    volatile uint32_t * commitReg;
    uint16_t portIndex;
    uint32_t pinMask;

    status = IsPinValid(pin);

    if (status == MCAL_GPIO_STATUS_OK)
    {
        portIndex = GetPortIndex(pin);
        pinMask = GetPinMask(pin);
        lockReg = GetLockReg(portIndex);
        commitReg = GetCommitReg(portIndex);

        if ((lockReg == NULL) || (commitReg == NULL))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }
        else if ((*commitReg & pinMask) != 0U)
        {
            status = MCAL_GPIO_STATUS_COMMITTED;
        }
        else
        {
            EALLOW;
            *lockReg |= pinMask;
            *commitReg |= pinMask;
            EDIS;

            if (((*lockReg & pinMask) == 0U) ||
                ((*commitReg & pinMask) == 0U))
            {
                status = MCAL_GPIO_STATUS_INV_CFG;
            }
        }
    }

    return status;
}

Mcal_GpioStatusType Mcal_Gpio_Write(
    Mcal_GpioPinType pin,
    Mcal_GpioLevelType level)
{
    Mcal_GpioStatusType status;
    uint16_t portIndex;
    uint32_t pinMask;

    status = IsPinValid(pin);

    if (status == MCAL_GPIO_STATUS_OK)
    {
        status = IsLevelValid(level);
    }

    if (status == MCAL_GPIO_STATUS_OK)
    {
        portIndex = GetPortIndex(pin);
        pinMask = GetPinMask(pin);

        if (level == MCAL_GPIO_LEVEL_HIGH)
        {
            WriteSet(portIndex, pinMask);
        }
        else
        {
            WriteClear(portIndex, pinMask);
        }
    }

    return status;
}

Mcal_GpioStatusType Mcal_Gpio_Toggle(
    Mcal_GpioPinType pin)
{
    Mcal_GpioStatusType status;
    uint16_t portIndex;
    uint32_t pinMask;

    status = IsPinValid(pin);

    if (status == MCAL_GPIO_STATUS_OK)
    {
        portIndex = GetPortIndex(pin);
        pinMask = GetPinMask(pin);
        WriteToggle(portIndex, pinMask);
    }

    return status;
}

Mcal_GpioStatusType Mcal_Gpio_Read(
    Mcal_GpioPinType pin,
    Mcal_GpioLevelType * levelPtr)
{
    Mcal_GpioStatusType status;
    uint16_t portIndex;
    uint32_t pinMask;
    uint32_t portValue;

    status = IsPinValid(pin);

    if ((status == MCAL_GPIO_STATUS_OK) &&
        (levelPtr == NULL))
    {
        status = MCAL_GPIO_STATUS_INV_ARG;
    }

    if (status == MCAL_GPIO_STATUS_OK)
    {
        portIndex = GetPortIndex(pin);
        pinMask = GetPinMask(pin);
        portValue = ReadPort(portIndex);

        if ((portValue & pinMask) != 0U)
        {
            *levelPtr = MCAL_GPIO_LEVEL_HIGH;
        }
        else
        {
            *levelPtr = MCAL_GPIO_LEVEL_LOW;
        }
    }

    return status;
}

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

static Mcal_GpioStatusType IsPinValid(
    Mcal_GpioPinType pin)
{
    Mcal_GpioStatusType status;

    if (pin <= MCAL_GPIO_MAX_PIN)
    {
        status = MCAL_GPIO_STATUS_OK;
    }
    else
    {
        status = MCAL_GPIO_STATUS_INV_PIN;
    }

    return status;
}

static Mcal_GpioStatusType IsLevelValid(
    Mcal_GpioLevelType level)
{
    Mcal_GpioStatusType status;

    if ((level == MCAL_GPIO_LEVEL_LOW) ||
        (level == MCAL_GPIO_LEVEL_HIGH))
    {
        status = MCAL_GPIO_STATUS_OK;
    }
    else
    {
        status = MCAL_GPIO_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_GpioStatusType IsDividerValid(
    uint16_t divider)
{
    Mcal_GpioStatusType status;

    if ((divider == 1U) ||
        (((divider >= 2U) && (divider <= MCAL_GPIO_MAX_DIV)) &&
         ((divider % 2U) == 0U)))
    {
        status = MCAL_GPIO_STATUS_OK;
    }
    else
    {
        status = MCAL_GPIO_STATUS_INV_ARG;
    }

    return status;
}

static Mcal_GpioStatusType CheckPinAccess(
    Mcal_GpioPinType pin)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * lockReg;
    volatile uint32_t * commitReg;
    uint16_t portIndex;
    uint32_t pinMask;

    status = IsPinValid(pin);

    if (status == MCAL_GPIO_STATUS_OK)
    {
        portIndex = GetPortIndex(pin);
        pinMask = GetPinMask(pin);
        lockReg = GetLockReg(portIndex);
        commitReg = GetCommitReg(portIndex);

        if ((lockReg == NULL) || (commitReg == NULL))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }
        else if ((*commitReg & pinMask) != 0U)
        {
            status = MCAL_GPIO_STATUS_COMMITTED;
        }
        else if ((*lockReg & pinMask) != 0U)
        {
            status = MCAL_GPIO_STATUS_LOCKED;
        }
        else
        {
            /* Configuration access is available. */
        }
    }

    return status;
}

static Mcal_GpioStatusType CheckQualAccess(
    Mcal_GpioPinType pin)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * lockReg;
    volatile uint32_t * commitReg;
    uint16_t portIndex;
    uint16_t bitIndex;
    uint16_t groupBase;
    uint32_t groupMask;

    status = IsPinValid(pin);

    if (status == MCAL_GPIO_STATUS_OK)
    {
        portIndex = GetPortIndex(pin);
        bitIndex = GetBitIndex(pin);
        groupBase = (uint16_t)((bitIndex / MCAL_GPIO_QUAL_SIZE) *
                               MCAL_GPIO_QUAL_SIZE);
        groupMask = ((uint32_t)0xFFU << groupBase);
        lockReg = GetLockReg(portIndex);
        commitReg = GetCommitReg(portIndex);

        if ((lockReg == NULL) || (commitReg == NULL))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }
        else if ((*commitReg & groupMask) != 0U)
        {
            status = MCAL_GPIO_STATUS_COMMITTED;
        }
        else if ((*lockReg & groupMask) != 0U)
        {
            status = MCAL_GPIO_STATUS_LOCKED;
        }
        else
        {
            /* Shared qualification configuration is available. */
        }
    }

    return status;
}

static Mcal_GpioStatusType ValidateConfig(
    const Mcal_GpioConfigType * config)
{
    Mcal_GpioStatusType status;

    status = MCAL_GPIO_STATUS_OK;

    if (config == NULL)
    {
        status = MCAL_GPIO_STATUS_INV_ARG;
    }
    else
    {
        status = IsPinValid(config->pin);

        if ((status == MCAL_GPIO_STATUS_OK) &&
            (config->dir != MCAL_GPIO_DIR_INPUT) &&
            (config->dir != MCAL_GPIO_DIR_OUTPUT))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }

        if ((status == MCAL_GPIO_STATUS_OK) &&
            (config->pull != MCAL_GPIO_PULL_DISABLE) &&
            (config->pull != MCAL_GPIO_PULL_ENABLE))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }

        if ((status == MCAL_GPIO_STATUS_OK) &&
            (config->odr != MCAL_GPIO_ODR_DISABLE) &&
            (config->odr != MCAL_GPIO_ODR_ENABLE))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }

        if ((status == MCAL_GPIO_STATUS_OK) &&
            (config->inv != MCAL_GPIO_INV_DISABLE) &&
            (config->inv != MCAL_GPIO_INV_ENABLE))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }

        if ((status == MCAL_GPIO_STATUS_OK) &&
            (config->qual != MCAL_GPIO_QUAL_SYNC) &&
            (config->qual != MCAL_GPIO_QUAL_3SAMPLE) &&
            (config->qual != MCAL_GPIO_QUAL_6SAMPLE) &&
            (config->qual != MCAL_GPIO_QUAL_ASYNC))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }

        if ((status == MCAL_GPIO_STATUS_OK) &&
            (config->owner != MCAL_GPIO_OWNER_CPU1) &&
            (config->owner != MCAL_GPIO_OWNER_CPU1_CLA1) &&
            (config->owner != MCAL_GPIO_OWNER_CPU2) &&
            (config->owner != MCAL_GPIO_OWNER_CPU2_CLA1))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }

        if (status == MCAL_GPIO_STATUS_OK)
        {
            status = IsLevelValid(config->initLevel);
        }

        if ((status == MCAL_GPIO_STATUS_OK) &&
            (config->dir == MCAL_GPIO_DIR_INPUT) &&
            (config->odr == MCAL_GPIO_ODR_ENABLE))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }

        if ((status == MCAL_GPIO_STATUS_OK) &&
            (config->dir == MCAL_GPIO_DIR_OUTPUT) &&
            (config->owner != MCAL_GPIO_OWNER_CPU1))
        {
            status = MCAL_GPIO_STATUS_INV_CFG;
        }
    }

    return status;
}

static uint16_t GetPortIndex(
    Mcal_GpioPinType pin)
{
    return (uint16_t)(pin / MCAL_GPIO_PORT_SIZE);
}

static uint16_t GetBitIndex(
    Mcal_GpioPinType pin)
{
    return (uint16_t)(pin % MCAL_GPIO_PORT_SIZE);
}

static uint32_t GetPinMask(
    Mcal_GpioPinType pin)
{
    uint16_t bitIndex;
    uint32_t pinMask;

    bitIndex = GetBitIndex(pin);
    pinMask = ((uint32_t)1U << bitIndex);

    return pinMask;
}

static volatile uint32_t * GetDirReg(
    uint16_t portIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U:
            regPtr = &GpioCtrlRegs.GPADIR.all;
            break;
        case 1U:
            regPtr = &GpioCtrlRegs.GPBDIR.all;
            break;
        case 2U:
            regPtr = &GpioCtrlRegs.GPCDIR.all;
            break;
        case 3U:
            regPtr = &GpioCtrlRegs.GPDDIR.all;
            break;
        case 4U:
            regPtr = &GpioCtrlRegs.GPEDIR.all;
            break;
        case 5U:
            regPtr = &GpioCtrlRegs.GPFDIR.all;
            break;
        default:
            break;
    }

    return regPtr;
}

static volatile uint32_t * GetPudReg(
    uint16_t portIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U:
            regPtr = &GpioCtrlRegs.GPAPUD.all;
            break;
        case 1U:
            regPtr = &GpioCtrlRegs.GPBPUD.all;
            break;
        case 2U:
            regPtr = &GpioCtrlRegs.GPCPUD.all;
            break;
        case 3U:
            regPtr = &GpioCtrlRegs.GPDPUD.all;
            break;
        case 4U:
            regPtr = &GpioCtrlRegs.GPEPUD.all;
            break;
        case 5U:
            regPtr = &GpioCtrlRegs.GPFPUD.all;
            break;
        default:
            break;
    }

    return regPtr;
}

static volatile uint32_t * GetInvReg(
    uint16_t portIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U:
            regPtr = &GpioCtrlRegs.GPAINV.all;
            break;
        case 1U:
            regPtr = &GpioCtrlRegs.GPBINV.all;
            break;
        case 2U:
            regPtr = &GpioCtrlRegs.GPCINV.all;
            break;
        case 3U:
            regPtr = &GpioCtrlRegs.GPDINV.all;
            break;
        case 4U:
            regPtr = &GpioCtrlRegs.GPEINV.all;
            break;
        case 5U:
            regPtr = &GpioCtrlRegs.GPFINV.all;
            break;
        default:
            break;
    }

    return regPtr;
}

static volatile uint32_t * GetOdrReg(
    uint16_t portIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U:
            regPtr = &GpioCtrlRegs.GPAODR.all;
            break;
        case 1U:
            regPtr = &GpioCtrlRegs.GPBODR.all;
            break;
        case 2U:
            regPtr = &GpioCtrlRegs.GPCODR.all;
            break;
        case 3U:
            regPtr = &GpioCtrlRegs.GPDODR.all;
            break;
        case 4U:
            regPtr = &GpioCtrlRegs.GPEODR.all;
            break;
        case 5U:
            regPtr = &GpioCtrlRegs.GPFODR.all;
            break;
        default:
            break;
    }

    return regPtr;
}

static volatile uint32_t * GetCtrlReg(
    uint16_t portIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U: regPtr = &GpioCtrlRegs.GPACTRL.all; break;
        case 1U: regPtr = &GpioCtrlRegs.GPBCTRL.all; break;
        case 2U: regPtr = &GpioCtrlRegs.GPCCTRL.all; break;
        case 3U: regPtr = &GpioCtrlRegs.GPDCTRL.all; break;
        case 4U: regPtr = &GpioCtrlRegs.GPECTRL.all; break;
        case 5U: regPtr = &GpioCtrlRegs.GPFCTRL.all; break;
        default: break;
    }

    return regPtr;
}

static volatile uint32_t * GetQselReg(
    uint16_t portIndex,
    uint16_t regIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPAQSEL1.all :
                                       &GpioCtrlRegs.GPAQSEL2.all;
            break;
        case 1U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPBQSEL1.all :
                                       &GpioCtrlRegs.GPBQSEL2.all;
            break;
        case 2U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPCQSEL1.all :
                                       &GpioCtrlRegs.GPCQSEL2.all;
            break;
        case 3U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPDQSEL1.all :
                                       &GpioCtrlRegs.GPDQSEL2.all;
            break;
        case 4U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPEQSEL1.all :
                                       &GpioCtrlRegs.GPEQSEL2.all;
            break;
        case 5U:
            if (regIndex == 0U)
            {
                regPtr = &GpioCtrlRegs.GPFQSEL1.all;
            }
            break;
        default:
            break;
    }

    return regPtr;
}

static volatile uint32_t * GetMuxReg(
    uint16_t portIndex,
    uint16_t regIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPAMUX1.all :
                                       &GpioCtrlRegs.GPAMUX2.all;
            break;
        case 1U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPBMUX1.all :
                                       &GpioCtrlRegs.GPBMUX2.all;
            break;
        case 2U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPCMUX1.all :
                                       &GpioCtrlRegs.GPCMUX2.all;
            break;
        case 3U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPDMUX1.all :
                                       &GpioCtrlRegs.GPDMUX2.all;
            break;
        case 4U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPEMUX1.all :
                                       &GpioCtrlRegs.GPEMUX2.all;
            break;
        case 5U:
            if (regIndex == 0U)
            {
                regPtr = &GpioCtrlRegs.GPFMUX1.all;
            }
            break;
        default:
            break;
    }

    return regPtr;
}

static volatile uint32_t * GetGmuxReg(
    uint16_t portIndex,
    uint16_t regIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPAGMUX1.all :
                                       &GpioCtrlRegs.GPAGMUX2.all;
            break;
        case 1U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPBGMUX1.all :
                                       &GpioCtrlRegs.GPBGMUX2.all;
            break;
        case 2U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPCGMUX1.all :
                                       &GpioCtrlRegs.GPCGMUX2.all;
            break;
        case 3U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPDGMUX1.all :
                                       &GpioCtrlRegs.GPDGMUX2.all;
            break;
        case 4U:
            regPtr = (regIndex == 0U) ? &GpioCtrlRegs.GPEGMUX1.all :
                                       &GpioCtrlRegs.GPEGMUX2.all;
            break;
        case 5U:
            if (regIndex == 0U)
            {
                regPtr = &GpioCtrlRegs.GPFGMUX1.all;
            }
            break;
        default:
            break;
    }

    return regPtr;
}

static volatile uint32_t * GetCselReg(
    uint16_t portIndex,
    uint16_t regIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U:
            switch (regIndex)
            {
                case 0U: regPtr = &GpioCtrlRegs.GPACSEL1.all; break;
                case 1U: regPtr = &GpioCtrlRegs.GPACSEL2.all; break;
                case 2U: regPtr = &GpioCtrlRegs.GPACSEL3.all; break;
                case 3U: regPtr = &GpioCtrlRegs.GPACSEL4.all; break;
                default: break;
            }
            break;
        case 1U:
            switch (regIndex)
            {
                case 0U: regPtr = &GpioCtrlRegs.GPBCSEL1.all; break;
                case 1U: regPtr = &GpioCtrlRegs.GPBCSEL2.all; break;
                case 2U: regPtr = &GpioCtrlRegs.GPBCSEL3.all; break;
                case 3U: regPtr = &GpioCtrlRegs.GPBCSEL4.all; break;
                default: break;
            }
            break;
        case 2U:
            switch (regIndex)
            {
                case 0U: regPtr = &GpioCtrlRegs.GPCCSEL1.all; break;
                case 1U: regPtr = &GpioCtrlRegs.GPCCSEL2.all; break;
                case 2U: regPtr = &GpioCtrlRegs.GPCCSEL3.all; break;
                case 3U: regPtr = &GpioCtrlRegs.GPCCSEL4.all; break;
                default: break;
            }
            break;
        case 3U:
            switch (regIndex)
            {
                case 0U: regPtr = &GpioCtrlRegs.GPDCSEL1.all; break;
                case 1U: regPtr = &GpioCtrlRegs.GPDCSEL2.all; break;
                case 2U: regPtr = &GpioCtrlRegs.GPDCSEL3.all; break;
                case 3U: regPtr = &GpioCtrlRegs.GPDCSEL4.all; break;
                default: break;
            }
            break;
        case 4U:
            switch (regIndex)
            {
                case 0U: regPtr = &GpioCtrlRegs.GPECSEL1.all; break;
                case 1U: regPtr = &GpioCtrlRegs.GPECSEL2.all; break;
                case 2U: regPtr = &GpioCtrlRegs.GPECSEL3.all; break;
                case 3U: regPtr = &GpioCtrlRegs.GPECSEL4.all; break;
                default: break;
            }
            break;
        case 5U:
            switch (regIndex)
            {
                case 0U: regPtr = &GpioCtrlRegs.GPFCSEL1.all; break;
                case 1U: regPtr = &GpioCtrlRegs.GPFCSEL2.all; break;
                default: break;
            }
            break;
        default:
            break;
    }

    return regPtr;
}

static volatile uint32_t * GetLockReg(
    uint16_t portIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U:
            regPtr = &GpioCtrlRegs.GPALOCK.all;
            break;
        case 1U:
            regPtr = &GpioCtrlRegs.GPBLOCK.all;
            break;
        case 2U:
            regPtr = &GpioCtrlRegs.GPCLOCK.all;
            break;
        case 3U:
            regPtr = &GpioCtrlRegs.GPDLOCK.all;
            break;
        case 4U:
            regPtr = &GpioCtrlRegs.GPELOCK.all;
            break;
        case 5U:
            regPtr = &GpioCtrlRegs.GPFLOCK.all;
            break;
        default:
            break;
    }

    return regPtr;
}

static volatile uint32_t * GetCommitReg(
    uint16_t portIndex)
{
    volatile uint32_t * regPtr;

    regPtr = NULL;

    switch (portIndex)
    {
        case 0U:
            regPtr = &GpioCtrlRegs.GPACR.all;
            break;
        case 1U:
            regPtr = &GpioCtrlRegs.GPBCR.all;
            break;
        case 2U:
            regPtr = &GpioCtrlRegs.GPCCR.all;
            break;
        case 3U:
            regPtr = &GpioCtrlRegs.GPDCR.all;
            break;
        case 4U:
            regPtr = &GpioCtrlRegs.GPECR.all;
            break;
        case 5U:
            regPtr = &GpioCtrlRegs.GPFCR.all;
            break;
        default:
            break;
    }

    return regPtr;
}

static void WriteBit(
    volatile uint32_t * regPtr,
    uint16_t bitIndex,
    uint16_t bitValue)
{
    uint32_t regValue;
    uint32_t bitMask;

    regValue = *regPtr;
    bitMask = ((uint32_t)1U << bitIndex);

    if (bitValue != 0U)
    {
        regValue |= bitMask;
    }
    else
    {
        regValue &= ~bitMask;
    }

    *regPtr = regValue;
}

static void Write2Bit(
    volatile uint32_t * regPtr,
    uint16_t fieldIndex,
    uint16_t fieldValue)
{
    uint16_t shift;
    uint32_t regValue;
    uint32_t fieldMask;
    uint32_t newValue;

    shift = (uint16_t)(fieldIndex * 2U);
    fieldMask = (MCAL_GPIO_2BIT_MASK << shift);
    newValue = ((uint32_t)fieldValue << shift);

    regValue = *regPtr;
    regValue &= ~fieldMask;
    regValue |= (newValue & fieldMask);
    *regPtr = regValue;
}

static void Write4Bit(
    volatile uint32_t * regPtr,
    uint16_t fieldIndex,
    uint16_t fieldValue)
{
    uint16_t shift;
    uint32_t regValue;
    uint32_t fieldMask;
    uint32_t newValue;

    shift = (uint16_t)(fieldIndex * 4U);
    fieldMask = (MCAL_GPIO_4BIT_MASK << shift);
    newValue = ((uint32_t)fieldValue << shift);

    regValue = *regPtr;
    regValue &= ~fieldMask;
    regValue |= (newValue & fieldMask);
    *regPtr = regValue;
}

static void Write8Bit(
    volatile uint32_t * regPtr,
    uint16_t fieldIndex,
    uint16_t fieldValue)
{
    uint16_t shift;
    uint32_t regValue;
    uint32_t fieldMask;
    uint32_t newValue;

    shift = (uint16_t)(fieldIndex * 8U);
    fieldMask = (MCAL_GPIO_8BIT_MASK << shift);
    newValue = ((uint32_t)fieldValue << shift);

    regValue = *regPtr;
    regValue &= ~fieldMask;
    regValue |= (newValue & fieldMask);
    *regPtr = regValue;
}

static Mcal_GpioStatusType SetDirection(
    Mcal_GpioPinType pin,
    Mcal_GpioDirType dir)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * regPtr;
    uint16_t portIndex;
    uint16_t bitIndex;
    uint16_t bitValue;

    portIndex = GetPortIndex(pin);
    bitIndex = GetBitIndex(pin);
    regPtr = GetDirReg(portIndex);

    if (regPtr == NULL)
    {
        status = MCAL_GPIO_STATUS_INV_CFG;
    }
    else
    {
        bitValue = (dir == MCAL_GPIO_DIR_OUTPUT) ? 1U : 0U;
        WriteBit(regPtr, bitIndex, bitValue);
        status = MCAL_GPIO_STATUS_OK;
    }

    return status;
}

static Mcal_GpioStatusType SetPull(
    Mcal_GpioPinType pin,
    Mcal_GpioPullType pull)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * regPtr;
    uint16_t portIndex;
    uint16_t bitIndex;
    uint16_t bitValue;

    portIndex = GetPortIndex(pin);
    bitIndex = GetBitIndex(pin);
    regPtr = GetPudReg(portIndex);

    if (regPtr == NULL)
    {
        status = MCAL_GPIO_STATUS_INV_CFG;
    }
    else
    {
        bitValue = (pull == MCAL_GPIO_PULL_ENABLE) ? 0U : 1U;
        WriteBit(regPtr, bitIndex, bitValue);
        status = MCAL_GPIO_STATUS_OK;
    }

    return status;
}

static Mcal_GpioStatusType SetOpenDrain(
    Mcal_GpioPinType pin,
    Mcal_GpioOdrType odr)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * regPtr;
    uint16_t portIndex;
    uint16_t bitIndex;
    uint16_t bitValue;

    portIndex = GetPortIndex(pin);
    bitIndex = GetBitIndex(pin);
    regPtr = GetOdrReg(portIndex);

    if (regPtr == NULL)
    {
        status = MCAL_GPIO_STATUS_INV_CFG;
    }
    else
    {
        bitValue = (odr == MCAL_GPIO_ODR_ENABLE) ? 1U : 0U;
        WriteBit(regPtr, bitIndex, bitValue);
        status = MCAL_GPIO_STATUS_OK;
    }

    return status;
}

static Mcal_GpioStatusType SetInvert(
    Mcal_GpioPinType pin,
    Mcal_GpioInvType inv)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * regPtr;
    uint16_t portIndex;
    uint16_t bitIndex;
    uint16_t bitValue;

    portIndex = GetPortIndex(pin);
    bitIndex = GetBitIndex(pin);
    regPtr = GetInvReg(portIndex);

    if (regPtr == NULL)
    {
        status = MCAL_GPIO_STATUS_INV_CFG;
    }
    else
    {
        bitValue = (inv == MCAL_GPIO_INV_ENABLE) ? 1U : 0U;
        WriteBit(regPtr, bitIndex, bitValue);
        status = MCAL_GPIO_STATUS_OK;
    }

    return status;
}

static Mcal_GpioStatusType SetQual(
    Mcal_GpioPinType pin,
    Mcal_GpioQualType qual)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * regPtr;
    uint16_t portIndex;
    uint16_t bitIndex;
    uint16_t regIndex;
    uint16_t fieldIndex;

    portIndex = GetPortIndex(pin);
    bitIndex = GetBitIndex(pin);
    regIndex = (uint16_t)(bitIndex / MCAL_GPIO_MUX_SIZE);
    fieldIndex = (uint16_t)(bitIndex % MCAL_GPIO_MUX_SIZE);
    regPtr = GetQselReg(portIndex, regIndex);

    if (regPtr == NULL)
    {
        status = MCAL_GPIO_STATUS_INV_CFG;
    }
    else
    {
        Write2Bit(regPtr, fieldIndex, (uint16_t)qual);
        status = MCAL_GPIO_STATUS_OK;
    }

    return status;
}

static Mcal_GpioStatusType SetOwner(
    Mcal_GpioPinType pin,
    Mcal_GpioOwnerType owner)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * regPtr;
    uint16_t portIndex;
    uint16_t bitIndex;
    uint16_t regIndex;
    uint16_t fieldIndex;

    portIndex = GetPortIndex(pin);
    bitIndex = GetBitIndex(pin);
    regIndex = (uint16_t)(bitIndex / MCAL_GPIO_CSEL_SIZE);
    fieldIndex = (uint16_t)(bitIndex % MCAL_GPIO_CSEL_SIZE);
    regPtr = GetCselReg(portIndex, regIndex);

    if (regPtr == NULL)
    {
        status = MCAL_GPIO_STATUS_INV_CFG;
    }
    else
    {
        Write4Bit(regPtr, fieldIndex, (uint16_t)owner);
        status = MCAL_GPIO_STATUS_OK;
    }

    return status;
}

static Mcal_GpioStatusType SetPinMux(
    Mcal_GpioPinType pin,
    Mcal_GpioMuxType mux)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * muxPtr;
    volatile uint32_t * gmuxPtr;
    uint16_t portIndex;
    uint16_t bitIndex;
    uint16_t regIndex;
    uint16_t fieldIndex;
    uint16_t muxValue;
    uint16_t gmuxValue;

    portIndex = GetPortIndex(pin);
    bitIndex = GetBitIndex(pin);
    regIndex = (uint16_t)(bitIndex / MCAL_GPIO_MUX_SIZE);
    fieldIndex = (uint16_t)(bitIndex % MCAL_GPIO_MUX_SIZE);
    muxPtr = GetMuxReg(portIndex, regIndex);
    gmuxPtr = GetGmuxReg(portIndex, regIndex);

    if ((muxPtr == NULL) || (gmuxPtr == NULL))
    {
        status = MCAL_GPIO_STATUS_INV_CFG;
    }
    else
    {
        muxValue = (uint16_t)(mux & 0x3U);
        gmuxValue = (uint16_t)((mux >> 2U) & 0x3U);

        /* Force MUX to GPIO-safe value before changing GMUX. */
        Write2Bit(muxPtr, fieldIndex, 0U);
        Write2Bit(gmuxPtr, fieldIndex, gmuxValue);
        Write2Bit(muxPtr, fieldIndex, muxValue);
        status = MCAL_GPIO_STATUS_OK;
    }

    return status;
}

static Mcal_GpioStatusType SetQualPeriod(
    Mcal_GpioPinType pin,
    uint16_t divider)
{
    Mcal_GpioStatusType status;
    volatile uint32_t * regPtr;
    uint16_t portIndex;
    uint16_t bitIndex;
    uint16_t fieldIndex;
    uint16_t fieldValue;

    portIndex = GetPortIndex(pin);
    bitIndex = GetBitIndex(pin);
    fieldIndex = (uint16_t)(bitIndex / MCAL_GPIO_QUAL_SIZE);
    regPtr = GetCtrlReg(portIndex);

    if (regPtr == NULL)
    {
        status = MCAL_GPIO_STATUS_INV_CFG;
    }
    else
    {
        fieldValue = (uint16_t)(divider / 2U);
        Write8Bit(regPtr, fieldIndex, fieldValue);
        status = MCAL_GPIO_STATUS_OK;
    }

    return status;
}

static void WriteSet(
    uint16_t portIndex,
    uint32_t pinMask)
{
    switch (portIndex)
    {
        case 0U:
            GpioDataRegs.GPASET.all = pinMask;
            break;
        case 1U:
            GpioDataRegs.GPBSET.all = pinMask;
            break;
        case 2U:
            GpioDataRegs.GPCSET.all = pinMask;
            break;
        case 3U:
            GpioDataRegs.GPDSET.all = pinMask;
            break;
        case 4U:
            GpioDataRegs.GPESET.all = pinMask;
            break;
        case 5U:
            GpioDataRegs.GPFSET.all = pinMask;
            break;
        default:
            break;
    }
}

static void WriteClear(
    uint16_t portIndex,
    uint32_t pinMask)
{
    switch (portIndex)
    {
        case 0U:
            GpioDataRegs.GPACLEAR.all = pinMask;
            break;
        case 1U:
            GpioDataRegs.GPBCLEAR.all = pinMask;
            break;
        case 2U:
            GpioDataRegs.GPCCLEAR.all = pinMask;
            break;
        case 3U:
            GpioDataRegs.GPDCLEAR.all = pinMask;
            break;
        case 4U:
            GpioDataRegs.GPECLEAR.all = pinMask;
            break;
        case 5U:
            GpioDataRegs.GPFCLEAR.all = pinMask;
            break;
        default:
            break;
    }
}

static void WriteToggle(
    uint16_t portIndex,
    uint32_t pinMask)
{
    switch (portIndex)
    {
        case 0U:
            GpioDataRegs.GPATOGGLE.all = pinMask;
            break;
        case 1U:
            GpioDataRegs.GPBTOGGLE.all = pinMask;
            break;
        case 2U:
            GpioDataRegs.GPCTOGGLE.all = pinMask;
            break;
        case 3U:
            GpioDataRegs.GPDTOGGLE.all = pinMask;
            break;
        case 4U:
            GpioDataRegs.GPETOGGLE.all = pinMask;
            break;
        case 5U:
            GpioDataRegs.GPFTOGGLE.all = pinMask;
            break;
        default:
            break;
    }
}

static uint32_t ReadPort(
    uint16_t portIndex)
{
    uint32_t portValue;

    portValue = 0U;

    switch (portIndex)
    {
        case 0U:
            portValue = GpioDataRegs.GPADAT.all;
            break;
        case 1U:
            portValue = GpioDataRegs.GPBDAT.all;
            break;
        case 2U:
            portValue = GpioDataRegs.GPCDAT.all;
            break;
        case 3U:
            portValue = GpioDataRegs.GPDDAT.all;
            break;
        case 4U:
            portValue = GpioDataRegs.GPEDAT.all;
            break;
        case 5U:
            portValue = GpioDataRegs.GPFDAT.all;
            break;
        default:
            break;
    }

    return portValue;
}
