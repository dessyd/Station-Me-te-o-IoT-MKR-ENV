#ifndef DATA_ENCODER_H
#define DATA_ENCODER_H

#include "Config.h"
#include "SensorManager.h"

class DataEncoder {
  public:
    // Encoder les données des capteurs en format binaire
    size_t encodeSensorData(uint8_t* buffer, size_t bufferSize, 
                          const SensorManager::SensorReadings& current,
                          const SensorManager::SensorReadings& last,
                          bool sendAll, uint8_t deviceId);
                          
    // Structure pour suivre quelles valeurs ont changé
    struct ChangedFlags {
      bool temperature;
      bool humidity;
      bool pressure;
      bool illuminance;
      bool uva;
      bool uvb;
      bool uvindex;
      bool battery;
    };
    
    // Déterminer quelles valeurs ont changé
    ChangedFlags detectChanges(const SensorManager::SensorReadings& current,
                               const SensorManager::SensorReadings& last);
                               
    // Générer le topic MQTT en utilisant l'ID
    void generateTopic(char* topicBuffer, size_t bufferSize, uint8_t deviceId);
    
    // Utilitaires de décodage pour debug
    void printEncodedMessage(const uint8_t* buffer, size_t length);
    
  private:
    // Fonctions d'encodage pour chaque type de donnée
    uint8_t encodeTemperature(float temperature);
    uint8_t encodeHumidity(float humidity);
    uint8_t encodePressure(float pressure);
    uint8_t encodeIlluminance(float illuminance);
    uint8_t encodeUV(float uv);
    uint8_t encodeUVIndex(float uvindex);
    uint8_t encodeBattery(float battery);
    
    // Vérification des plages de valeurs
    bool isValidTemperature(float temp);
    bool isValidHumidity(float hum);
    bool isValidPressure(float pres);
};

#endif
