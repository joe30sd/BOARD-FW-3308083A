#include "state_machine.h"

StateMachine::StateMachine(RelayController* relay, TDKPowerSupply* tdk, DisplayCommunication* disp) 
    : relay_ctrl(relay), tdk_psu(tdk), display(disp) {
    resetState();
}

void StateMachine::init() {
    resetState();
}

void StateMachine::resetState() {
    state.magnet_current = 0;
    state.magnet_voltage = 0;
    state.u_target_current = 0;
    state.u_inc_current = 0;
    state.u_voltage = 6.00;
    state.u_check = 0.8;
    state.up_mode = 0;
    state.up_done = 0;
    
    state.d_start_current = 0;
    state.d_dec_current = 0;
    state.d_voltage = 6.00;
    state.d_check = 0.9;
    state.d_mode = 0;
    state.d_done = 0;
    
    state.att_relay = 0;
    state.main_relay = 0;
    state.rda_relay = 0;
    state.rda_relay_dm = 0;
    
    state.minutes_passed = 0;
    state.minutes2 = 0;
    state.dm4_timer = 0;
    
    state.display_mode = MODE_IDLE;
    state.diag_x = 0;
    state.diag_start_time = 0;
    state.g_mode = 0;
}

void StateMachine::processCommand(const String& command) {
    if (command.startsWith("D")) {
        state.display_mode = (DisplayMode)command.substring(1).toInt();
        if (state.display_mode == MODE_MANUAL) {
            tdk_psu->setOutput(true);
        }
        return;
    }
    
    if (command.startsWith("G")) {
        state.g_mode = command.substring(1).toInt();
        if (state.g_mode == 1) {
            relay_ctrl->allOff();
            tdk_psu->setCurrentVoltage(0.0, 0.0);
        }
        return;
    }
    
    switch (state.display_mode) {
        case MODE_MANUAL:
            handleManualMode(command);
            break;
        case MODE_RAMP_UP:
            handleRampUpMode(command);
            break;
        case MODE_RAMP_DOWN:
            handleRampDownMode(command);
            break;
        case MODE_DIAGNOSTIC:
            handleDiagnosticMode(command);
            break;
        default:
            break;
    }
}

void StateMachine::handleManualMode(const String& command) {
    if (command.startsWith("V")) {
        state.magnet_voltage = command.substring(1).toFloat();
    } else if (command.startsWith("C")) {
        state.magnet_current = command.substring(1).toFloat();
    } else if (command.startsWith("A")) {
        state.att_relay = command.substring(1).toInt();
    } else if (command.startsWith("M")) {
        state.main_relay = command.substring(1).toInt();
    } else if (command.startsWith("R")) {
        state.rda_relay = command.substring(1).toInt();
    }
}

void StateMachine::handleRampUpMode(const String& command) {
    if (command.startsWith("T")) {
        state.u_target_current = command.substring(1).toFloat();
    } else if (command.startsWith("I")) {
        state.u_inc_current = command.substring(1).toFloat();
    } else if (command.startsWith("U")) {
        state.up_mode = command.substring(1).toInt();
    } else if (command.startsWith("A")) {
        state.att_relay = command.substring(1).toInt();
    } else if (command.startsWith("M")) {
        state.main_relay = command.substring(1).toInt();
    } else if (command.startsWith("R")) {
        state.rda_relay = command.substring(1).toInt();
    } else if (command.startsWith("V") && state.up_mode == 6) {
        state.magnet_voltage = command.substring(1).toFloat();
    } else if (command.startsWith("C") && state.up_mode == 6) {
        state.magnet_current = command.substring(1).toFloat();
    }
}

void StateMachine::handleRampDownMode(const String& command) {
    if (command.startsWith("T")) {
        state.d_start_current = command.substring(1).toFloat();
    } else if (command.startsWith("I")) {
        state.d_dec_current = command.substring(1).toFloat();
    } else if (command.startsWith("U")) {
        state.d_mode = command.substring(1).toInt();
    } else if (command.startsWith("A")) {
        state.att_relay = command.substring(1).toInt();
    } else if (command.startsWith("M")) {
        state.main_relay = command.substring(1).toInt();
        state.main_relay = 0; // Force main relay off in ramp down
    } else if (command.startsWith("R")) {
        state.rda_relay_dm = command.substring(1).toInt();
        state.rda_relay = 0;
    }
}

void StateMachine::handleDiagnosticMode(const String& command) {
    if (command.startsWith("X")) {
        state.diag_x = command.substring(1).toInt();
        tdk_psu->setOutput(false);
    }
}

void StateMachine::update(unsigned long current_millis, int minutes_elapsed) {
    state.minutes_passed = minutes_elapsed;
    
    switch (state.display_mode) {
        case MODE_RAMP_UP:
            processRampUp();
            break;
        case MODE_RAMP_DOWN:
            processRampDown();
            break;
        case MODE_DIAGNOSTIC:
            processDiagnostic();
            break;
        default:
            break;
    }
}

void StateMachine::processRampUp() {
    // Implementation of ramp up logic from original code
    // This is a simplified version - full implementation would include all cases
    switch (state.up_mode) {
        case 1:
            state.minutes2 = 0;
            tdk_psu->setCurrentVoltage(0.0, 0.0);
            state.up_done = 1;
            tdk_psu->setOutput(true);
            break;
        case 2:
            relay_ctrl->setRelays(state.att_relay, state.main_relay, state.rda_relay);
            state.up_done = 2;
            break;
        // Additional cases would be implemented here
    }
}

void StateMachine::processRampDown() {
    // Implementation of ramp down logic from original code
    switch (state.d_mode) {
        case 1:
            state.rda_relay = state.rda_relay_dm;
            relay_ctrl->setRelays(0, 0, state.rda_relay);
            state.rda_relay_dm = 0;
            state.magnet_current = state.d_start_current;
            state.magnet_voltage = state.d_voltage;
            tdk_psu->setCurrentVoltage(state.magnet_current, state.magnet_voltage);
            tdk_psu->setOutput(true);
            state.d_mode = 2;
            break;
        // Additional cases would be implemented here
    }
}

void StateMachine::processDiagnostic() {
    // Diagnostic mode processing
    // Implementation would include all diagnostic test cases
}