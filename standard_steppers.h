/*
 *  © 2022 Peter Cole
 *
 *  These are the standard stepper controller and motor definitions.
 */

#ifndef STANDARD_STEPPERS_h
#define STANDARD_STEPPERS_h

#include <Arduino.h>
#include "AccelStepper.h"
#include "atmegSTEPPER_PIN_028p_pins.h"

#define UNUSED_PIN 127

#define FULLSTEPS 4096

#define ULN2003_HALF_CW AccelStepper(AccelStepper::HALF4WIRE, STEPPER_PIN_0, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3)
#define ULN2003_HALF_CCW AccelStepper(AccelStepper::HALF4WIRE, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_1, STEPPER_PIN_0)
#define ULN2003_FULL_CW AccelStepper(AccelStepper::FULL4WIRE, STEPPER_PIN_0, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3)
#define ULN2003_FULL_CCW AccelStepper(AccelStepper::FULL4WIRE, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_1, STEPPER_PIN_0)
#define A4988 AccelStepper(AccelStepper::DRIVER, STEPPER_PIN_3, STEPPER_PIN_1)

#endif
