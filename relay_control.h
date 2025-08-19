#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include <Arduino.h>
#include "config.h"

class RelayController {
private:
    uint8_t att_relay_state;
    uint8_t main_relay_state;
    uint8_t rda_relay_state;

public:
    RelayController();
    void init();
    void setRelays(uint8_t att_value, uint8_t main_value, uint8_t rda_value);
    void allOff();
    
    uint8_t getAttState() const { return att_relay_state; }
    uint8_t getMainState() const { return main_relay_state; }
    uint8_t getRdaState() const { return rda_relay_state; }
};

#endif