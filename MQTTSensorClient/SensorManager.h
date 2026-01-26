#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino_MKRENV.h>
#include "Config.h"

class SensorManager {
  public:
    // Structure pour regrouper toutes les lectures de capteurs
    struct SensorReadings {
      float temperature;
      float humidity;
      float pressure;
      float illuminance;
      float uva;
      float uvb;
      float uvindex;
      float battery;
    };
    
    // Initialisation des capteurs
    bool begin();
    
    // Lecture de tous les capteurs
    SensorReadings readAll();

  private:
    // Lecture de la tension de la batterie
    float readBatteryVoltage();
    
    // Variables pour les lectures UV simulées (si pas disponibles sur le shield)
    float simulateUVA();
    float simulateUVB();
    float simulateUVIndex();
};

#endif
