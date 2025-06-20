#include "NetworkManager.h"

NetworkManager::NetworkManager() : mqttClient(wifiClient) {
  // Initialisation du client ID vide
  memset(clientId, 0, sizeof(clientId));
}

bool NetworkManager::connectWiFi() {
  // Vérifier si déjà connecté
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  
  DEBUG_PRINT("Connexion WiFi à ");
  DEBUG_PRINT(WIFI_SSID);
  DEBUG_PRINT("...");
  
  // Démarrer la connexion
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Attendre la connexion avec timeout
  if (waitForWiFiConnection(10000)) {
    DEBUG_PRINTLN(" OK");
    DEBUG_PRINT("Adresse IP: ");
    DEBUG_PRINTLN(WiFi.localIP());
    return true;
  } else {
    DEBUG_PRINTLN(" ÉCHEC");
    return false;
  }
}

bool NetworkManager::connectMQTT(const char* brokerIP, uint8_t deviceId) {
  // Vérifier si déjà connecté
  if (mqttClient.connected()) {
    return true;
  }
  
  DEBUG_PRINT("Connexion MQTT à ");
  DEBUG_PRINT(brokerIP);
  DEBUG_PRINT(" avec ID ");
  DEBUG_PRINT(deviceId);
  DEBUG_PRINT("...");
  
  // Créer un identifiant client unique basé sur l'ID attribué
  sprintf(clientId, "arduino-%d", deviceId);
  
  // Configuration de l'authentification MQTT si définie
  #ifdef MQTT_USER
    mqttClient.setUsernamePassword(MQTT_USER, MQTT_PASSWORD);
    DEBUG_PRINT(" (auth)");
  #endif
  
  // Tentative de connexion au broker
  mqttClient.setId(clientId);
  
  // Connexion directe au broker MQTT
  unsigned long startTime = millis();
  while (!mqttClient.connect(brokerIP, 1883) && (millis() - startTime) < 5000) {
    delay(500);
    DEBUG_PRINT(".");
  }
  
  if (mqttClient.connected()) {
    DEBUG_PRINTLN(" OK");
    return true;
  } else {
    DEBUG_PRINTLN(" ÉCHEC");
    DEBUG_PRINT("Code erreur MQTT: ");
    DEBUG_PRINTLN(mqttClient.connectError());
    return false;
  }
}

bool NetworkManager::sendMessage(const char* topic, const uint8_t* buffer, size_t length) {
  if (!mqttClient.connected()) {
    DEBUG_PRINTLN("MQTT non connecté pour envoi");
    return false;
  }
  
  // Démarrer le message
  mqttClient.beginMessage(topic, (unsigned int)length, false, 0);
  
  // Écrire les données binaires
  size_t bytesWritten = mqttClient.write(buffer, length);
  
  // Terminer et envoyer le message
  bool success = mqttClient.endMessage();
  
  if (success && bytesWritten == length) {
    DEBUG_PRINT("Message envoyé: ");
    DEBUG_PRINT(bytesWritten);
    DEBUG_PRINT(" octets sur topic ");
    DEBUG_PRINTLN(topic);
    return true;
  } else {
    DEBUG_PRINTLN("Échec d'envoi du message MQTT");
    return false;
  }
}

void NetworkManager::disconnect() {
  // Déconnexion MQTT propre
  if (mqttClient.connected()) {
    mqttClient.stop();
    DEBUG_PRINTLN("MQTT déconnecté");
  }
  
  // Déconnexion WiFi
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.end();
    DEBUG_PRINTLN("WiFi déconnecté");
  }
}

bool NetworkManager::isWiFiConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

bool NetworkManager::isMQTTConnected() {
  return mqttClient.connected();
}

bool NetworkManager::waitForWiFiConnection(unsigned long timeout) {
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
    delay(500);
    DEBUG_PRINT(".");
  }
  
  return (WiFi.status() == WL_CONNECTED);
}
