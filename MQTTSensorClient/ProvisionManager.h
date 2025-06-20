#ifndef PROVISION_MANAGER_H
#define PROVISION_MANAGER_H

#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <ArduinoECCX08.h>
#include "Config.h"

class ProvisionManager {
  public:
    ProvisionManager();
    
    // Initialisation
    bool begin();
    
    // Demande d'ID au serveur
    bool requestID();
    
    // Vérification si l'ID a été reçu
    bool isProvisioned();
    
    // Obtenir l'ID de l'appareil
    uint8_t getDeviceID();
    
    // Obtenir l'adresse du broker MQTT
    const char* getMQTTBroker();
    
    // Obtenir l'adresse MAC
    const char* getMACAddress();
    
    // Vérification périodique et retentatives
    void update();
    
    // Forcer une nouvelle demande de provisionnement
    void forceNewRequest();
    
    // Diagnostic et débogage
    void printStatus();
    
  private:
    WiFiUDP udp;
    char macAddress[18];  // XX:XX:XX:XX:XX:XX\0
    char mqttBrokerIP[16] = "";
    uint8_t deviceID = 0;
    bool idReceived = false;
    
    unsigned long lastRetryTime = 0;
    const int MAX_RETRIES = 5;
    const long RETRY_INTERVAL = 30000;  // 30 secondes
    const int RETRY_TIMEOUT = 2000;     // 2 secondes par tentative
    
    // Méthodes privées
    bool sendProvisionRequest(byte* randomToken);
    bool parseProvisionResponse(byte* randomToken);
    void initializeMACAddress();
    bool validateBrokerIP(const char* ip);
};

#endif
