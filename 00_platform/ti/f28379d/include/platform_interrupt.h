/**
 * @file    platform_interrupt.h
 * @brief   F28379D interrupt vector integration interface.
 */

#ifndef PLATFORM_INTERRUPT_H
#define PLATFORM_INTERRUPT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef __interrupt void (*Platform_IsrType)(void);

typedef enum
{
    PLATFORM_INT_STATUS_OK = 0U,
    PLATFORM_INT_STATUS_INV_ARG = 1U
} Platform_IntStatusType;

Platform_IntStatusType Platform_IntSetTimer0(
    Platform_IsrType handler);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_INTERRUPT_H */
