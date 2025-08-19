#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include "config.h"
#include "display_communication.h"

class OTAManager {
private:
    DisplayCommunication* display;
    bool wifi_connected;
    
    bool connectWiFi();
    void disconnectWiFi();

public:
    OTAManager(DisplayCommunication* disp);
    bool performUpdate();
};

#endif