/**
 * @file    platform_clock.c
 * @brief   F28379D platform clock driver implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "platform_clock.h"
#include "F2837xD_device.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

#define PLATFORM_CLOCK_XTAL_SRC            (1U)
#define PLATFORM_CLOCK_IMULT               (40U)
#define PLATFORM_CLOCK_FMULT               (0U)

#define PLATFORM_CLOCK_DIV_SAFE            (2U)
#define PLATFORM_CLOCK_DIV_FINAL           (1U)

#define PLATFORM_CLOCK_LSPCLK_DIV_4        (2U)

#define PLATFORM_CLOCK_EPWMCLK_HZ          (100000000UL)

/*==============================================================================
 * Private Types
 *============================================================================*/


/*==============================================================================
 * Private Variables
 *============================================================================*/


/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

static void ClockSourceDelay(void);

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

Platform_ClockStatusType Platform_ClockInit(void)
{
    Platform_ClockStatusType status;

    status = PLATFORM_CLOCK_STATUS_PLL_FAIL;

    EALLOW;

    /* Enable and select external crystal oscillator. */
    ClkCfgRegs.CLKSRCCTL1.bit.XTALOFF = 0U;

    ClockSourceDelay();

    ClkCfgRegs.CLKSRCCTL1.bit.OSCCLKSRCSEL =
        PLATFORM_CLOCK_XTAL_SRC;

    /* Bypass PLL. */
    ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 0U;

    asm(" RPT #120 || NOP");

    ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = 0U;

    /* Power down and reconfigure PLL. */
    ClkCfgRegs.SYSPLLCTL1.bit.PLLEN = 0U;

    asm(" RPT #20 || NOP");

    ClkCfgRegs.SYSPLLMULT.all =
        ((PLATFORM_CLOCK_FMULT << 8U) |
         PLATFORM_CLOCK_IMULT);

    while(ClkCfgRegs.SYSPLLSTS.bit.LOCKS == 0U)
    {
        /* Wait for PLL lock. */
    }

    /* Initially use a slower system clock. */
    ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV =
        PLATFORM_CLOCK_DIV_SAFE;

    ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 1U;

    asm(" RPT #20 || NOP");

    /* Allow the regulator to stabilize. */
    asm(" RPT #200 || NOP");

    /* Switch to final 200 MHz configuration. */
    ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV =
        PLATFORM_CLOCK_DIV_FINAL;

    /* Divide PLLSYSCLK by 2 for the 100 MHz ePWM clock. */
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1U;

    /* Divide 200 MHz SYSCLK by 4 for the 50 MHz LSPCLK. */
    ClkCfgRegs.LOSPCP.bit.LSPCLKDIV =
        PLATFORM_CLOCK_LSPCLK_DIV_4;

    status = PLATFORM_CLOCK_STATUS_OK;

    EDIS;

    return status;
}

uint32_t Platform_ClockGetEpwmClkHz(void)
{
    return PLATFORM_CLOCK_EPWMCLK_HZ;
}

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

static void ClockSourceDelay(void)
{
    asm(" RPT #250 || NOP");
    asm(" RPT #50 || NOP");
}
