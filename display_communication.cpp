#include "display_communication.h"

DisplayCommunication::DisplayCommunication(HardwareSerial* ser) : serial(ser) {}

void DisplayCommunication::init() {
    // Display initialization if needed
}

void DisplayCommunication::sendValues(float magnet_voltage, float magnet_current, 
                                     float live_voltage, float live_current,
                                     const SensorReadings& sensors,
                                     uint8_t att_relay, uint8_t main_relay, uint8_t rda_relay,
                                     int minutes, uint8_t up_mode, uint8_t up_done) {
    serial->print("MV"); serial->println(magnet_voltage);
    serial->print("MC"); serial->println(magnet_current);
    serial->print("LV"); serial->println(live_voltage);
    serial->print("LC"); serial->println(live_current);
    serial->print("AV"); serial->println(sensors.att_voltage);
    serial->print("CV"); serial->println(sensors.main_voltage);
    serial->print("RV"); serial->println(sensors.rda_voltage);
    serial->print("RA"); serial->println(att_relay);
    serial->print("RM"); serial->println(main_relay);
    serial->print("RR"); serial->println(rda_relay);
    serial->print("TP"); serial->println(minutes);
    serial->print("UP"); serial->println(up_mode);
    serial->print("UD"); serial->println(up_done);
    serial->print("OR"); serial->println(sensors.resistance_R2);
    serial->print("OV"); serial->println(sensors.bipolar_voltage);
}

void DisplayCommunication::sendDiagnosticResult(int result_code) {
    serial->print("YD");
    serial->println(result_code);
}

void DisplayCommunication::sendDeviceID(const String& id) {
    serial->print("ID");
    serial->println(id);
}

bool DisplayCommunication::hasCommand() {
    return serial->available() > 0;
}

String DisplayCommunication::readCommand() {
    if (serial->available() > 0) {
        command_buffer = serial->readStringUntil('\n');
        return command_buffer;
    }
    return "";
}

void DisplayCommunication::sendInitialValues(const SensorReadings& sensors) {
    serial->print("AV"); serial->println(sensors.att_voltage);
    serial->print("CV"); serial->println(sensors.main_voltage);  
    serial->print("RV"); serial->println(sensors.rda_voltage);
    serial->print("RA");
    delay(50);
}