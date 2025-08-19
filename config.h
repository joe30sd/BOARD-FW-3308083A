#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
const char* ssid = "Modric";
const char* password = "arctic9744";

// Pin Definitions
#define RXD2 16
#define TXD2 17
#define RXD3 32
#define TXD3 33

#define AX_relay_pin 25
#define T1_relay_pin 26
#define T2_relay_pin 27
#define RDA_relay_pin 12
#define MAIN_relay_pin 14

// Timing Constants
const unsigned long MAIN_INTERVAL = 60000; // 1 minute
const unsigned long DISPLAY_INTERVAL = 250; // 250ms

// ADC Constants
const float Vin = 3.3;
const float R1 = 2000.0;
const int adcMaxValue = 17800;

// OTA Configuration
const char* firmwareUrl = "https://raw.githubusercontent.com/byronin/rnn-bins/main/firmware2.bin";

#endif