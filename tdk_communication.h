#ifndef TDK_COMMUNICATION_H
#define TDK_COMMUNICATION_H

#include <Arduino.h>

class TDKPowerSupply {
private:
    HardwareSerial* serial;
    int ask_mode;
    float live_current;
    float live_voltage;
    String response_buffer;
    bool idn_mode;
    String device_id;
    
    int fastIndexOf(const String &str, const String &toFind);
    void parseResponse(const String& response);

public:
    TDKPowerSupply(HardwareSerial* ser);
    void init();
    void setOutput(bool enabled);
    void setCurrent(float current);
    void setVoltage(float voltage);
    void setCurrentVoltage(float current, float voltage);
    void requestMeasurements();
    void processResponse();
    void requestIdentification();
    
    float getLiveCurrent() const { return live_current; }
    float getLiveVoltage() const { return live_voltage; }
    String getDeviceID() const { return device_id; }
    bool isIdnMode() const { return idn_mode; }
    void clearIdnMode() { idn_mode = false; }
};

#endif