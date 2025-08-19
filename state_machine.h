#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "relay_control.h"
#include "tdk_communication.h"
#include "display_communication.h"

enum DisplayMode {
    MODE_IDLE = 0,
    MODE_MANUAL = 1,
    MODE_RAMP_UP = 2,
    MODE_RAMP_DOWN = 3,
    MODE_UNUSED = 4,
    MODE_DIAGNOSTIC = 5
};

struct SystemState {
    float magnet_current;
    float magnet_voltage;
    float u_target_current;
    float u_inc_current;
    float u_voltage;
    float u_check;
    uint8_t up_mode;
    uint8_t up_done;
    
    float d_start_current;
    float d_dec_current;
    float d_voltage;
    float d_check;
    uint8_t d_mode;
    uint8_t d_done;
    
    uint8_t att_relay;
    uint8_t main_relay;
    uint8_t rda_relay;
    uint8_t rda_relay_dm;
    
    int minutes_passed;
    int minutes2;
    int dm4_timer;
    
    DisplayMode display_mode;
    int diag_x;
    unsigned long diag_start_time;
    uint8_t g_mode;
};

class StateMachine {
private:
    SystemState state;
    RelayController* relay_ctrl;
    TDKPowerSupply* tdk_psu;
    DisplayCommunication* display;
    
    void resetState();
    void handleManualMode(const String& command);
    void handleRampUpMode(const String& command);
    void handleRampDownMode(const String& command);
    void handleDiagnosticMode(const String& command);
    
    void processRampUp();
    void processRampDown();
    void processDiagnostic();
    
public:
    StateMachine(RelayController* relay, TDKPowerSupply* tdk, DisplayCommunication* disp);
    void init();
    void update(unsigned long current_millis, int minutes_elapsed);
    void processCommand(const String& command);
    
    const SystemState& getState() const { return state; }
    DisplayMode getDisplayMode() const { return state.display_mode; }
};

#endif