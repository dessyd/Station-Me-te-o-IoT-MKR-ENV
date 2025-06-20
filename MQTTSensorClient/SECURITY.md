# Guide de Sécurité - Station Météo IoT

## Bonnes pratiques de sécurité

### 1. Gestion des secrets

#### ✅ À faire
- Utilisez toujours le fichier `arduino_secrets.h` pour les informations sensibles
- Ajoutez `arduino_secrets.h` à votre `.gitignore`
- Utilisez des mots de passe forts et uniques
- Changez les mots de passe par défaut de tous vos équipements

#### ❌ À éviter
- Ne jamais commiter `arduino_secrets.h` dans un dépôt public
- Ne pas partager vos identifiants WiFi ou MQTT
- Ne pas utiliser des mots de passe faibles (123456, password, etc.)
- Ne pas réutiliser les mêmes mots de passe partout

### 2. Sécurité réseau

#### Segmentation réseau
- Utilisez un réseau WiFi dédié pour vos objets IoT
- Isolez le réseau IoT du réseau principal si possible
- Configurez un firewall pour limiter les communications

#### Chiffrement
- Assurez-vous que votre WiFi utilise WPA3 (ou au minimum WPA2)
- Évitez les réseaux WiFi ouverts pour vos appareils IoT
- Considérez l'utilisation de MQTT over TLS (port 8883)

### 3. Authentification MQTT

#### Configuration recommandée
```cpp
// Dans arduino_secrets.h
#define SECRET_MQTT_USER "arduino_station_001"
#define SECRET_MQTT_PASSWORD "mot_de_passe_fort_et_unique"
```

#### Permissions MQTT
- Créez un utilisateur MQTT dédié pour chaque appareil
- Limitez les permissions aux topics nécessaires
- Utilisez des ACL (Access Control Lists) sur votre broker

### 4. Sécurité physique

#### Protection de l'appareil
- Sécurisez physiquement l'accès à l'Arduino
- Utilisez un boîtier étanche et verrouillable
- Protégez les connexions contre les intempéries

#### Accès série
- Désactivez le mode debug en production
- Protégez l'accès USB/série de l'appareil
- Considérez la désactivation complète du port série

### 5. Mise à jour et maintenance

#### Mises à jour régulières
- Maintenez le firmware à jour
- Surveillez les alertes de sécurité des bibliothèques
- Testez les mises à jour en environnement de développement

#### Monitoring
- Surveillez les logs de votre broker MQTT
- Mettez en place des alertes sur les connexions suspectes
- Surveillez la consommation de bande passante

### 6. Provisionnement sécurisé

#### Token de sécurité
Le système utilise la puce ECCX08 pour générer des tokens cryptographiques :
- Les tokens sont générés par matériel sécurisé
- Chaque demande utilise un token unique
- Protection contre les attaques par rejeu

#### Serveur de provisionnement
- Sécurisez votre serveur de provisionnement
- Validez toujours l'adresse MAC source
- Loggez toutes les tentatives de provisionnement
- Limitez les tentatives par adresse MAC

### 7. Gestion des données

#### Chiffrement des données
- Considérez le chiffrement des données sensibles
- Utilisez HTTPS pour les APIs web
- Chiffrez les sauvegardes de base de données

#### Retention des données
- Définissez une politique de rétention des données
- Supprimez automatiquement les anciennes données
- Respectez la réglementation RGPD si applicable

### 8. Surveillance et détection d'intrusion

#### Métriques à surveiller
- Nombre de connexions MQTT par appareil
- Fréquence des messages inhabituels
- Tentatives de connexion échouées
- Changements dans les patterns de données

#### Alertes recommandées
- Connexion depuis une nouvelle adresse IP
- Envoi de données en dehors des heures normales
- Volume de données anormalement élevé
- Échecs répétés d'authentification

### 9. Plan de réponse aux incidents

#### En cas de compromission
1. **Isolement** : Déconnectez l'appareil du réseau
2. **Évaluation** : Analysez l'étendue du problème
3. **Confinement** : Empêchez la propagation
4. **Éradication** : Supprimez la menace
5. **Récupération** : Restaurez le service sécurisé
6. **Leçons** : Documentez et améliorez

#### Contacts d'urgence
- Responsable IT de l'organisation
- Administrateur du broker MQTT
- Support technique Arduino (si nécessaire)

### 10. Configuration de production

#### Checklist avant déploiement
- [ ] `DEBUG_ENABLED` est commenté dans `Config.h`
- [ ] `arduino_secrets.h` n'est pas dans le dépôt
- [ ] Mots de passe uniques et forts configurés
- [ ] Réseau WiFi sécurisé (WPA3/WPA2)
- [ ] Authentification MQTT activée
- [ ] Boîtier physique sécurisé
- [ ] Plan de mise à jour défini
- [ ] Monitoring en place

---

**Rappel** : La sécurité est un processus continu, pas un état final. Réévaluez régulièrement vos mesures de sécurité et adaptez-les aux nouvelles menaces.
