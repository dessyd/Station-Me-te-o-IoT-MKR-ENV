#include "ProvisionManager.h"

ProvisionManager::ProvisionManager() {
  // Initialiser les adresses vides
  memset(macAddress, 0, sizeof(macAddress));
  memset(mqttBrokerIP, 0, sizeof(mqttBrokerIP));
}

bool ProvisionManager::begin() {
  DEBUG_PRINTLN("Initialisation du système de provisionnement...");
  
  // Initialiser la puce ECCX08
  if (!ECCX08.begin()) {
    DEBUG_PRINTLN("Échec d'initialisation de la puce ECCX08!");
    return false;
  }
  DEBUG_PRINTLN("Puce ECCX08 initialisée");
  
  // Initialiser UDP sur le port 8888
  udp.begin(8888);
  DEBUG_PRINTLN("UDP initialisé sur port 8888");
  
  // Obtenir et formater l'adresse MAC
  initializeMACAddress();
  
  return true;
}

bool ProvisionManager::requestID() {
  if (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINTLN("Pas de connexion WiFi pour la demande de provisionnement");
    return false;
  }
  
  DEBUG_PRINTLN("=== Début du processus de provisionnement ===");
  DEBUG_PRINT("Adresse MAC: "); DEBUG_PRINTLN(macAddress);
  DEBUG_PRINT("Serveur: "); DEBUG_PRINT(PROVISION_SERVER_IP);
  DEBUG_PRINT(":"); DEBUG_PRINTLN(PROVISION_SERVER_PORT);
  
  for (int retry = 0; retry < MAX_RETRIES; retry++) {
    DEBUG_PRINT("Tentative ");
    DEBUG_PRINT(retry + 1);
    DEBUG_PRINT("/");
    DEBUG_PRINT(MAX_RETRIES);
    DEBUG_PRINTLN(" de provisionnement...");
    
    // Générer un token aléatoire sécurisé avec ECCX08
    byte randomToken[4];
    if (!ECCX08.random(randomToken, sizeof(randomToken))) {
      DEBUG_PRINTLN("Échec de génération du token aléatoire");
      continue;
    }
    
    DEBUG_PRINT("Token généré: ");
    for (int i = 0; i < 4; i++) {
      DEBUG_PRINT_HEX(randomToken[i]);
      DEBUG_PRINT(" ");
    }
    DEBUG_PRINTLN();
    
    // Envoyer la demande
    if (sendProvisionRequest(randomToken)) {
      // Attendre et analyser la réponse
      if (parseProvisionResponse(randomToken)) {
        DEBUG_PRINTLN("=== Provisionnement réussi ===");
        DEBUG_PRINT("ID attribué: "); DEBUG_PRINTLN(deviceID);
        DEBUG_PRINT("Broker MQTT: "); DEBUG_PRINTLN(mqttBrokerIP);
        idReceived = true;
        return true;
      }
    }
    
    DEBUG_PRINT("Échec tentative ");
    DEBUG_PRINT(retry + 1);
    DEBUG_PRINTLN(", attente avant retry...");
    delay(1000); // Attente entre les tentatives
  }
  
  DEBUG_PRINTLN("=== Échec du provisionnement après toutes les tentatives ===");
  return false;
}

bool ProvisionManager::isProvisioned() {
  return idReceived;
}

uint8_t ProvisionManager::getDeviceID() {
  return deviceID;
}

const char* ProvisionManager::getMQTTBroker() {
  return mqttBrokerIP;
}

const char* ProvisionManager::getMACAddress() {
  return macAddress;
}

void ProvisionManager::update() {
  if (!idReceived) {
    unsigned long currentTime = millis();
    
    // Vérifier si c'est le moment de réessayer
    if (currentTime - lastRetryTime >= RETRY_INTERVAL) {
      lastRetryTime = currentTime;
      DEBUG_PRINTLN("Tentative périodique de provisionnement...");
      requestID();
    }
  }
}

void ProvisionManager::forceNewRequest() {
  DEBUG_PRINTLN("Forçage d'une nouvelle demande de provisionnement");
  idReceived = false;
  deviceID = 0;
  memset(mqttBrokerIP, 0, sizeof(mqttBrokerIP));
  lastRetryTime = 0;
}

void ProvisionManager::printStatus() {
  DEBUG_PRINTLN("=== État du provisionnement ===");
  DEBUG_PRINT("Adresse MAC: "); DEBUG_PRINTLN(macAddress);
  DEBUG_PRINT("Provisionné: "); DEBUG_PRINTLN(idReceived ? "Oui" : "Non");
  
  if (idReceived) {
    DEBUG_PRINT("ID Appareil: "); DEBUG_PRINTLN(deviceID);
    DEBUG_PRINT("Broker MQTT: "); DEBUG_PRINTLN(mqttBrokerIP);
  } else {
    DEBUG_PRINT("Prochaine tentative dans: ");
    unsigned long timeLeft = RETRY_INTERVAL - (millis() - lastRetryTime);
    DEBUG_PRINT(timeLeft / 1000); DEBUG_PRINTLN(" secondes");
  }
  DEBUG_PRINTLN("============================");
}

// Méthodes privées

bool ProvisionManager::sendProvisionRequest(byte* randomToken) {
  // Préparer le message: MAC + séparateur + token binaire
  int macLength = strlen(macAddress);
  uint8_t message[macLength + 1 + 4];  // MAC + ':' + token (4 octets)
  
  // Copier l'adresse MAC
  memcpy(message, macAddress, macLength);
  message[macLength] = ':';  // Séparateur
  
  // Ajouter le token binaire
  memcpy(&message[macLength + 1], randomToken, 4);
  
  // Envoyer via UDP
  udp.beginPacket(PROVISION_SERVER_IP, PROVISION_SERVER_PORT);
  size_t bytesWritten = udp.write(message, macLength + 1 + 4);
  udp.endPacket();
  
  DEBUG_PRINT("Demande envoyée: ");
  DEBUG_PRINT(bytesWritten);
  DEBUG_PRINTLN(" octets");
  
  return (bytesWritten == (macLength + 1 + 4));
}

bool ProvisionManager::parseProvisionResponse(byte* randomToken) {
  unsigned long startTime = millis();
  
  while (millis() - startTime < RETRY_TIMEOUT) {
    int packetSize = udp.parsePacket();
    
    if (packetSize > 0) {
      // Vérifier l'adresse IP source
      IPAddress remoteIP = udp.remoteIP();
      char remoteIPStr[16];
      sprintf(remoteIPStr, "%d.%d.%d.%d", remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3]);
      
      DEBUG_PRINT("Réponse reçue de: "); DEBUG_PRINTLN(remoteIPStr);
      DEBUG_PRINT("Taille: "); DEBUG_PRINT(packetSize); DEBUG_PRINTLN(" octets");
      
      if (strcmp(remoteIPStr, PROVISION_SERVER_IP) == 0) {
        // Format attendu: [ID:1byte][TOKEN:4bytes][MQTT_IP:variable]
        if (packetSize >= 5) { // Au minimum ID + TOKEN
          
          // Lire l'ID attribué
          uint8_t receivedID = udp.read();
          DEBUG_PRINT("ID reçu: "); DEBUG_PRINTLN(receivedID);
          
          // Lire le token retourné
          byte receivedToken[4];
          udp.read(receivedToken, 4);
          
          DEBUG_PRINT("Token retourné: ");
          for (int i = 0; i < 4; i++) {
            DEBUG_PRINT_HEX(receivedToken[i]);
            DEBUG_PRINT(" ");
          }
          DEBUG_PRINTLN();
          
          // Vérifier que le token correspond
          if (memcmp(receivedToken, randomToken, 4) == 0) {
            DEBUG_PRINTLN("Token validé ✓");
            
            // Lire l'adresse IP du broker MQTT
            int ipLength = packetSize - 5; // Soustraire ID (1) et token (4)
            if (ipLength > 0 && ipLength < 16) {
              char ipBuffer[16] = {0};
              udp.read((uint8_t*)ipBuffer, ipLength);
              ipBuffer[ipLength] = '\0';
              
              // Valider le format IP
              if (validateBrokerIP(ipBuffer)) {
                // Stocker les résultats
                deviceID = receivedID;
                strcpy(mqttBrokerIP, ipBuffer);
                
                DEBUG_PRINT("Broker MQTT validé: "); DEBUG_PRINTLN(mqttBrokerIP);
                return true;
              } else {
                DEBUG_PRINT("Format IP broker invalide: "); DEBUG_PRINTLN(ipBuffer);
              }
            } else {
              DEBUG_PRINT("Taille IP broker invalide: "); DEBUG_PRINTLN(ipLength);
            }
          } else {
            DEBUG_PRINTLN("Token incorrect, réponse ignorée ✗");
          }
        } else {
          DEBUG_PRINT("Taille de paquet insuffisante: "); DEBUG_PRINTLN(packetSize);
        }
      } else {
        DEBUG_PRINT("Réponse d'une IP inconnue ignorée: "); DEBUG_PRINTLN(remoteIPStr);
      }
    }
    
    delay(10); // Petite pause pour éviter la surcharge
  }
  
  DEBUG_PRINTLN("Timeout d'attente de réponse");
  return false;
}

void ProvisionManager::initializeMACAddress() {
  byte mac[6];
  WiFi.macAddress(mac);
  sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", 
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  
  DEBUG_PRINT("Adresse MAC initialisée: "); DEBUG_PRINTLN(macAddress);
}

bool ProvisionManager::validateBrokerIP(const char* ip) {
  // Validation basique du format IP
  int dots = 0;
  int currentNumber = 0;
  bool hasDigit = false;

  for (int i = 0; ip[i] != '\0'; i++) {
    char c = ip[i];

    if (c >= '0' && c <= '9') {
      currentNumber = currentNumber * 10 + (c - '0');
      hasDigit = true;

      // Vérifier que le nombre n'est pas trop grand
      if (currentNumber > 255) {
        return false;
      }
    } else if (c == '.') {
      if (!hasDigit || dots >= 3) {
        return false;
      }
      dots++;
      currentNumber = 0;
      hasDigit = false;
    } else {
      // Caractère invalide
      return false;
    }
  }

  return (dots == 3 && hasDigit);
}
