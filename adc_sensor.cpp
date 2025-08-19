#include "adc_sensor.h"

ADCSensorManager::ADCSensorManager() : initialized(false) {
    memset(&readings, 0, sizeof(readings));
}

bool ADCSensorManager::init() {
    if (!ads1.begin(0x48)) {
        Serial.println("Right ADS1115 not found!");
        return false;
    }
    
    if (!ads2.begin(0x49)) {
        Serial.println("Left ADS1115 not found!");
        return false;
    }
    
    pinMode(35, INPUT);  // For RDA current measurement
    initialized = true;
    return true;
}

void ADCSensorManager::readAllSensors() {
    if (!initialized) return;
    
    // Read ADS1 (Right) - Res, Mv-, Mv+, ATT Current
    adc0_1 = ads1.readADC_SingleEnded(0);
    adc1_1 = ads1.readADC_SingleEnded(1);
    adc2_1 = ads1.readADC_SingleEnded(2);
    adc3_1 = ads1.readADC_SingleEnded(3);
    
    // Read ADS2 (Left) - ATT Voltage, Main Voltage, Main Current, RDA Voltage
    adc0_2 = ads2.readADC_SingleEnded(0);
    adc1_2 = ads2.readADC_SingleEnded(1);
    adc2_2 = ads2.readADC_SingleEnded(2);
    adc3_2 = ads2.readADC_SingleEnded(3);
}

void ADCSensorManager::calculateValues(uint8_t att_relay_state, uint8_t main_relay_state, uint8_t rda_relay_state) {
    if (!initialized) return;
    
    // Resistance calculation
    float Vout = (adc0_1 * Vin) / adcMaxValue;
    readings.resistance_R2 = (Vout * R1) / (Vin - Vout);
    
    // Bipolar voltage calculation (Mv-/Mv+)
    readings.bipolar_voltage = 0;
    if (adc1_1 <= 0 && adc2_1 > 0) {
        int16_t adj_adc1 = adc1_1 * -1;
        float v_ts = adj_adc1 + adc2_1;
        readings.bipolar_voltage = v_ts / 920.0;
    } else if (adc1_1 > 0 && adc2_1 <= 0) {
        int16_t adj_adc2 = adc2_1 * -1;
        float v_ts = adc1_1 + adj_adc2;
        readings.bipolar_voltage = v_ts / 920.0;
    }
    
    // Voltage calculations
    readings.att_voltage = (adc0_2 / 920.0) + 0.2;
    readings.main_voltage = (adc1_2 / 920.0) + 0.2;
    readings.rda_voltage = (adc3_2 / 920.0) + 0.2;
    
    // Current calculations (only when relays are active)
    if (att_relay_state > 0) {
        readings.att_current = (adc3_1 / 920.0) + 0.2 - 13.40;
    } else {
        readings.att_current = 0;
    }
    
    if (main_relay_state > 0) {
        readings.main_current = (adc2_2 / 920.0) + 0.2 - 12.30;
    } else {
        readings.main_current = 0;
    }
    
    if (rda_relay_state > 0) {
        float rda_voltage_raw = (analogRead(35) / 4095.0) * 3.3;
        rda_voltage_raw = rda_voltage_raw * (11.0 / 10.0);
        readings.rda_current = (rda_voltage_raw - 2.5) / 0.185 + 2.0;
    } else {
        readings.rda_current = 0;
    }
}