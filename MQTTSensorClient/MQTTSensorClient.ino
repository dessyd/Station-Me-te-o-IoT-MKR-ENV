#include <ArduinoLowPower.h>
#include "Config.h"
#include "SensorManager.h"
#include "NetworkManager.h"
#include "DataEncoder.h"
#include "StorageManager.h"
#include "ProvisionManager.h"

// ===================================================================
// DÉCLARATIONS DE FONCTIONS
// ===================================================================

// Fonctions utilitaires
bool transmitData(const SensorManager::SensorReadings& currentReadings, unsigned long currentTime);
unsigned long calculateTimeSinceLastSend(unsigned long currentTime);
void printSensorReadings(const SensorManager::SensorReadings& readings);
void printSystemStatus(unsigned long currentTime, unsigned long timeSinceLastSend);
void signalError();
void printMemoryInfo();
void testAllSensors();

// ===================================================================
// STATION MÉTÉO IoT - MKR ENV SHIELD
// ===================================================================
// Architecture modulaire avec provisionnement automatique
// Optimisée pour l'économie d'énergie et la transmission efficace
// ===================================================================

// Instances des classes (gestionnaires de modules)
SensorManager sensorManager;
NetworkManager networkManager;
DataEncoder dataEncoder;
StorageManager storageManager;
ProvisionManager provisionManager;

// Variables d'état globales
SensorManager::SensorReadings lastReadings;
bool forceSendAll = true;
unsigned long lastSendTime = 0;
bool firstBoot = true;
unsigned long lastWiFiCheckTime = 0;
unsigned long lastStatusPrintTime = 0;

// Constantes de timing
const unsigned long WIFI_CHECK_INTERVAL = 5000;      // Vérification WiFi toutes les 5s
const unsigned long STATUS_PRINT_INTERVAL = 60000;   // Affichage statut toutes les 60s

void setup() {
  // ===============================
  // INITIALISATION SYSTÈME
  // ===============================
  
  // Initialisation des communications série (pour debug)
  DEBUG_INIT();
  DEBUG_WAIT();
  
  // Message de démarrage
  DEBUG_PRINTLN("\n");
  DEBUG_PRINTLN("==================================================");
  DEBUG_PRINTLN("    STATION MÉTÉO IoT - MKR ENV SHIELD");
  DEBUG_PRINTLN("    Version: 2.0 - Architecture modulaire");
  DEBUG_PRINTLN("    Fonctionnalités: Deep Sleep + Provisionnement");
  DEBUG_PRINTLN("==================================================");
  DEBUG_PRINTLN();
  
  // ===============================
  // INITIALISATION DES MODULES
  // ===============================
  
  // 1. Initialisation des capteurs environnementaux
  DEBUG_PRINTLN("1. Initialisation des capteurs...");
  if (!sensorManager.begin()) {
    DEBUG_PRINTLN("ERREUR: Échec d'initialisation des capteurs!");
    signalError();
    return;
  }
  DEBUG_PRINTLN("   ✓ Capteurs initialisés avec succès");
  
  // 2. Initialisation du système de provisionnement
  DEBUG_PRINTLN("2. Initialisation du provisionnement...");
  if (!provisionManager.begin()) {
    DEBUG_PRINTLN("ERREUR: Échec d'initialisation du provisionnement!");
    signalError();
    return;
  }
  DEBUG_PRINTLN("   ✓ Système de provisionnement initialisé");
  
  // ===============================
  // CONNEXION RÉSEAU INITIALE
  // ===============================
  
  // 3. Connexion WiFi initiale
  DEBUG_PRINTLN("3. Connexion WiFi initiale...");
  if (networkManager.connectWiFi()) {
    DEBUG_PRINTLN("   ✓ WiFi connecté");
    
    // 4. Première tentative de provisionnement
    DEBUG_PRINTLN("4. Tentative de provisionnement...");
    provisionManager.requestID();
    
    if (provisionManager.isProvisioned()) {
      DEBUG_PRINTLN("   ✓ Provisionnement réussi");
    } else {
      DEBUG_PRINTLN("   ⚠ Provisionnement échoué (retentatives programmées)");
    }
  } else {
    DEBUG_PRINTLN("   ⚠ Connexion WiFi échouée (retentatives programmées)");
  }
  
  // ===============================
  // CHARGEMENT DE L'ÉTAT PERSISTANT
  // ===============================
  
  // 5. Chargement des données sauvegardées
  DEBUG_PRINTLN("5. Chargement de l'état persistant...");
  if (storageManager.isDataValid()) {
    StoredData storedData = storageManager.loadData();
    lastReadings = storedData.lastReadings;
    forceSendAll = storedData.forceSendAll;
    lastSendTime = storedData.lastSendTime;
    DEBUG_PRINTLN("   ✓ État restauré depuis la mémoire flash");
  } else {
    DEBUG_PRINTLN("   ⚠ Mémoire flash vide, initialisation par défaut");
    forceSendAll = true;
    lastSendTime = 0;
  }
  
  // ===============================
  // FINALISATION SETUP
  // ===============================
  
  DEBUG_PRINTLN("6. Configuration terminée");
  DEBUG_PRINT("   Taille données stockées: ");
  DEBUG_PRINT(storageManager.getStoredDataSize());
  DEBUG_PRINTLN(" octets");
  
  // Afficher le statut initial
  provisionManager.printStatus();
  
  // Marquer la fin du premier boot
  firstBoot = true;
  
  DEBUG_PRINTLN("==================================================");
  DEBUG_PRINTLN("Setup terminé - Début du cycle principal");
  DEBUG_PRINTLN("==================================================\n");
}

void loop() {
  // ===============================
  // GESTION DU RÉVEIL
  // ===============================
  
  // Réinitialisation du port série après réveil (si nécessaire)
  if (!firstBoot) {
    DEBUG_INIT();
    DEBUG_WAIT();
    DEBUG_PRINTLN("\n--- RÉVEIL du Deep Sleep ---");
  } else {
    firstBoot = false;
  }
  
  unsigned long currentTime = millis();
  
  // ===============================
  // MAINTENANCE RÉSEAU
  // ===============================
  
  // Vérification périodique de la connexion WiFi
  if (currentTime - lastWiFiCheckTime >= WIFI_CHECK_INTERVAL) {
    lastWiFiCheckTime = currentTime;
    
    if (!networkManager.isWiFiConnected()) {
      DEBUG_PRINTLN("Reconnexion WiFi nécessaire...");
      networkManager.connectWiFi();
    }
  }
  
  // Mise à jour du provisionnement (retentatives si nécessaire)
  provisionManager.update();
  
  // ===============================
  // ACQUISITION DES DONNÉES
  // ===============================
  
  DEBUG_PRINTLN("Lecture des capteurs...");
  SensorManager::SensorReadings currentReadings = sensorManager.readAll();
  
  // Affichage des valeurs en mode debug
  printSensorReadings(currentReadings);
  
  // ===============================
  // ANALYSE DES CHANGEMENTS
  // ===============================
  
  // Vérifier s'il y a des changements significatifs
  bool anyChange = sensorManager.hasSignificantChanges(
    currentReadings.temperature, lastReadings.temperature,
    currentReadings.humidity, lastReadings.humidity,
    currentReadings.pressure, lastReadings.pressure,
    currentReadings.illuminance, lastReadings.illuminance,
    currentReadings.uva, lastReadings.uva,
    currentReadings.uvb, lastReadings.uvb,
    currentReadings.uvindex, lastReadings.uvindex,
    currentReadings.battery, lastReadings.battery
  );
  
  // Calculer le temps écoulé depuis le dernier envoi
  unsigned long timeSinceLastSend = calculateTimeSinceLastSend(currentTime);
  
  // Forcer l'envoi complet si ça fait longtemps qu'on n'a rien envoyé
  if (timeSinceLastSend >= MAX_NO_SEND_INTERVAL) {
    forceSendAll = true;
    DEBUG_PRINT("Intervalle maximal dépassé (");
    DEBUG_PRINT(timeSinceLastSend / 1000);
    DEBUG_PRINTLN("s), envoi forcé");
  }
  
  // ===============================
  // TRANSMISSION DES DONNÉES
  // ===============================
  
  // Décider s'il faut envoyer les données
  bool shouldSend = (anyChange || forceSendAll) && provisionManager.isProvisioned();
  
  if (shouldSend) {
    DEBUG_PRINTLN("--- DÉBUT TRANSMISSION ---");
    
    if (transmitData(currentReadings, currentTime)) {
      DEBUG_PRINTLN("--- TRANSMISSION RÉUSSIE ---");
      
      // Mettre à jour l'état après succès
      lastReadings = currentReadings;
      lastSendTime = currentTime;
      forceSendAll = false;
      
      // Sauvegarder le nouvel état
      storageManager.saveData(lastReadings, forceSendAll, lastSendTime);
      
    } else {
      DEBUG_PRINTLN("--- TRANSMISSION ÉCHOUÉE ---");
      // En cas d'échec, on garde l'ancien état et on retentera
    }
    
  } else {
    // Expliquer pourquoi on n'envoie pas
    if (!provisionManager.isProvisioned()) {
      DEBUG_PRINTLN("⚠ Appareil non provisionné, transmission impossible");
    } else if (!anyChange && !forceSendAll) {
      DEBUG_PRINTLN("ℹ Aucun changement significatif détecté");
    }
  }
  
  // ===============================
  // AFFICHAGE PÉRIODIQUE DU STATUT
  // ===============================
  
  if (currentTime - lastStatusPrintTime >= STATUS_PRINT_INTERVAL) {
    lastStatusPrintTime = currentTime;
    printSystemStatus(currentTime, timeSinceLastSend);
  }
  
  // ===============================
  // PRÉPARATION AU SOMMEIL
  // ===============================
  
  DEBUG_PRINT("Passage en Deep Sleep pour ");
  DEBUG_PRINT(SLEEP_TIME / 1000);
  DEBUG_PRINTLN(" secondes...");
  
  // S'assurer que tous les messages série sont envoyés
  Serial.flush();
  delay(100);
  
  // Entrer en mode deep sleep pour économiser l'énergie
  LowPower.deepSleep(SLEEP_TIME);
  
  // Le code continuera ici après le réveil automatique
}

// ===================================================================
// FONCTIONS UTILITAIRES
// ===================================================================

bool transmitData(const SensorManager::SensorReadings& currentReadings, unsigned long currentTime) {
  // Obtenir les informations de provisionnement
  uint8_t deviceId = provisionManager.getDeviceID();
  const char* mqttBroker = provisionManager.getMQTTBroker();
  
  DEBUG_PRINT("Transmission avec ID ");
  DEBUG_PRINT(deviceId);
  DEBUG_PRINT(" vers ");
  DEBUG_PRINTLN(mqttBroker);
  
  // Préparer le buffer pour l'encodage
  uint8_t buffer[10];  // Taille max: 2 octets d'en-tête + 8 octets de données
  size_t length = dataEncoder.encodeSensorData(buffer, sizeof(buffer), 
                                             currentReadings, lastReadings, 
                                             forceSendAll, deviceId);
  
  if (length == 0) {
    DEBUG_PRINTLN("Aucune donnée à envoyer après encodage");
    return false;
  }
  
  // Générer le topic MQTT
  char topic[30];
  dataEncoder.generateTopic(topic, sizeof(topic), deviceId);
  
  // Afficher le message encodé pour debug
  dataEncoder.printEncodedMessage(buffer, length);
  
  // Établir les connexions réseau
  bool success = false;
  
  if (networkManager.connectWiFi()) {
    if (networkManager.connectMQTT(mqttBroker, deviceId)) {
      // Envoyer le message
      if (networkManager.sendMessage(topic, buffer, length)) {
        DEBUG_PRINTLN("✓ Message MQTT envoyé avec succès");
        success = true;
      } else {
        DEBUG_PRINTLN("✗ Échec d'envoi du message MQTT");
      }
    } else {
      DEBUG_PRINTLN("✗ Échec de connexion MQTT");
    }
    
    // Déconnexion propre pour économiser l'énergie
    networkManager.disconnect();
  } else {
    DEBUG_PRINTLN("✗ Échec de connexion WiFi");
  }
  
  return success;
}

unsigned long calculateTimeSinceLastSend(unsigned long currentTime) {
  if (lastSendTime == 0) {
    return MAX_NO_SEND_INTERVAL + 1; // Forcer l'envoi si jamais envoyé
  }
  
  // Gérer le cas du débordement de millis() (après ~49 jours)
  if (currentTime < lastSendTime) {
    return currentTime + (0xFFFFFFFF - lastSendTime);
  } else {
    return currentTime - lastSendTime;
  }
}

void printSensorReadings(const SensorManager::SensorReadings& readings) {
  DEBUG_PRINTLN("┌─────────────────────────────────────┐");
  DEBUG_PRINTLN("│        LECTURES CAPTEURS            │");
  DEBUG_PRINTLN("├─────────────────────────────────────┤");
  
  DEBUG_PRINT("│ Température : ");
  DEBUG_PRINT(readings.temperature);
  DEBUG_PRINTLN(" °C");
  
  DEBUG_PRINT("│ Humidité    : ");
  DEBUG_PRINT(readings.humidity);
  DEBUG_PRINTLN(" %");
  
  DEBUG_PRINT("│ Pression    : ");
  DEBUG_PRINT(readings.pressure);
  DEBUG_PRINTLN(" kPa");
  
  DEBUG_PRINT("│ Luminosité  : ");
  DEBUG_PRINT(readings.illuminance);
  DEBUG_PRINTLN(" lux");
  
  DEBUG_PRINT("│ UVA         : ");
  DEBUG_PRINT(readings.uva);
  DEBUG_PRINTLN("");
  
  DEBUG_PRINT("│ UVB         : ");
  DEBUG_PRINT(readings.uvb);
  DEBUG_PRINTLN("");
  
  DEBUG_PRINT("│ Index UV    : ");
  DEBUG_PRINT(readings.uvindex);
  DEBUG_PRINTLN("");
  
  DEBUG_PRINT("│ Batterie    : ");
  DEBUG_PRINT(readings.battery);
  DEBUG_PRINTLN(" V");
  
  DEBUG_PRINTLN("└─────────────────────────────────────┘");
}

void printSystemStatus(unsigned long currentTime, unsigned long timeSinceLastSend) {
  DEBUG_PRINTLN("\n┌═══════════════════════════════════════┐");
  DEBUG_PRINTLN("│           STATUT SYSTÈME              │");
  DEBUG_PRINTLN("├───────────────────────────────────────┤");
  
  // État du provisionnement
  DEBUG_PRINT("│ Provisionné    : ");
  DEBUG_PRINTLN(provisionManager.isProvisioned() ? "✓ Oui" : "✗ Non");
  
  if (provisionManager.isProvisioned()) {
    DEBUG_PRINT("│ ID Appareil    : ");
    DEBUG_PRINTLN(provisionManager.getDeviceID());
    
    DEBUG_PRINT("│ Broker MQTT    : ");
    DEBUG_PRINTLN(provisionManager.getMQTTBroker());
  }
  
  // État réseau
  DEBUG_PRINT("│ WiFi           : ");
  DEBUG_PRINTLN(networkManager.isWiFiConnected() ? "✓ Connecté" : "✗ Déconnecté");
  
  // Timing
  DEBUG_PRINT("│ Uptime         : ");
  DEBUG_PRINT(currentTime / 1000);
  DEBUG_PRINTLN(" s");
  
  DEBUG_PRINT("│ Dernier envoi  : ");
  if (lastSendTime == 0) {
    DEBUG_PRINTLN("Jamais");
  } else {
    DEBUG_PRINT(timeSinceLastSend / 1000);
    DEBUG_PRINTLN(" s");
  }
  
  // État mémoire
  DEBUG_PRINT("│ Données flash  : ");
  DEBUG_PRINTLN(storageManager.isDataValid() ? "✓ Valides" : "✗ Invalides");
  
  // Mode de transmission
  DEBUG_PRINT("│ Mode envoi     : ");
  DEBUG_PRINTLN(forceSendAll ? "Complet" : "Différentiel");
  
  DEBUG_PRINTLN("└═══════════════════════════════════════┘\n");
}

void signalError() {
  // Clignotement de la LED pour signaler une erreur
  pinMode(LED_BUILTIN, OUTPUT);
  
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
  
  DEBUG_PRINTLN("Erreur critique - Redémarrage nécessaire");
  
  // En cas d'erreur critique, on peut soit redémarrer, soit passer en deep sleep
  // Ici on passe en deep sleep long pour économiser la batterie
  LowPower.deepSleep(60000); // 1 minute avant retry
}

// ===================================================================
// FONCTIONS DE DIAGNOSTIC (optionnelles)
// ===================================================================

void printMemoryInfo() {
  // Cette fonction peut être appelée pour diagnostiquer l'usage mémoire
  DEBUG_PRINTLN("=== DIAGNOSTIC MÉMOIRE ===");
  DEBUG_PRINT("Taille SensorReadings: ");
  DEBUG_PRINT(sizeof(SensorManager::SensorReadings));
  DEBUG_PRINTLN(" octets");
  
  DEBUG_PRINT("Taille StoredData: ");
  DEBUG_PRINT(sizeof(StoredData));
  DEBUG_PRINTLN(" octets");
  
  DEBUG_PRINT("Variables globales: ~");
  DEBUG_PRINT(sizeof(lastReadings) + sizeof(forceSendAll) + sizeof(lastSendTime) + sizeof(firstBoot));
  DEBUG_PRINTLN(" octets");
  
  DEBUG_PRINTLN("========================");
}

void testAllSensors() {
  // Fonction de test pour vérifier tous les capteurs
  DEBUG_PRINTLN("=== TEST DES CAPTEURS ===");
  
  for (int i = 0; i < 5; i++) {
    SensorManager::SensorReadings test = sensorManager.readAll();
    
    DEBUG_PRINT("Test ");
    DEBUG_PRINT(i + 1);
    DEBUG_PRINT(": T=");
    DEBUG_PRINT(test.temperature);
    DEBUG_PRINT("°C, H=");
    DEBUG_PRINT(test.humidity);
    DEBUG_PRINT("%, P=");
    DEBUG_PRINT(test.pressure);
    DEBUG_PRINT("kPa, L=");
    DEBUG_PRINT(test.illuminance);
    DEBUG_PRINT("lux, B=");
    DEBUG_PRINT(test.battery);
    DEBUG_PRINTLN("V");
    
    delay(1000);
  }
  
  DEBUG_PRINTLN("=====================");
}
