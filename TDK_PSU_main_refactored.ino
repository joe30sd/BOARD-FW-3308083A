#include <Arduino.h>
#include <Wire.h>

#include "config.h"

// Define global constants from config.h
const char* ssid = "Modric";
const char* password = "arctic9744";
const char* firmwareUrl = "https://raw.githubusercontent.com/byronin/rnn-bins/main/firmware2.bin";
#include "relay_control.h"
#include "adc_sensor.h"
#include "tdk_communication.h"
#include "display_communication.h"
#include "state_machine.h"
#include "ota_update.h"

// Hardware Serial instances
HardwareSerial SerialPort(2);

// System components
RelayController relayController;
ADCSensorManager sensorManager;
TDKPowerSupply tdkPSU(&Serial1);
DisplayCommunication displayComm(&Serial2);
StateMachine stateMachine(&relayController, &tdkPSU, &displayComm);
OTAManager otaManager(&displayComm);

// Timing variables
unsigned long previousMillis = 0;
unsigned long pvm1 = 0, pvm2 = 0, pvm3 = 0;
unsigned long diag_pvm = 0;
int minutesPassed = 0;
int old_value = 0;

void setup() {
    Serial.begin(115200);
    delay(100);
    
    // Initialize serial communications
    Serial1.begin(115200, SERIAL_8N1, RXD2, TXD2);  // TDK PSU
    Serial2.begin(115200, SERIAL_8N1, RXD3, TXD3);  // Display
    
    // Initialize all components
    if (!sensorManager.init()) {
        Serial.println("Sensor initialization failed!");
        while (1);
    }
    
    relayController.init();
    tdkPSU.init();
    displayComm.init();
    stateMachine.init();
    
    Serial.println("System initialized successfully");
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Update minute counter
    if (currentMillis - previousMillis >= MAIN_INTERVAL) {
        previousMillis = currentMillis;
        minutesPassed++;
    }
    
    // Handle display mode initialization
    if (stateMachine.getDisplayMode() == MODE_IDLE) {
        handleIdleMode();
        return;
    }
    
    // Process display commands
    if (displayComm.hasCommand()) {
        String command = displayComm.readCommand();
        stateMachine.processCommand(command);
        
        // Handle OTA update request
        if (command.startsWith("X50")) {
            otaManager.performUpdate();
        }
    }
    
    // Process TDK responses
    tdkPSU.processResponse();
    
    // Update sensor readings
    updateSensors();
    
    // Update state machine
    stateMachine.update(currentMillis, minutesPassed);
    
    // Handle periodic tasks based on display mode
    handlePeriodicTasks(currentMillis);
    
    // Set initial values if needed
    if (old_value == 0) {
        const SystemState& state = stateMachine.getState();
        tdkPSU.setCurrentVoltage(state.magnet_current, state.magnet_voltage);
        old_value = 1;
    }
}

void handleIdleMode() {
    // Reset all values and send initial display data
    minutesPassed = 0;
    old_value = 0;
    
    const SensorReadings& sensors = sensorManager.getReadings();
    displayComm.sendInitialValues(sensors);
}

void updateSensors() {
    if (sensorManager.isReady()) {
        sensorManager.readAllSensors();
        sensorManager.calculateValues(
            relayController.getAttState(),
            relayController.getMainState(),
            relayController.getRdaState()
        );
    }
}

void handlePeriodicTasks(unsigned long currentMillis) {
    const SystemState& state = stateMachine.getState();
    
    switch (state.display_mode) {
        case MODE_MANUAL:
            if (currentMillis - pvm1 >= DISPLAY_INTERVAL) {
                pvm1 = currentMillis;
                relayController.setRelays(state.att_relay, state.main_relay, state.rda_relay);
                tdkPSU.requestMeasurements();
                sendDataToDisplay();
            }
            break;
            
        case MODE_RAMP_UP:
        case MODE_RAMP_DOWN:
            if (currentMillis - pvm2 >= DISPLAY_INTERVAL) {
                pvm2 = currentMillis;
                tdkPSU.requestMeasurements();
                sendDataToDisplay();
            }
            break;
            
        case MODE_DIAGNOSTIC:
            if (currentMillis - pvm1 >= DISPLAY_INTERVAL && state.diag_x > 20) {
                pvm1 = currentMillis;
                tdkPSU.requestMeasurements();
                handleDiagnosticChecks();
            }
            break;
    }
}

void sendDataToDisplay() {
    const SystemState& state = stateMachine.getState();
    const SensorReadings& sensors = sensorManager.getReadings();
    
    displayComm.sendValues(
        state.magnet_voltage,
        state.magnet_current,
        tdkPSU.getLiveVoltage(),
        tdkPSU.getLiveCurrent(),
        sensors,
        state.att_relay,
        state.main_relay,
        state.rda_relay,
        state.minutes2,
        state.up_mode,
        state.up_done
    );
}

void handleDiagnosticChecks() {
    const SystemState& state = stateMachine.getState();
    
    // Diagnostic timeout and current checks
    if (millis() - diag_pvm < 7000 && state.diag_x == 26 && tdkPSU.getLiveCurrent() > 740) {
        // Diagnostic passed
        displayComm.sendDiagnosticResult(27);
    } else if (millis() - diag_pvm > 7000 && state.diag_x == 26 && tdkPSU.getLiveCurrent() < 740) {
        // Diagnostic failed
        displayComm.sendDiagnosticResult(28);
    }
    
    if (millis() - diag_pvm > 7000) {
        if (tdkPSU.getLiveCurrent() < 1 && tdkPSU.getLiveVoltage() > 5.8 && state.diag_x == 21) {
            displayComm.sendDiagnosticResult(24);
            tdkPSU.setCurrentVoltage(0.0, 0.0);
            tdkPSU.setOutput(false);
        }
    }
}