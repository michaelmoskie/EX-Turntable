#pragma once
#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H
const uint8_t limitSensorPin = 2; // Define pin 2 for the traverser mode limit sensor.
const uint8_t homeSensorPin = 5;  // Define pin 5 for the home sensor.
const uint8_t relay1Pin = 3;      // Control pin for relay 1.
const uint8_t relay2Pin = 4;      // Control pin for relay 2.
const uint8_t ledPin = 6;         // Pin for LED output.
const uint8_t accPin = 7;         // Pin for accessory output.

const uint8_t STEPPER_PIN_0 = A3;
const uint8_t STEPPER_PIN_1 = A1;
const uint8_t STEPPER_PIN_2 = A2;
const uint8_t STEPPER_PIN_3 = A0;
#endif