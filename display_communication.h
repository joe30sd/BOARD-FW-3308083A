#ifndef DISPLAY_COMMUNICATION_H
#define DISPLAY_COMMUNICATION_H

#include <Arduino.h>
#include "adc_sensor.h"

class DisplayCommunication {
private:
    HardwareSerial* serial;
    String command_buffer;
    
public:
    DisplayCommunication(HardwareSerial* ser);
    void init();
    void sendValues(float magnet_voltage, float magnet_current, 
                   float live_voltage, float live_current,
                   const SensorReadings& sensors,
                   uint8_t att_relay, uint8_t main_relay, uint8_t rda_relay,
                   int minutes, uint8_t up_mode, uint8_t up_done);
    void sendDiagnosticResult(int result_code);
    void sendDeviceID(const String& id);
    bool hasCommand();
    String readCommand();
    void sendInitialValues(const SensorReadings& sensors);
};

#endif