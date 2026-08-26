/**
 * @file    mcal_pie.h
 * @brief   C2000 PIE MCAL driver interface.
 */

#ifndef MCAL_PIE_H
#define MCAL_PIE_H

/*==============================================================================
 * Includes
 *============================================================================*/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * Public Macros
 *============================================================================*/

/*==============================================================================
 * Public Types
 *============================================================================*/

typedef enum
{
    MCAL_PIE_GROUP_1 = 1U,
    MCAL_PIE_GROUP_2 = 2U,
    MCAL_PIE_GROUP_3 = 3U,
    MCAL_PIE_GROUP_4 = 4U,
    MCAL_PIE_GROUP_5 = 5U,
    MCAL_PIE_GROUP_6 = 6U,
    MCAL_PIE_GROUP_7 = 7U,
    MCAL_PIE_GROUP_8 = 8U,
    MCAL_PIE_GROUP_9 = 9U,
    MCAL_PIE_GROUP_10 = 10U,
    MCAL_PIE_GROUP_11 = 11U,
    MCAL_PIE_GROUP_12 = 12U
} Mcal_PieGroupType;

typedef uint16_t Mcal_PieChannelType;

typedef enum
{
    MCAL_PIE_STATUS_OK = 0U,
    MCAL_PIE_STATUS_INV_GROUP = 1U,
    MCAL_PIE_STATUS_INV_CHANNEL = 2U
} Mcal_PieStatusType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Initializes the PIE controller to a known state.
 *
 * All PIE interrupt enables and pending flags are cleared, all groups are
 * acknowledged, and the PIE controller is enabled. CPU interrupt enables and
 * the global CPU interrupt mask are not modified.
 *
 * @return Driver status.
 */
Mcal_PieStatusType Mcal_Pie_Init(void);

/**
 * @brief Enables the selected PIE group channel.
 *
 * @param group   PIE interrupt group.
 * @param channel PIE channel in the range 1U to 16U.
 *
 * @return Driver status.
 */
Mcal_PieStatusType Mcal_Pie_Enable(
    Mcal_PieGroupType group,
    Mcal_PieChannelType channel);

/**
 * @brief Disables the selected PIE group channel.
 *
 * @param group   PIE interrupt group.
 * @param channel PIE channel in the range 1U to 16U.
 *
 * @return Driver status.
 */
Mcal_PieStatusType Mcal_Pie_Disable(
    Mcal_PieGroupType group,
    Mcal_PieChannelType channel);

/**
 * @brief Acknowledges the selected PIE interrupt group.
 *
 * @param group PIE interrupt group.
 *
 * @return Driver status.
 */
Mcal_PieStatusType Mcal_Pie_Ack(
    Mcal_PieGroupType group);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_PIE_H */
