/**
 * @file    mcal_gpio.h
 * @brief   F28379D GPIO MCAL driver interface.
 */

#ifndef MCAL_GPIO_H
#define MCAL_GPIO_H

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * Public Types
 *============================================================================*/

/**
 * @brief GPIO pin number type.
 *
 * Valid GPIO pin numbers are in the range GPIO0 through GPIO168.
 */
typedef uint16_t Mcal_GpioPinType;

/**
 * @brief GPIO mux selection code.
 *
 * The low two bits select MUX and the next two bits select GMUX.
 * Valid encoded values are in the range 0 through 15.
 */
typedef uint16_t Mcal_GpioMuxType;

typedef enum
{
    MCAL_GPIO_LEVEL_LOW = 0U,
    MCAL_GPIO_LEVEL_HIGH = 1U
} Mcal_GpioLevelType;

typedef enum
{
    MCAL_GPIO_DIR_INPUT = 0U,
    MCAL_GPIO_DIR_OUTPUT = 1U
} Mcal_GpioDirType;

typedef enum
{
    MCAL_GPIO_PULL_DISABLE = 0U,
    MCAL_GPIO_PULL_ENABLE = 1U
} Mcal_GpioPullType;

/**
 * @brief GPIO native open-drain configuration.
 *
 * @note MCAL_GPIO_ODR_ENABLE configures the device native GPxODR feature.
 *       Device errata shall be reviewed before using native open-drain in
 *       contention-sensitive applications.
 */
typedef enum
{
    MCAL_GPIO_ODR_DISABLE = 0U,
    MCAL_GPIO_ODR_ENABLE = 1U
} Mcal_GpioOdrType;

typedef enum
{
    MCAL_GPIO_INV_DISABLE = 0U,
    MCAL_GPIO_INV_ENABLE = 1U
} Mcal_GpioInvType;

typedef enum
{
    MCAL_GPIO_QUAL_SYNC = 0U,
    MCAL_GPIO_QUAL_3SAMPLE = 1U,
    MCAL_GPIO_QUAL_6SAMPLE = 2U,
    MCAL_GPIO_QUAL_ASYNC = 3U
} Mcal_GpioQualType;

/**
 * @brief GPIO data-register controller selection.
 */
typedef enum
{
    MCAL_GPIO_OWNER_CPU1 = 0U,
    MCAL_GPIO_OWNER_CPU1_CLA1 = 1U,
    MCAL_GPIO_OWNER_CPU2 = 2U,
    MCAL_GPIO_OWNER_CPU2_CLA1 = 3U
} Mcal_GpioOwnerType;

/**
 * @brief Static configuration of one GPIO pin.
 *
 * @note This configuration type is intended for pins used as GPIO.
 *       Peripheral alternate-function selection is configured separately
 *       with Mcal_Gpio_SetMux().
 */
typedef struct
{
    Mcal_GpioPinType pin;
    Mcal_GpioDirType dir;
    Mcal_GpioPullType pull;
    Mcal_GpioOdrType odr;
    Mcal_GpioInvType inv;
    Mcal_GpioQualType qual;
    Mcal_GpioOwnerType owner;
    Mcal_GpioLevelType initLevel;
} Mcal_GpioConfigType;

typedef enum
{
    MCAL_GPIO_STATUS_OK = 0U,
    MCAL_GPIO_STATUS_INV_PIN = 1U,
    MCAL_GPIO_STATUS_INV_ARG = 2U,
    MCAL_GPIO_STATUS_INV_CFG = 3U,
    MCAL_GPIO_STATUS_COMMITTED = 4U,
    MCAL_GPIO_STATUS_LOCKED = 5U
} Mcal_GpioStatusType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes one pin for GPIO operation.
 *
 * @param[in] config Pointer to the static GPIO configuration.
 *
 * @return GPIO service status.
 *
 * @note GPIO control registers are CPU1 controlled on F28379D. This service
 *       is therefore intended to execute on CPU1.
 *
 * @note Output initialization currently supports CPU1 ownership only. A
 *       synchronized ownership handover service will be added separately.
 */
Mcal_GpioStatusType Mcal_Gpio_InitPin(
    const Mcal_GpioConfigType * config);

/**
 * @brief Sets the input qualification sampling divider.
 *
 * The divider is shared by an eight-pin GPIO group. Configuring one pin
 * changes the qualification period of the complete group.
 *
 * @param[in] pin     Any GPIO pin in the affected eight-pin group.
 * @param[in] divider SYSCLKOUT divider. Valid values are 1 or an even value
 *                    from 2 through 510.
 *
 * @return GPIO service status.
 *
 * @note Because the qualification divider is shared by eight pins, this
 *       service rejects the update if any pin in the affected group is
 *       locked or committed by this driver.
 */
Mcal_GpioStatusType Mcal_Gpio_SetQualPeriod(
    Mcal_GpioPinType pin,
    uint16_t divider);

/**
 * @brief Selects the digital alternate function of one GPIO pin.
 *
 * @param[in] pin GPIO pin number.
 * @param[in] mux Four-bit GMUX/MUX selection code from 0 through 15.
 *
 * @return GPIO service status.
 *
 * @note The caller is responsible for using a pin and mux combination
 *       supported by the device pin-mux table.
 */
Mcal_GpioStatusType Mcal_Gpio_SetMux(
    Mcal_GpioPinType pin,
    Mcal_GpioMuxType mux);

/**
 * @brief Locks the configuration of one GPIO pin.
 *
 * @param[in] pin GPIO pin number.
 *
 * @return GPIO service status.
 *
 * @note Locking affects configuration registers only. GPIO data can still be
 *       changed through the data-path services.
 *
 * @note If the lock state was previously committed, this service returns
 *       MCAL_GPIO_STATUS_COMMITTED and does not modify the lock register.
 */
Mcal_GpioStatusType Mcal_Gpio_Lock(
    Mcal_GpioPinType pin);

/**
 * @brief Unlocks the configuration of one GPIO pin.
 *
 * @param[in] pin GPIO pin number.
 *
 * @return GPIO service status.
 *
 * @note If the lock state was previously committed, this service returns
 *       MCAL_GPIO_STATUS_COMMITTED and the pin remains locked.
 */
Mcal_GpioStatusType Mcal_Gpio_Unlock(
    Mcal_GpioPinType pin);

/**
 * @brief Locks and commits the configuration of one GPIO pin.
 *
 * @param[in] pin GPIO pin number.
 *
 * @return GPIO service status.
 *
 * @note This service first locks the pin and then commits the lock state.
 *       After a successful commit, lock and unlock writes have no effect
 *       until the device is reset.
 */
Mcal_GpioStatusType Mcal_Gpio_CommitLock(
    Mcal_GpioPinType pin);

/**
 * @brief Writes a logical level to a GPIO pin.
 *
 * @param[in] pin   GPIO pin number.
 * @param[in] level Logical output level.
 *
 * @return GPIO service status.
 *
 * @note The pin must be configured as a GPIO output and the caller must own
 *       the corresponding GPIO data registers.
 */
Mcal_GpioStatusType Mcal_Gpio_Write(
    Mcal_GpioPinType pin,
    Mcal_GpioLevelType level);

/**
 * @brief Toggles the current output latch state of a GPIO pin.
 *
 * @param[in] pin GPIO pin number.
 *
 * @return GPIO service status.
 *
 * @note The pin must be configured as a GPIO output and the caller must own
 *       the corresponding GPIO data registers.
 */
Mcal_GpioStatusType Mcal_Gpio_Toggle(
    Mcal_GpioPinType pin);

/**
 * @brief Reads the current GPIO data register level.
 *
 * @param[in]  pin      GPIO pin number.
 * @param[out] levelPtr Pointer used to return the logical pin level.
 *
 * @return GPIO service status.
 */
Mcal_GpioStatusType Mcal_Gpio_Read(
    Mcal_GpioPinType pin,
    Mcal_GpioLevelType * levelPtr);

/**
 * @brief Sets qualification mode for the SPI, SCI config.
 *
 * @param[in]  pin      GPIO pin number.
 * @param[out] qual     Qualification type
 *
 * @return GPIO service status.
 */
Mcal_GpioStatusType Mcal_Gpio_SetQualMode(
    Mcal_GpioPinType pin,
    Mcal_GpioQualType qual);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_GPIO_H */
