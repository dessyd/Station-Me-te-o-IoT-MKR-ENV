#include "DataEncoder.h"

DataEncoder::ChangedFlags DataEncoder::detectChanges(const SensorManager::SensorReadings& current,
                                                  const SensorManager::SensorReadings& last) {
  ChangedFlags changes;
  
  changes.temperature = abs(current.temperature - last.temperature) >= TEMP_THRESHOLD;
  changes.humidity = abs(current.humidity - last.humidity) >= HUM_THRESHOLD;
  changes.pressure = abs(current.pressure - last.pressure) >= PRES_THRESHOLD;
  changes.illuminance = abs(current.illuminance - last.illuminance) >= LUX_THRESHOLD;
  changes.uva = abs(current.uva - last.uva) >= UV_THRESHOLD;
  changes.uvb = abs(current.uvb - last.uvb) >= UV_THRESHOLD;
  changes.uvindex = abs(current.uvindex - last.uvindex) >= UVI_THRESHOLD;
  changes.battery = abs(current.battery - last.battery) >= BAT_THRESHOLD;
  
  return changes;
}

size_t DataEncoder::encodeSensorData(uint8_t* buffer, size_t bufferSize, 
                                   const SensorManager::SensorReadings& current,
                                   const SensorManager::SensorReadings& last,
                                   bool sendAll, uint8_t deviceId) {
  
  if (bufferSize < 2) {
    DEBUG_PRINTLN("Buffer trop petit pour l'encodage");
    return 0;  // Buffer trop petit
  }
  
  ChangedFlags changes = detectChanges(current, last);
  
  // Créer les flags pour indiquer quelles valeurs sont incluses
  uint8_t flags = 0;
  if (sendAll || changes.temperature) flags |= 0x01;
  if (sendAll || changes.humidity)    flags |= 0x02;
  if (sendAll || changes.pressure)    flags |= 0x04;
  if (sendAll || changes.illuminance) flags |= 0x08;
  if (sendAll || changes.uva)         flags |= 0x10;
  if (sendAll || changes.uvb)         flags |= 0x20;
  if (sendAll || changes.uvindex)     flags |= 0x40;
  if (sendAll || changes.battery)     flags |= 0x80;
  
  // Si rien à envoyer, sortir
  if (flags == 0) {
    DEBUG_PRINTLN("Aucun changement détecté, rien à encoder");
    return 0;
  }
  
  size_t bufferPos = 0;
  
  // En-tête: ID de l'appareil et flags
  buffer[bufferPos++] = deviceId;
  buffer[bufferPos++] = flags;
  
  // Ajouter les valeurs selon les flags (vérifier la taille du buffer)
  if ((flags & 0x01) && bufferPos < bufferSize) {
    buffer[bufferPos++] = encodeTemperature(current.temperature);
    DEBUG_PRINT("Temp encodée: ");
    DEBUG_PRINTLN(current.temperature);
  }
  
  if ((flags & 0x02) && bufferPos < bufferSize) {
    buffer[bufferPos++] = encodeHumidity(current.humidity);
    DEBUG_PRINT("Hum encodée: ");
    DEBUG_PRINTLN(current.humidity);
  }
  
  if ((flags & 0x04) && bufferPos < bufferSize) {
    buffer[bufferPos++] = encodePressure(current.pressure);
    DEBUG_PRINT("Pres encodée: ");
    DEBUG_PRINTLN(current.pressure);
  }
  
  if ((flags & 0x08) && bufferPos < bufferSize) {
    buffer[bufferPos++] = encodeIlluminance(current.illuminance);
    DEBUG_PRINT("Lux encodée: ");
    DEBUG_PRINTLN(current.illuminance);
  }
  
  if ((flags & 0x10) && bufferPos < bufferSize) {
    buffer[bufferPos++] = encodeUV(current.uva);
    DEBUG_PRINT("UVA encodée: ");
    DEBUG_PRINTLN(current.uva);
  }
  
  if ((flags & 0x20) && bufferPos < bufferSize) {
    buffer[bufferPos++] = encodeUV(current.uvb);
    DEBUG_PRINT("UVB encodée: ");
    DEBUG_PRINTLN(current.uvb);
  }
  
  if ((flags & 0x40) && bufferPos < bufferSize) {
    buffer[bufferPos++] = encodeUVIndex(current.uvindex);
    DEBUG_PRINT("UVI encodé: ");
    DEBUG_PRINTLN(current.uvindex);
  }
  
  if ((flags & 0x80) && bufferPos < bufferSize) {
    buffer[bufferPos++] = encodeBattery(current.battery);
    DEBUG_PRINT("Bat encodée: ");
    DEBUG_PRINTLN(current.battery);
  }
  
  DEBUG_PRINT("Message encodé: ");
  DEBUG_PRINT(bufferPos);
  DEBUG_PRINTLN(" octets");
  
  return bufferPos;
}

void DataEncoder::generateTopic(char* topicBuffer, size_t bufferSize, uint8_t deviceId) {
  snprintf(topicBuffer, bufferSize, "sensors/%d/compact", deviceId);
}

void DataEncoder::printEncodedMessage(const uint8_t* buffer, size_t length) {
  DEBUG_PRINT("Message binaire (");
  DEBUG_PRINT(length);
  DEBUG_PRINT(" octets): ");
  
  for (size_t i = 0; i < length; i++) {
    if (buffer[i] < 16) DEBUG_PRINT("0");
    DEBUG_PRINT_HEX(buffer[i]);
    DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
}

// Fonctions d'encodage privées

uint8_t DataEncoder::encodeTemperature(float temperature) {
  // Température encodée: valeur * 10 (précision 0.1°C)
  // Plage: -25.5 à +25.5 °C
  if (!isValidTemperature(temperature)) {
    DEBUG_PRINT("Température hors plage: ");
    DEBUG_PRINTLN(temperature);
    temperature = constrain(temperature, -25.5, 25.5);
  }
  
  if (temperature < 0) {
    return 256 + (int8_t)(temperature * 10);
  } else {
    return (uint8_t)(temperature * 10);
  }
}

uint8_t DataEncoder::encodeHumidity(float humidity) {
  // Humidité directe (0-100%)
  if (!isValidHumidity(humidity)) {
    DEBUG_PRINT("Humidité hors plage: ");
    DEBUG_PRINTLN(humidity);
    humidity = constrain(humidity, 0, 100);
  }
  return (uint8_t)humidity;
}

uint8_t DataEncoder::encodePressure(float pressure) {
  // Pression: (valeur - 90) * 10 pour couvrir 90-115.5 kPa
  if (!isValidPressure(pressure)) {
    DEBUG_PRINT("Pression hors plage: ");
    DEBUG_PRINTLN(pressure);
    pressure = constrain(pressure, 90, 115.5);
  }
  return (uint8_t)((pressure - 90) * 10);
}

uint8_t DataEncoder::encodeIlluminance(float illuminance) {
  // Luminosité: valeur / 10 pour couvrir 0-2550 lux
  illuminance = constrain(illuminance, 0, 2550);
  return (uint8_t)(illuminance / 10);
}

uint8_t DataEncoder::encodeUV(float uv) {
  // Valeurs UV directes (0-255)
  uv = constrain(uv, 0, 255);
  return (uint8_t)uv;
}

uint8_t DataEncoder::encodeUVIndex(float uvindex) {
  // Index UV: valeur * 100 pour couvrir 0-2.55
  uvindex = constrain(uvindex, 0, 2.55);
  return (uint8_t)(uvindex * 100);
}

uint8_t DataEncoder::encodeBattery(float battery) {
  // Batterie: (valeur - 2) * 100 pour couvrir 2-4.55V
  battery = constrain(battery, 2.0, 4.55);
  return (uint8_t)((battery - 2) * 100);
}

// Fonctions de validation

bool DataEncoder::isValidTemperature(float temp) {
  return (temp >= -25.5 && temp <= 25.5);
}

bool DataEncoder::isValidHumidity(float hum) {
  return (hum >= 0 && hum <= 100);
}

bool DataEncoder::isValidPressure(float pres) {
  return (pres >= 90 && pres <= 115.5);
}
