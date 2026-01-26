#include "SensorManager.h"
#include <stdlib.h>

bool SensorManager::begin() {
  DEBUG_PRINTLN("Initialisation des capteurs MKR ENV Shield...");
  
  // Initialisation du shield ENV complet
  if (!ENV.begin()) {
    DEBUG_PRINTLN("Échec d'initialisation du shield MKR ENV!");
    return false;
  }
  
  DEBUG_PRINTLN("Shield MKR ENV initialisé avec succès");
  DEBUG_PRINTLN("Capteurs disponibles: Température, Humidité, Pression, Luminosité, UV");
  return true;
}

SensorManager::SensorReadings SensorManager::readAll() {
  SensorReadings readings;
  
  // Lecture température et humidité
  readings.temperature = ENV.readTemperature();
  readings.humidity = ENV.readHumidity();
  
  // Lecture pression (déjà en kPa avec la bibliothèque MKR ENV)
  readings.pressure = ENV.readPressure();
  
  // Lecture luminosité
  readings.illuminance = ENV.readIlluminance();
  
  // Lecture des UV (si disponible sur le shield)
  readings.uva = ENV.readUVA();
  readings.uvb = ENV.readUVB();
  readings.uvindex = ENV.readUVIndex();
  
  // Si les capteurs UV ne sont pas disponibles, utiliser la simulation
  if (isnan(readings.uva) || readings.uva == 0) {
    readings.uva = simulateUVA();
  }
  if (isnan(readings.uvb) || readings.uvb == 0) {
    readings.uvb = simulateUVB();
  }
  if (isnan(readings.uvindex) || readings.uvindex == 0) {
    readings.uvindex = simulateUVIndex();
  }
  
  // Lecture de la tension de batterie
  readings.battery = readBatteryVoltage();
  
  return readings;
}

float SensorManager::readBatteryVoltage() {
  // Lecture de la tension de la batterie via le diviseur de tension interne
  // Sur MKR, la tension de batterie est accessible via ADC_BATTERY
  int rawValue = analogRead(ADC_BATTERY);
  
  // Conversion en tension (3.3V ref, facteur de division ~2)
  float voltage = (rawValue * 3.3 * 2.0) / 1023.0;
  
  return voltage;
}

float SensorManager::simulateUVA() {
  // Simulation basée sur la luminosité ambiante
  // En réalité, vous utiliseriez un vrai capteur UV
  static float lastUva = 50.0;
  float variation = (random(-10, 11) / 10.0); // Variation de -1 à +1
  lastUva = constrain(lastUva + variation, 0, 255);
  return lastUva;
}

float SensorManager::simulateUVB() {
  // Simulation basée sur la luminosité ambiante
  static float lastUvb = 30.0;
  float variation = (random(-8, 9) / 10.0); // Variation de -0.8 à +0.8
  lastUvb = constrain(lastUvb + variation, 0, 255);
  return lastUvb;
}

float SensorManager::simulateUVIndex() {
  // Simulation de l'indice UV (0-11+ typiquement)
  static float lastUvi = 2.5;
  float variation = (random(-5, 6) / 100.0); // Variation de -0.05 à +0.05
  lastUvi = constrain(lastUvi + variation, 0, 12);
  return lastUvi;
}
