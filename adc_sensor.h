#ifndef ADC_SENSOR_H
#define ADC_SENSOR_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include "config.h"

struct SensorReadings {
    float att_current;
    float rda_current;
    float main_current;
    float att_voltage;
    float rda_voltage;
    float main_voltage;
    float resistance_R2;
    float bipolar_voltage;
};

class ADCSensorManager {
private:
    Adafruit_ADS1115 ads1;  // 0x48 - Right ADS1115
    Adafruit_ADS1115 ads2;  // 0x49 - Left ADS1115
    
    int16_t adc0_1, adc1_1, adc2_1, adc3_1;  // ADS1 readings
    int16_t adc0_2, adc1_2, adc2_2, adc3_2;  // ADS2 readings
    
    SensorReadings readings;
    bool initialized;

public:
    ADCSensorManager();
    bool init();
    void readAllSensors();
    void calculateValues(uint8_t att_relay_state, uint8_t main_relay_state, uint8_t rda_relay_state);
    
    const SensorReadings& getReadings() const { return readings; }
    bool isReady() const { return initialized; }
};

#endif