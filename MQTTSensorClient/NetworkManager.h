#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include "Config.h"

class NetworkManager {
  public:
    NetworkManager();
    
    // Connexion au WiFi
    bool connectWiFi();
    
    // Connexion au broker MQTT avec adresse IP dynamique
    bool connectMQTT(const char* brokerIP, uint8_t deviceId);
    
    // Envoi d'un message MQTT
    bool sendMessage(const char* topic, const uint8_t* buffer, size_t length);
    
    // Déconnexion
    void disconnect();
    
    // Vérification de l'état de connexion
    bool isWiFiConnected();
    bool isMQTTConnected();
    
  private:
    WiFiClient wifiClient;
    MqttClient mqttClient;
    char clientId[20]; // Assez grand pour "arduino-" + ID (en décimal)
    
    // Gestion des timeouts
    bool waitForWiFiConnection(unsigned long timeout);
};

#endif
