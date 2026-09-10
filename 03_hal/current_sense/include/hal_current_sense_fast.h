/**
 * @file    hal_current_sense_fast.h
 * @brief   CLA-compatible current-sense fast-path data interface.
 */

#ifndef HAL_CURRENT_SENSE_FAST_H
#define HAL_CURRENT_SENSE_FAST_H

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
 * @brief Precomputed current-sense parameters used by the fast control path.
 *
 * @details
 * The CPU-side CurrentSense component owns calibration and scaling state.
 * A validated snapshot of that state is published through this structure for
 * use by CLA-compatible fast-path code.
 */
typedef struct
{
    float offsetU;
    float offsetV;
    float scaleU;
    float scaleV;
} CurrentSenseFastConfigType;

/**
 * @brief Calculated U, V and W phase current outputs.
 */
typedef struct
{
    float phaseU;
    float phaseV;
    float phaseW;
} PhaseCurrentType;

/*==============================================================================
 * Public Function Declarations
 *============================================================================*/

/**
 * @brief Converts raw ADC samples to three-phase currents in amperes.
 *
 * @param rawU Raw ADC sample for phase U.
 * @param rawV Raw ADC sample for phase V.
 * @param config Calibrated current-sense conversion parameters.
 * @param phaseCurrents Receives the calculated phase currents.
 */
void CurrentSenseFast_Convert(
    uint16_t rawU, 
    uint16_t rawV,
    const CurrentSenseFastConfigType * config,
    PhaseCurrentType * phaseCurrents);

#ifdef __cplusplus
}
#endif

#endif /* HAL_CURRENT_SENSE_FAST_H */
