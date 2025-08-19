#include "relay_control.h"

RelayController::RelayController() {
    att_relay_state = 0;
    main_relay_state = 0;
    rda_relay_state = 0;
}

void RelayController::init() {
    pinMode(AX_relay_pin, OUTPUT);
    pinMode(T1_relay_pin, OUTPUT);
    pinMode(T2_relay_pin, OUTPUT);
    pinMode(RDA_relay_pin, OUTPUT);
    pinMode(MAIN_relay_pin, OUTPUT);
    
    allOff();
}

void RelayController::setRelays(uint8_t att_value, uint8_t main_value, uint8_t rda_value) {
    att_relay_state = att_value;
    main_relay_state = main_value;
    rda_relay_state = rda_value;
    
    // ATT relay control (3 different configurations)
    if (att_value == 0) {
        digitalWrite(AX_relay_pin, LOW);
        digitalWrite(T1_relay_pin, LOW);
        digitalWrite(T2_relay_pin, LOW);
    } else if (att_value == 1) {
        digitalWrite(AX_relay_pin, HIGH);
        digitalWrite(T1_relay_pin, LOW);
        digitalWrite(T2_relay_pin, LOW);
    } else if (att_value == 2) {
        digitalWrite(AX_relay_pin, LOW);
        digitalWrite(T1_relay_pin, HIGH);
        digitalWrite(T2_relay_pin, LOW);
    } else if (att_value == 3) {
        digitalWrite(AX_relay_pin, LOW);
        digitalWrite(T1_relay_pin, LOW);
        digitalWrite(T2_relay_pin, HIGH);
    }
    
    // Main relay control
    digitalWrite(MAIN_relay_pin, main_value ? HIGH : LOW);
    
    // RDA relay control
    digitalWrite(RDA_relay_pin, rda_value ? HIGH : LOW);
}

void RelayController::allOff() {
    setRelays(0, 0, 0);
}