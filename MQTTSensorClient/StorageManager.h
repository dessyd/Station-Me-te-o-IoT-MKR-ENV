#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <FlashStorage.h>
#include "Config.h"
#include "SensorManager.h"

// Déclaration forward du stockage flash
class StorageManager;
typedef struct {
  SensorManager::SensorReadings lastReadings;
  bool forceSendAll;
  unsigned long lastSendTime;
  bool valid;
} StoredData;

// Déclaration externe du stockage flash
extern FlashStorageClass<StoredData> flash_store;

class StorageManager {
  public:
    // Utilisation de la structure StoredData définie globalement
    // (pas de typedef nécessaire)
    
    // Constructeur
    StorageManager();
    
    // Sauvegarder les données en mémoire flash
    void saveData(const SensorManager::SensorReadings& lastReadings,
                  bool forceSendAll,
                  unsigned long lastSendTime);
    
    // Charger les données de la mémoire flash
    StoredData loadData();
    
    // Vérifier si les données sont valides
    bool isDataValid();
    
    // Réinitialiser les données stockées
    void resetData();
    
    // Obtenir la taille des données stockées
    size_t getStoredDataSize();
    
  private:
    // Déclaration externe du stockage flash (défini dans le .cpp)
    // FlashStorage est défini globalement dans StorageManager.cpp
    
    // Vérification de l'intégrité des données
    bool validateStoredData(const StoredData& data);
    
    // Données par défaut
    StoredData getDefaultData();
};

#endif
