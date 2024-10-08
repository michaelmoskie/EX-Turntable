/*
 *  © 2023 Peter Cole
 *
 *  This file is part of EX-Turntable
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EX-Turntable.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "IOFunctions.h"
#include "EEPROMFunctions.h"
#include <avr/wdt.h>

unsigned long gearingFactor = STEPPER_GEARING_FACTOR;
const byte numChars = 20;
char serialInputChars[numChars];
bool newSerialData = false;
bool testCommandSent = false;
uint8_t testActivity = 0;
uint8_t testStepsMSB = 0;
uint8_t testStepsLSB = 0;
#ifdef DEBUG
bool debug = true;
#else
bool debug = false;
#endif
#ifdef SENSOR_TESTING
bool sensorTesting = true;
#else
bool sensorTesting = false;
#endif

// Function to setup Wire library and functions
void setupWire() {
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

// Function to read and process serial input for valid test commands
void processSerialInput() {
  static bool serialInProgress = false;
  static byte serialIndex = 0;
  char startMarker = '<';
  char endMarker = '>';
  char serialChar;
  while (Serial.available() > 0 && newSerialData == false) {
    serialChar = Serial.read();
    if (serialInProgress == true) {
      if (serialChar != endMarker) {
        serialInputChars[serialIndex] = serialChar;
        serialIndex++;
        if (serialIndex >= numChars) {
          serialIndex = numChars - 1;
        }
      } else {
        serialInputChars[serialIndex] = '\0';
        serialInProgress = false;
        serialIndex = 0;
        newSerialData = true;
      }
    } else if (serialChar == startMarker) {
      serialInProgress = true;
    }
  }
  if (newSerialData == true) {
    newSerialData = false;
    char *strtokIndex;
    strtokIndex = strtok(serialInputChars, " ");
    char command = strtokIndex[0];   // first parameter is activity
    strtokIndex = strtok(NULL, " "); // space separator
    long steps;
    if (command == 'M') {
      steps = atol(strtokIndex);
      strtokIndex = strtok(NULL, " ");
      testActivity = atoi(strtokIndex);
    }
    switch (command) {
    case 'C':
      serialCommandC();
      break;

    case 'D':
      serialCommandD();
      break;

    case 'E':
      serialCommandE();
      break;

    case 'H':
      serialCommandH();
      break;

    case 'M':
      serialCommandM(steps);
      break;

    case 'R':
      serialCommandR();
      break;

    case 'T':
      serialCommandT();
      break;

    case 'V':
      serialCommandV();
      break;

    default:
      break;
    }
  }
}

// C command to initiate calibration
void serialCommandC() {
  if (stepper.isRunning()) {
    Serial.println(F("Stepper is running, ignoring <C>"));
    return;
  }
  if (!calibrating || homed == 2) {
    initiateCalibration();
  }
}

// D command to enable debug output
void serialCommandD() {
  if (debug) {
    Serial.println(F("Disabling debug output"));
    debug = false;
  } else {
    Serial.println(F("Enabling debug output"));
    debug = true;
  }
}

// E command to erase EEPROM
void serialCommandE() {
  if (stepper.isRunning()) {
    Serial.println(F("Stepper is running, ignoring <E>"));
    return;
  }
  Serial.println(F("Erasing full step count from EEPROM"));
  clearEEPROM();
#ifndef FULL_STEP_COUNT
  Serial.println(F("Resetting full step count to 0"));
  fullTurnSteps = 0;
#endif
}

// H command to initiate homing
void serialCommandH() {
  if (stepper.isRunning()) {
    Serial.println(F("Stepper is running, ignoring <H>"));
    return;
  }
  if (!calibrating || homed == 2) {
    initiateHoming();
  }
}

// M command to move
void serialCommandM(long steps) {
  if (stepper.isRunning()) {
    Serial.println(F("Stepper is running, ignoring <M>"));
    return;
  }
  if (steps < 0) {
    Serial.println(F("Cannot provide a negative step count"));
  } else if (steps > 32767) {
    Serial.println(F("Step count too large, refer to the documentation for "
                     "large step counts > 32767"));
  } else {
    Serial.print(F("Test move "));
    Serial.print(steps);
    Serial.print(F(" steps, activity ID "));
    Serial.println(testActivity);
    testStepsMSB = steps >> 8;
    testStepsLSB = steps & 0xFF;
    testCommandSent = true;
    receiveEvent(3);
  }
}

void serialCommandR() {
  wdt_enable(WDTO_15MS);
  delay(50);
}

// T command to perform sensor testing
void serialCommandT() {
  if (stepper.isRunning()) {
    Serial.println(F("Stepper is running, ignoring <T>"));
    return;
  }
  if (sensorTesting) {
    Serial.println(F("Disabling sensor testing mode, reboot required"));
    sensorTesting = false;
  } else {
    Serial.println(
        F("Enabling sensor testing mode, taking EX-Turntable offline"));
    Wire.end();
    sensorTesting = true;
  }
}

// V command to display version and other info
void serialCommandV() { displayTTEXConfig(); }

// Function to display the defined stepper motor config.
void displayTTEXConfig() {
  // Basic setup, display what this is.
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println(F("License GPLv3 fsf.org (c) dcc-ex.com"));
  Serial.print(F("EX-Turntable version "));
  Serial.println(VERSION);
  Serial.print(F("Available at I2C address 0x"));
  Serial.println(I2C_ADDRESS, HEX);
  if (fullTurnSteps == 0) {
    Serial.println(F("EX-Turntable has not been calibrated yet"));
  } else {
#ifdef FULL_STEP_COUNT
    Serial.print(F("Manual override has been set for "));
#else
    Serial.print(F("EX-Turntable has been calibrated for "));
#endif
    Serial.print(fullTurnSteps);
    Serial.println(F(" steps per revolution"));
  }
  Serial.print(F("Gearing factor set to "));
  Serial.println(gearingFactor);
#if PHASE_SWITCHING == AUTO
  Serial.print(F("Automatic phase switching enabled at "));
  Serial.print(PHASE_SWITCH_ANGLE);
  Serial.println(F(" degrees"));
  Serial.print(F("Phase will switch at "));
  Serial.print(phaseSwitchStartSteps);
  Serial.print(F(" steps from home, and revert at "));
  Serial.print(phaseSwitchStopSteps);
  Serial.println(F(" steps from home"));
#else
  Serial.println(F("Manual phase switching enabled"));
#endif
#if TURNTABLE_EX_MODE == TRAVERSER
  Serial.println(F("EX-Turntable in TRAVERSER mode"));
#else
  Serial.println(F("EX-Turntable in TURNTABLE mode"));
#endif

#if defined(ROTATE_FORWARD_ONLY)
  Serial.println(F("Rotating FORWARD only"));
#elif defined(ROTATE_REVERSE_ONLY)
  Serial.println(F("Rotating REVERSE only"));
#else
  Serial.println(F("Rotating SHORTEST DIRECTION"));
#endif

#if defined(INVERT_DIRECTION)
  Serial.println(F("INVERT_DIRECTION enabled"));
#endif
#if defined(INVERT_STEP)
  Serial.println(F("INVERT_STEP enabled"));
#endif
#if defined(INVERT_ENABLE)
  Serial.println(F("INVERT_ENABLE enabled"));
#endif

  Serial.print(F("STEPPER_MAX_SPEED "));
  Serial.println(STEPPER_MAX_SPEED);
  Serial.print(F("STEPPER_ACCELERATION "));
  Serial.println(STEPPER_ACCELERATION);

  if (debug) {
    Serial.print(F("DEBUG: maxSpeed()|acceleration(): "));
    Serial.print(stepper.maxSpeed());
    Serial.print(F("|"));
    Serial.println(stepper.acceleration());
  }

  // If in sensor testing mode, display this, don't enable stepper or I2C
  if (sensorTesting) {
    Serial.println(
        F("SENSOR TESTING ENABLED, EX-Turntable operations disabled"));
    Serial.print(F("Home/limit switch current state: "));
    Serial.print(homeSensorState);
    Serial.print(F("/"));
    Serial.println(limitSensorState);
    Serial.print(F("Debounce delay: "));
    Serial.println(DEBOUNCE_DELAY);
  }
}

// Function to define the action on a received I2C event.
void receiveEvent(int received) {
  if (debug) {
    Serial.print(F("DEBUG: Received "));
    Serial.print(received);
    Serial.println(F(" bytes"));
  }
  int16_t receivedSteps;
  long steps;
  uint8_t activity;
  uint8_t receivedStepsMSB;
  uint8_t receivedStepsLSB;
  // We need 3 received bytes in order to care about what's received.
  if (received == 3) {
    // Get our 3 bytes of data, bit shift into steps.
    if (testCommandSent == true) {
      receivedStepsMSB = testStepsMSB;
      receivedStepsLSB = testStepsLSB;
      activity = testActivity;
      testCommandSent = false;
    } else {
      receivedStepsMSB = Wire.read();
      receivedStepsLSB = Wire.read();
      activity = Wire.read();
    }
    receivedSteps = (receivedStepsMSB << 8) + receivedStepsLSB;
    if (gearingFactor > 10) {
      gearingFactor = 10;
    }
    steps = receivedSteps * gearingFactor;
    if (debug) {
      Serial.print(F("DEBUG: receivedStepsMSB|receivedStepsLSB|activity: "));
      Serial.print(receivedStepsMSB);
      Serial.print(F("|"));
      Serial.print(receivedStepsLSB);
      Serial.print(F("|"));
      Serial.println(activity);
      Serial.print(F("DEBUG: gearingFactor|receivedSteps|steps: "));
      Serial.print(gearingFactor);
      Serial.print(F("|"));
      Serial.print(receivedSteps);
      Serial.print(F("|"));
      Serial.println(steps);
    }
    if (steps <= fullTurnSteps && activity < 2 && !stepper.isRunning() &&
        !calibrating) {
      // Activities 0/1 require turning and setting phase, process only if
      // stepper is not running.
      if (debug) {
        Serial.print(F("DEBUG: Requested valid step move to: "));
        Serial.print(steps);
        Serial.print(F(" with phase switch: "));
        Serial.println(activity);
      }
      moveToPosition(steps, activity);
    } else if (activity == 2 && !stepper.isRunning() &&
               (!calibrating || homed == 2)) {
      // Activity 2 needs to reset our homed flag to initiate the homing
      // process, only if stepper not running.
      if (debug) {
        Serial.println(F("DEBUG: Requested to home"));
      }
      initiateHoming();
    } else if (activity == 3 && !stepper.isRunning() &&
               (!calibrating || homed == 2)) {
      // Activity 3 will initiate calibration sequence, only if stepper not
      // running.
      if (debug) {
        Serial.println(F("DEBUG: Calibration requested"));
      }
      initiateCalibration();
    } else if (activity > 3 && activity < 8) {
      // Activities 4 through 7 set LED state.
      if (debug) {
        Serial.print(F("DEBUG: Set LED state to: "));
        Serial.println(activity);
      }
      setLEDActivity(activity);
    } else if (activity == 8) {
      // Activity 8 turns accessory pin on at any time.
      if (debug) {
        Serial.println(F("DEBUG: Turn accessory pin on"));
      }
      setAccessory(HIGH);
    } else if (activity == 9) {
      // Activity 9 turns accessory pin off at any time.
      if (debug) {
        Serial.println(F("DEBUG: Turn accessory pin off"));
      }
      setAccessory(LOW);
    } else {
      if (debug) {
        Serial.print(F("DEBUG: Invalid step count or activity provided, or "
                       "turntable still moving: "));
        Serial.print(steps);
        Serial.print(F(" steps, activity: "));
        Serial.println(activity);
      }
    }
  } else {
    // Even if we have nothing to do, we need to read and discard all the bytes
    // to avoid timeouts in the CS.
    if (debug) {
      Serial.println(
          F("DEBUG: Incorrect number of bytes received, discarding"));
    }
    while (Wire.available()) {
      Wire.read();
    }
  }
}

// Function to return the stepper status when requested by the IO_TurntableEX.h
// device driver. 0 = Finished moving to the correct position. 1 = Still moving.
void requestEvent() {
  uint8_t stepperStatus;
  if (stepper.isRunning()) {
    stepperStatus = 1;
  } else {
    stepperStatus = 0;
  }
  Wire.write(stepperStatus);
}
