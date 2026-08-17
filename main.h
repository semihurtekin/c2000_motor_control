/**
 * @file    main.h
 * @brief   CPU Timer0 interrupt lab declarations.
 */

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief CPU Timer0 interrupt service routine.
 */
__interrupt void Timer0Isr(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */
