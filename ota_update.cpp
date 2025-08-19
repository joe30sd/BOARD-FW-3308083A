#include "ota_update.h"

OTAManager::OTAManager(DisplayCommunication* disp) : display(disp), wifi_connected(false) {}

bool OTAManager::connectWiFi() {
    display->sendDiagnosticResult(51);  // Connecting to WiFi
    Serial.println("Connecting to WiFi...");
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
        wifi_connected = true;
        return true;
    } else {
        Serial.println("\nFailed to connect to WiFi");
        display->sendDiagnosticResult(53);  // Update failed
        return false;
    }
}

void OTAManager::disconnectWiFi() {
    WiFi.disconnect();
    wifi_connected = false;
}

bool OTAManager::performUpdate() {
    if (!connectWiFi()) {
        return false;
    }
    
    HTTPClient http;
    http.begin(firmwareUrl);
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        int contentLength = http.getSize();
        
        if (contentLength > 0) {
            bool canBegin = Update.begin(contentLength);
            
            if (canBegin) {
                Serial.println("Starting OTA update...");
                WiFiClient* stream = http.getStreamPtr();
                
                size_t written = Update.writeStream(*stream);
                
                if (written == contentLength) {
                    Serial.println("Written : " + String(written) + " bytes successfully");
                } else {
                    Serial.println("Written only : " + String(written) + "/" + String(contentLength) + " bytes");
                    http.end();
                    disconnectWiFi();
                    display->sendDiagnosticResult(53);  // Update failed
                    return false;
                }
                
                if (Update.end()) {
                    Serial.println("OTA update completed!");
                    display->sendDiagnosticResult(52);  // Update successful
                    
                    if (Update.isFinished()) {
                        Serial.println("Update successful. Rebooting...");
                        http.end();
                        disconnectWiFi();
                        ESP.restart();
                    } else {
                        Serial.println("Update not finished. Something went wrong!");
                        display->sendDiagnosticResult(53);  // Update failed
                        http.end();
                        disconnectWiFi();
                        return false;
                    }
                } else {
                    Serial.println("Update error: #" + String(Update.getError()));
                    display->sendDiagnosticResult(53);  // Update failed
                    http.end();
                    disconnectWiFi();
                    return false;
                }
            } else {
                Serial.println("Not enough space to begin OTA update");
                display->sendDiagnosticResult(53);  // Update failed
                http.end();
                disconnectWiFi();
                return false;
            }
        } else {
            Serial.println("Firmware size is invalid (<= 0)");
            display->sendDiagnosticResult(53);  // Update failed
            http.end();
            disconnectWiFi();
            return false;
        }
    } else {
        Serial.println("HTTP error: " + String(httpCode));
        display->sendDiagnosticResult(53);  // Update failed
        http.end();
        disconnectWiFi();
        return false;
    }
    
    http.end();
    disconnectWiFi();
    return true;
}