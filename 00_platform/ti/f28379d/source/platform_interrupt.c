/**
 * @file    platform_interrupt.c
 * @brief   F28379D interrupt vector integration implementation.
 */

#include <stddef.h>

#include "platform_interrupt.h"
#include "F2837xD_device.h"

Platform_IntStatusType Platform_IntSetTimer0(
    Platform_IsrType handler)
{
    Platform_IntStatusType status;

    if(handler != NULL)
    {
        EALLOW;

        PieVectTable.TIMER0_INT = handler;

        EDIS;

        status = PLATFORM_INT_STATUS_OK;
    }
    else
    {
        status = PLATFORM_INT_STATUS_INV_ARG;
    }

    return status;
}
