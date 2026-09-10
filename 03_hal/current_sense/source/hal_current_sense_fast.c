/**
 * @file    hal_current_sense_fast.c
 * @brief   CLA-compatible current-sense fast-path data implementation.
 */

/*==============================================================================
 * Includes
 *============================================================================*/

#include "hal_current_sense_fast.h"

/*==============================================================================
 * Private Macros
 *============================================================================*/

/*==============================================================================
 * Private Function Declarations
 *============================================================================*/

/*==============================================================================
 * Public Function Definitions
 *============================================================================*/

void CurrentSenseFast_Convert(
    uint16_t rawU, 
    uint16_t rawV,
    const CurrentSenseFastConfigType * config,
    PhaseCurrentType * phaseCurrents)
{
    float correctedRawU;
    float correctedRawV;

    correctedRawU = (float)rawU - config->offsetU;
    correctedRawV = (float)rawV - config->offsetV;

    phaseCurrents->phaseU = correctedRawU * config->scaleU;
    phaseCurrents->phaseV = correctedRawV * config->scaleV;
    phaseCurrents->phaseW = -(phaseCurrents->phaseU + phaseCurrents->phaseV);    // Kirchhoff reconstruction
}

/*==============================================================================
 * Private Function Definitions
 *============================================================================*/

