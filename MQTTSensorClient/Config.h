#ifndef CONFIG_H
#define CONFIG_H

// Inclusion des secrets (informations sensibles)
#include "arduino_secrets.h"

// ===================================================================
// CONFIGURATION PUBLIQUE - PARAMÈTRES SYSTÈME
// ===================================================================
// Ce fichier contient les paramètres de configuration non sensibles
// qui peuvent être partagés publiquement
// ===================================================================

// Informations d'authentification WiFi (depuis arduino_secrets.h)
#define WIFI_SSID SECRET_SSID
#define WIFI_PASSWORD SECRET_PASSWORD

// Paramètres de provisionnement
#define PROVISION_SERVER_IP SECRET_PROVISIONNING_IP  // Depuis arduino_secrets.h
#define PROVISION_SERVER_PORT 5000                   // Port du serveur de provisionnement

// Configuration de veille
#define SLEEP_TIME 300000  // Durée du deep sleep en ms (5 minutes)

// Seuils de changement pour envoi des données
#define TEMP_THRESHOLD 0.5   // °C
#define HUM_THRESHOLD 2.0    // %
#define PRES_THRESHOLD 0.2   // kPa
#define LUX_THRESHOLD 10.0   // lx
#define UV_THRESHOLD 1.0     // unité
#define UVI_THRESHOLD 0.05   // indice
#define BAT_THRESHOLD 0.1    // V

// Intervalle maximal entre deux envois complets (en ms)
#define MAX_NO_SEND_INTERVAL 3600000  // 1 heure

// Configuration MQTT optionnelle (depuis arduino_secrets.h si défini)
#ifdef SECRET_MQTT_USER
  #define MQTT_USER SECRET_MQTT_USER
  #define MQTT_PASSWORD SECRET_MQTT_PASSWORD
#endif
// Sinon, décommentez et configurez manuellement ci-dessous:
// #define MQTT_USER "mqtt_username"
// #define MQTT_PASSWORD "mqtt_password"

// Activation du mode débogage
#define DEBUG_ENABLED  // Commenter cette ligne pour désactiver le débogage

// Macros de débogage
#ifdef DEBUG_ENABLED
  #define DEBUG_INIT() Serial.begin(9600)
  #define DEBUG_WAIT() do { unsigned long _start = millis(); while(!Serial && millis() - _start < 3000); } while(0)
  #define DEBUG_PRINT(x) do { if(Serial) Serial.print(x); } while(0)
  #define DEBUG_PRINTLN(x) do { if(Serial) Serial.println(x); } while(0)
  #define DEBUG_PRINT_HEX(x) do { if(Serial) Serial.print(x, HEX); } while(0)
#else
  #define DEBUG_INIT()
  #define DEBUG_WAIT()
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT_HEX(x)
#endif

#endif
