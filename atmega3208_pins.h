#pragma once
#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H
const uint8_t limitSensorPin = PIN_PA5; // Define pin 2 for the traverser mode limit sensor.
const uint8_t homeSensorPin = PIN_PA6;  // Define pin 5 for the home sensor.
const uint8_t relay1Pin = PIN_PD7;      // Control pin for relay 1.
const uint8_t relay2Pin = PIN_PD6;      // Control pin for relay 2.
const uint8_t ledPin = PIN_PD5;         // Pin for LED output.
const uint8_t accPin = PIN_PD4;         // Pin for accessory output.

const uint8_t STEPPER_PIN_0 = PIN_PA7;
const uint8_t STEPPER_PIN_1 = PIN_PC0;
const uint8_t STEPPER_PIN_2 = PIN_PC1;
const uint8_t STEPPER_PIN_3 = PIN_PC2;
#endif
