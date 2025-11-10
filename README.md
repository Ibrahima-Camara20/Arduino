# 🧠 Projet ESP32 HTTP/Async/LittleFS

## 📋 Description
Ce projet met en œuvre un **serveur web asynchrone** sur un **ESP32**, capable :
- d’héberger une page web (HTML/CSS/JS) via **LittleFS** ;
- d’afficher les mesures de capteurs (température, luminosité) ;
- de contrôler des sorties (LED, ventilateur, etc.) ;
- d’envoyer périodiquement des données JSON vers un serveur externe (comme **Node-RED**) si activé.

L’interface web, accessible depuis un navigateur, permet d’interagir avec l’ESP32 grâce à des **inputs HTML** (champs texte, boutons, curseurs…).

---

## ⚙️ Fonctionnalités principales

| Fonction | Description |
|-----------|--------------|
| 🌐 Serveur web asynchrone | Fournit une interface utilisateur locale via HTTP |
| 📁 LittleFS | Stocke la page HTML, CSS et les scripts JavaScript dans la mémoire flash |
| 🌡️ Capteurs | Mesure la température et la luminosité simulées ou réelles |
| 💡 Commande | Permet d’activer/désactiver une LED ou un “cooler” depuis la page |
| 🔁 Reporting | Envoi périodique des mesures en JSON vers un serveur externe (optionnel) |
| ⚙️ Configuration web | Champs pour modifier IP, port et période d’envoi via formulaire HTML |

---

## 🧩 Dépendances (bibliothèques Arduino)

Installe ces bibliothèques via le **gestionnaire de bibliothèques Arduino** ou depuis GitHub :

| Bibliothèque | Source / Lien |
|---------------|----------------|
| **ESPAsyncWebServer** | [mathieucarbou/ESPAsyncWebServer](https://github.com/mathieucarbou/ESPAsyncWebServer) ✅ (fork compatible ESP32 2.x & 3.x) |
| **AsyncTCP** | [me-no-dev/AsyncTCP](https://github.com/me-no-dev/AsyncTCP) |
| **ArduinoJson** | Par Benoît Blanchon (v6.x) |
| **LittleFS** | Intégrée au core ESP32 (version ≥ 2.0.0) |
| **WiFi** | Intégrée au core ESP32 |
| *(Optionnel)* **HTTPClient** | Pour le reporting vers Node-RED |

---

## 🧰 Outils requis

- **Arduino IDE 1.8.x ou 2.x**
- **Core ESP32 by Espressif Systems (v2.0.17 recommandée)**  
  (à installer via l’URL :  
  `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`)
- **Plugin ESP32 LittleFS Data Upload**  
  pour téléverser les fichiers du dossier `/data`.

---

## 📂 Structure du projet

```
http_as_serverasync_littlefs/
├── http_as_serverasync_littlefs.ino    # Programme principal
├── routes.cpp                          # Définition des routes HTTP
├── routes.h
└── data/                               # Fichiers du système de fichiers LittleFS
    ├── index.html                      # Page web principale
    └── esp.css                         # Feuille de style
```

---

## 🚀 Étapes d’exécution

### 1️⃣ Installer les dépendances
- Installe le **core ESP32 2.0.17**  
- Installe les bibliothèques listées ci-dessus.  
- Vérifie qu’elles apparaissent dans le dossier :  
  `Documents/Arduino/libraries/`

---

### 2️⃣ Charger les fichiers LittleFS

1. Place `index.html` et `esp.css` dans le dossier `data/` du projet.  
2. Dans Arduino IDE : **Outils → ESP32 LittleFS Data Upload**  
3. Attends le message “LittleFS Upload Done”.

---

### 3️⃣ Compiler et téléverser le programme

1. Ouvre `http_as_serverasync_littlefs.ino`.  
2. Sélectionne la carte **ESP32 Dev Module**.  
3. Branche le module et choisis le bon port COM.  
4. Clique sur **→ Téléverser**.

---

### 4️⃣ Lancer le moniteur série
Ouvre **Outils → Moniteur série** à **9600 baud**.

Tu verras :
```
Booting ESP32 HTTP/Async/LittleFS...
WiFi connected : yes !
IP address : 172.20.10.10
LittleFS mount OK
Sensors updated -> T = 19.6 °C, L = 0
```

---

### 5️⃣ Accéder à la page web

1. Connecte ton PC ou smartphone au **même réseau Wi-Fi** que l’ESP32.  
2. Ouvre un navigateur à l’adresse :  
   ```
   http://<adresse_IP_affichée>
   ```
   (exemple : `http://172.20.10.10`)

3. La page web s’affiche 🎉


