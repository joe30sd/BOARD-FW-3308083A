#include "tdk_communication.h"

TDKPowerSupply::TDKPowerSupply(HardwareSerial* ser) : serial(ser) {
    ask_mode = 0;
    live_current = 0.0;
    live_voltage = 0.0;
    idn_mode = false;
}

void TDKPowerSupply::init() {
    setOutput(false);
}

void TDKPowerSupply::setOutput(bool enabled) {
    serial->println("INST:NSEL 6");
    delay(100);
    if (enabled) {
        serial->println("OUTPut ON");
    } else {
        serial->println("OUTPut OFF");
    }
}

void TDKPowerSupply::setCurrent(float current) {
    serial->print("SOUR:CURR ");
    serial->println(current);
}

void TDKPowerSupply::setVoltage(float voltage) {
    serial->print("SOUR:VOLT ");
    serial->println(voltage);
}

void TDKPowerSupply::setCurrentVoltage(float current, float voltage) {
    setCurrent(current);
    setVoltage(voltage);
}

void TDKPowerSupply::requestMeasurements() {
    if (ask_mode == 0) {
        serial->println("MEAS:VOLT?");
        ask_mode = 1;
    } else {
        ask_mode = 0;
        serial->println("MEAS:CURR?");
    }
}

void TDKPowerSupply::requestIdentification() {
    serial->println("*IDN?");
    idn_mode = true;
}

void TDKPowerSupply::processResponse() {
    if (serial->available() > 0) {
        if (idn_mode) {
            device_id = serial->readString();
            Serial.print("TDK ID: ");
            Serial.println(device_id);
            idn_mode = false;
        } else {
            response_buffer = serial->readString();
            parseResponse(response_buffer);
        }
    }
}

void TDKPowerSupply::parseResponse(const String& response) {
    Serial.println(response);
    
    int curr_index = fastIndexOf(response, "MEAS:CURR?");
    int volt_index = fastIndexOf(response, "MEAS:VOLT?");
    
    if (ask_mode == 0 && curr_index <= 0) {
        live_current = response.toFloat();
    } else if (ask_mode == 1 && volt_index <= 0) {
        live_voltage = response.toFloat();
    }
    
    if (curr_index >= 0 && fastIndexOf(response, ".") >= 0) {
        String curr_str = response.substring(curr_index + 12, curr_index + 19);
        live_current = curr_str.toFloat();
    } else if (volt_index >= 0 && fastIndexOf(response, ".") >= 0) {
        String volt_str = response.substring(volt_index + 12, volt_index + 18);
        live_voltage = volt_str.toFloat();
    }
}

int TDKPowerSupply::fastIndexOf(const String &str, const String &toFind) {
    int strLen = str.length();
    int findLen = toFind.length();
    
    if (findLen == 0) return -1;
    
    for (int i = 0; i <= strLen - findLen; i++) {
        int j = 0;
        while (j < findLen && str[i + j] == toFind[j]) {
            j++;
        }
        if (j == findLen) return i;
    }
    return -1;
}