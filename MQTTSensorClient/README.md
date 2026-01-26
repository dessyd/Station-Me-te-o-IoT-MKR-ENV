# Station Météo IoT MKR ENV - Guide d'installation

Ce fichier README contient les instructions pour configurer et utiliser la station météo IoT basée sur Arduino MKR avec le shield ENV.

## Prérequis matériels

- Arduino MKR WiFi 1010 ou MKR Vidor 4000
- Arduino MKR ENV Shield
- Batterie Li-Po (optionnel pour alimentation autonome)
- Antenne WiFi (incluse avec MKR)

## Prérequis logiciels

### Arduino IDE

- Arduino IDE 1.8.19 ou plus récent
- Installation du support pour les cartes Arduino SAMD

### Bibliothèques requises

Installez les bibliothèques suivantes via le gestionnaire de bibliothèques Arduino :

```text
WiFiNINA              // Communication WiFi
ArduinoMqttClient     // Client MQTT
Arduino_MKRENV        // Shield MKR ENV (capteurs intégrés)
ArduinoECCX08         // Sécurité cryptographique
FlashStorage          // Stockage persistant
ArduinoLowPower       // Gestion du mode deep sleep
```

## Configuration

### 1. Configuration des secrets (arduino_secrets.h)

**IMPORTANT:** Le projet utilise un système de fichier de secrets séparé pour protéger vos informations sensibles.

1. **Copiez le fichier d'exemple :**

   ```bash
   cp arduino_secrets.h.example arduino_secrets.h
   ```

2. **Éditez arduino_secrets.h** avec vos vraies informations :

   ```cpp
   #define SECRET_SSID "VotreSSIDWiFi"
   #define SECRET_PASSWORD "VotreMotDePasseWiFi"
   #define SECRET_PROVISIONNING_IP "192.168.1.100"
   ```

3. **Vérifiez que arduino_secrets.h est dans .gitignore** (déjà fait)

> ⚠️ **Sécurité** : Ne commitez JAMAIS le fichier `arduino_secrets.h` dans un dépôt public !

### 2. Configuration des paramètres système (Config.h)

Les paramètres non sensibles peuvent être ajustés dans `Config.h` :

```cpp
// Seuils de détection de changements
#define TEMP_THRESHOLD 0.5   // °C
#define HUM_THRESHOLD 2.0    // %
#define PRES_THRESHOLD 0.2   // kPa

// Paramètres d'économie d'énergie
#define SLEEP_TIME 300000  // Durée du deep sleep (5 min)
#define MAX_NO_SEND_INTERVAL 3600000  // Envoi forcé (1h)
```

Vous devez mettre en place un serveur UDP qui répond aux requêtes de provisionnement.

### 3. Configuration du serveur de provisionnement

**Format de la requête** (envoyée par l'Arduino) :

```text
[Adresse MAC]:[Token 4 octets]
```

**Format de la réponse** (envoyée par le serveur) :

```text
[ID appareil 1 octet][Token 4 octets][Adresse IP MQTT variable]
```

### 4. Configuration MQTT (optionnel)

Si votre broker MQTT nécessite une authentification, ajoutez dans `arduino_secrets.h` :

```cpp
#define SECRET_MQTT_USER "votre_utilisateur_mqtt"
#define SECRET_MQTT_PASSWORD "votre_mot_de_passe_mqtt"
```

Ou décommentez dans `Config.h` :

```cpp
#define MQTT_USER "mqtt_username"
#define MQTT_PASSWORD "mqtt_password"
```

### 5. Ajustement des seuils de détection

Modifiez les seuils selon vos besoins dans `Config.h` :

```cpp
#define TEMP_THRESHOLD 0.5   // °C
#define HUM_THRESHOLD 2.0    // %
#define PRES_THRESHOLD 0.2   // kPa
// ... autres seuils
```

## Installation

1. **Télécharger les fichiers**
   - Copiez tous les fichiers .h et .cpp dans le même dossier
   - Le fichier .ino doit avoir le même nom que le dossier

2. **Ouvrir dans Arduino IDE**
   - Ouvrez le fichier MQTTSensorClient.ino
   - Vérifiez que tous les fichiers apparaissent dans les onglets

3. **Sélectionner la carte**
   - Outils → Type de carte → Arduino MKR WiFi 1010

4. **Compiler et téléverser**
   - Vérifier/Compiler (Ctrl+R)
   - Téléverser (Ctrl+U)

## Utilisation

### Premier démarrage

1. **Mise sous tension**
   - Connectez l'Arduino au PC ou à une batterie
   - Ouvrez le moniteur série (115200 bauds)

2. **Vérification du provisionnement**
   - L'appareil tentera automatiquement de se provisionner
   - Vérifiez les messages dans le moniteur série

3. **Fonctionnement normal**
   - Une fois provisionné, l'appareil entrera en cycle automatique
   - Lecture → Transmission → Deep Sleep

### Messages MQTT

Les données sont publiées sur le topic :

```text
sensors/{deviceID}/compact
```

**Format binaire du message :**

```text
[Device ID 1 octet][Flags 1 octet][Données variables]
```

### Décodage des données

Voici un exemple en Python pour décoder les messages :

```python
def decode_sensor_data(payload):
    if len(payload) < 2:
        return None
    
    device_id = payload[0]
    flags = payload[1]
    data = {}
    pos = 2
    
    if flags & 0x01:  # Température
        temp_raw = payload[pos]
        if temp_raw > 127:
            data['temperature'] = (temp_raw - 256) / 10.0
        else:
            data['temperature'] = temp_raw / 10.0
        pos += 1
    
    if flags & 0x02:  # Humidité
        data['humidity'] = payload[pos]
        pos += 1
    
    if flags & 0x04:  # Pression
        data['pressure'] = (payload[pos] / 10.0) + 90
        pos += 1
    
    # ... autres capteurs
    
    return device_id, data
```

## Dépannage

### Problèmes de connexion WiFi

- Vérifiez les identifiants WiFi dans Config.h
- Assurez-vous que le réseau est accessible
- Vérifiez la puissance du signal

### Problèmes de provisionnement

- Vérifiez que le serveur de provisionnement fonctionne
- Vérifiez l'adresse IP et le port du serveur
- Consultez les logs série pour les détails d'erreur

### Problèmes de capteurs

- Vérifiez les connexions du shield ENV
- Utilisez la fonction `testAllSensors()` pour diagnostiquer
- Vérifiez les logs d'initialisation

### Mode débogage

Pour activer les logs détaillés, assurez-vous que `DEBUG_ENABLED` est défini dans Config.h.

Pour désactiver les logs en production, commentez cette ligne :

```cpp
// #define DEBUG_ENABLED
```

## Optimisation énergétique

### Autonomie de la batterie

Avec une batterie 1200mAh :

- **Mode actif** : ~24h de fonctionnement continu
- **Mode deep sleep** : ~3-6 mois d'autonomie (selon la fréquence des réveils)

### Réglages pour économiser l'énergie

1. **Augmenter l'intervalle de deep sleep** :

   ```cpp
   #define SLEEP_TIME 600000  // 10 minutes au lieu de 5
   ```

2. **Augmenter les seuils de détection** :

   ```cpp
   #define TEMP_THRESHOLD 1.0   // Moins sensible
   #define HUM_THRESHOLD 5.0    // Moins sensible
   ```

3. **Augmenter l'intervalle maximal** :

   ```cpp
   #define MAX_NO_SEND_INTERVAL 7200000  // 2 heures au lieu d'1
   ```

## Support et maintenance

### Mise à jour du firmware

1. Gardez une copie de votre fichier Config.h personnalisé
2. Remplacez les autres fichiers par les nouvelles versions
3. Restaurez votre configuration
4. Recompilez et téléversez

### Réinitialisation des données

Pour effacer les données stockées en mémoire flash :

```cpp
storageManager.resetData();
provisionManager.forceNewRequest();
```

### Monitoring

Le système fournit des informations de diagnostic via :

- Messages série (si DEBUG_ENABLED)
- Statut dans les messages MQTT
- LED de statut sur la carte

## Architecture des fichiers

```text
MQTTSensorClient/
├── MQTTSensorClient.ino     # Programme principal
├── Config.h                 # Configuration globale
├── SensorManager.h/.cpp     # Gestion des capteurs
├── NetworkManager.h/.cpp    # Gestion réseau WiFi/MQTT
├── DataEncoder.h/.cpp       # Encodage binaire des données
├── StorageManager.h/.cpp    # Persistance en mémoire flash
├── ProvisionManager.h/.cpp  # Provisionnement automatique
└── README.md               # Ce fichier
```

## Liens utiles

- [Documentation Arduino MKR](https://docs.arduino.cc/hardware/mkr-wifi-1010)
- [Arduino MKR ENV Shield](https://docs.arduino.cc/hardware/mkr-env-shield)
- [Bibliothèque WiFiNINA](https://www.arduino.cc/en/Reference/WiFiNINA)
- [Protocole MQTT](https://mqtt.org/)

---

Pour toute question ou problème, consultez les logs série en mode debug ou créez une issue sur le dépôt du projet.
