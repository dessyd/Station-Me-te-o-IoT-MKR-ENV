#include "StorageManager.h"

// Définition du stockage flash (DOIT être en dehors de la classe)
FlashStorage(flash_store, StoredData);

void StorageManager::saveData(const SensorManager::SensorReadings& lastReadings,
                            bool forceSendAll,
                            unsigned long lastSendTime) {
  
  StoredData dataToStore;
  
  // Copier les données
  dataToStore.lastReadings = lastReadings;
  dataToStore.forceSendAll = forceSendAll;
  dataToStore.lastSendTime = lastSendTime;
  dataToStore.valid = true;  // Marquer comme valide
  
  // Sauvegarder en mémoire flash
  flash_store.write(dataToStore);
  
  DEBUG_PRINTLN("Données sauvegardées en mémoire flash");
  DEBUG_PRINT("Temp: "); DEBUG_PRINT(lastReadings.temperature);
  DEBUG_PRINT(", Hum: "); DEBUG_PRINT(lastReadings.humidity);
  DEBUG_PRINT(", Force: "); DEBUG_PRINT(forceSendAll ? "Oui" : "Non");
  DEBUG_PRINT(", Time: "); DEBUG_PRINTLN(lastSendTime);
}

StoredData StorageManager::loadData() {
  StoredData loadedData = flash_store.read();
  
  // Vérifier la validité des données
  if (!validateStoredData(loadedData)) {
    DEBUG_PRINTLN("Données flash invalides, utilisation des valeurs par défaut");
    loadedData = getDefaultData();
  } else {
    DEBUG_PRINTLN("Données chargées de la mémoire flash");
    DEBUG_PRINT("Temp: "); DEBUG_PRINT(loadedData.lastReadings.temperature);
    DEBUG_PRINT(", Hum: "); DEBUG_PRINT(loadedData.lastReadings.humidity);
    DEBUG_PRINT(", Force: "); DEBUG_PRINT(loadedData.forceSendAll ? "Oui" : "Non");
    DEBUG_PRINT(", Time: "); DEBUG_PRINTLN(loadedData.lastSendTime);
  }
  
  return loadedData;
}

bool StorageManager::isDataValid() {
  StoredData data = flash_store.read();
  return validateStoredData(data);
}

void StorageManager::resetData() {
  StoredData defaultData = getDefaultData();
  defaultData.valid = false;  // Marquer comme invalide
  
  flash_store.write(defaultData);
  
  DEBUG_PRINTLN("Données flash réinitialisées");
}

size_t StorageManager::getStoredDataSize() {
  return sizeof(StoredData);
}

bool StorageManager::validateStoredData(const StoredData& data) {
  // Vérifier le flag de validité
  if (!data.valid) {
    return false;
  }
  
  // Vérifier les plages de valeurs des capteurs
  const SensorManager::SensorReadings& readings = data.lastReadings;
  
  // Température entre -50 et +80°C (plage élargie pour la validité)
  if (readings.temperature < -50.0 || readings.temperature > 80.0) {
    DEBUG_PRINT("Température invalide: "); DEBUG_PRINTLN(readings.temperature);
    return false;
  }
  
  // Humidité entre 0 et 100%
  if (readings.humidity < 0.0 || readings.humidity > 100.0) {
    DEBUG_PRINT("Humidité invalide: "); DEBUG_PRINTLN(readings.humidity);
    return false;
  }
  
  // Pression entre 50 et 150 kPa (plage élargie)
  if (readings.pressure < 50.0 || readings.pressure > 150.0) {
    DEBUG_PRINT("Pression invalide: "); DEBUG_PRINTLN(readings.pressure);
    return false;
  }
  
  // Luminosité positive
  if (readings.illuminance < 0.0) {
    DEBUG_PRINT("Luminosité invalide: "); DEBUG_PRINTLN(readings.illuminance);
    return false;
  }
  
  // Valeurs UV entre 0 et 300 (plage élargie)
  if (readings.uva < 0.0 || readings.uva > 300.0 ||
      readings.uvb < 0.0 || readings.uvb > 300.0 ||
      readings.uvindex < 0.0 || readings.uvindex > 15.0) {
    DEBUG_PRINTLN("Valeurs UV invalides");
    return false;
  }
  
  // Tension batterie entre 1.5V et 5V
  if (readings.battery < 1.5 || readings.battery > 5.0) {
    DEBUG_PRINT("Tension batterie invalide: "); DEBUG_PRINTLN(readings.battery);
    return false;
  }
  
  // Vérifier que lastSendTime n'est pas dans le futur (avec une marge)
  unsigned long currentTime = millis();
  if (data.lastSendTime > currentTime + 3600000) { // Marge d'1 heure
    DEBUG_PRINT("Temps d'envoi futur détecté: "); DEBUG_PRINTLN(data.lastSendTime);
    return false;
  }
  
  return true;
}

StoredData StorageManager::getDefaultData() {
  StoredData defaultData;
  
  // Valeurs par défaut pour les lectures
  defaultData.lastReadings.temperature = 20.0;
  defaultData.lastReadings.humidity = 50.0;
  defaultData.lastReadings.pressure = 101.3;
  defaultData.lastReadings.illuminance = 100.0;
  defaultData.lastReadings.uva = 50.0;
  defaultData.lastReadings.uvb = 30.0;
  defaultData.lastReadings.uvindex = 2.0;
  defaultData.lastReadings.battery = 3.7;
  
  // État par défaut
  defaultData.forceSendAll = true;  // Forcer l'envoi complet au premier démarrage
  defaultData.lastSendTime = 0;     // Jamais envoyé
  defaultData.valid = false;        // Marqué comme invalide jusqu'à la première utilisation
  
  return defaultData;
}
