/*
 * Projet : ESP32 HTTP + Dashboard Node-RED
 * Basé sur les codes du TPHTTP (ESPAsyncWebServer, LittleFS, HTTPClient, ArduinoJson...)
 */

#include <ArduinoOTA.h>
#include "ArduinoJson.h"
#include <HTTPClient.h>         
#include <ArduinoJson.h> 
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "wifi_utils.h"      // fourni dans le TP
#include "sensors.h"        // fourni dans le TP (get_temperature, get_light, etc.)
#include "OneWire.h"
#include "DallasTemperature.h"
#include "FS.h"
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#include "routes.h"         // on va le définir plus bas

#define USE_SERIAL Serial

void setup_OTA(); // si tu as déjà un ota.ino, sinon tu peux commenter

/*===== ESP GPIO configuration ==============*/
/* ---- LED ----*/
 int LEDpin   = 19;  // LED de "cooler" par exemple
/* ---- Light ----*/
 int LightPin = A5;  // ADC1_CHANNEL_5 (GPIO 33) ou ce que tu utilises déjà
/* ---- Temperature ----*/
OneWire oneWire(23);
DallasTemperature TempSensor(&oneWire);

/*====== ESP Statut =========================*/
// Représentation interne de l’état de l’objet
String LEDState = "off";      // "cooler"
String last_temp;
String last_light;

// Seuils de régulation
float HT = 30.0;  // High temp threshold
float LT = 18.0;  // Low  temp threshold

// Lumière : juste un exemple, si tu en as besoin
short int Light_threshold = 250; // Less => night, more => day

// Host pour reporting périodique
String target_ip   = "";
int    target_port = 1880;   // Node-RED par défaut
int    target_sp   = 0;      // période d’envoi en secondes (0 = off)

// Timers
unsigned long lastSensorsMillis = 0;
unsigned long sensorsPeriodMs   = 10L * 1000; // 10 s
unsigned long lastReportMillis  = 0;

// Serveur HTTP asynchrone
AsyncWebServer server(80);

/*================= Prototypes locaux =================*/
void update_sensors();
void sendPeriodicReport();

/*=====================================================*/
/*                    setup()                           */
/*=====================================================*/
void setup() {
  /* Serial -----------------------------*/
  USE_SERIAL.begin(9600);
  while (!USE_SERIAL) { }

  USE_SERIAL.println("\nBooting ESP32 HTTP/Async/LittleFS...");

  /* WiFi ------------------------------*/
  String hostname = "Mon petit objet ESP32";
  wificonnect_multi(hostname);

  if (WiFi.status() == WL_CONNECTED) {
    USE_SERIAL.println("WiFi connected : yes !");
    wifi_printstatus(0);
  } else {
    USE_SERIAL.println("WiFi connected : no !");
  }

  /* LittleFS --------------------------*/
  if (!LittleFS.begin()) {
    USE_SERIAL.println("LittleFS mount failed !");
  } else {
    USE_SERIAL.println("LittleFS mount OK");
  }

  /* GPIO ------------------------------*/
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, LOW);
  LEDState = "off";

  /* Capteurs --------------------------*/
  TempSensor.begin();
  update_sensors();  // première lecture

  /* OTA (optionnel) -------------------*/
  // setup_OTA();

  /* Routes HTTP -----------------------*/
  setup_http_routes(&server);

  /* Lancement serveur HTTP -----------*/
  server.begin();
  USE_SERIAL.println("Async HTTP server started on port 80");
}

/*=====================================================*/
/*                    loop()                            */
/*=====================================================*/
void loop() {
  // OTA si utilisé
  // ArduinoOTA.handle();

  unsigned long now = millis();

  // Mise à jour des capteurs toutes sensorsPeriodMs ms (non bloquant)
  if (now - lastSensorsMillis >= sensorsPeriodMs) {
    lastSensorsMillis = now;
    update_sensors();
    wifi_printstatus(0); // juste pour debug
  }

  // Envoi périodique vers Node-RED si activé
  if (target_sp > 0 && (now - lastReportMillis >= (unsigned long)target_sp * 1000UL)) {
    lastReportMillis = now;
    sendPeriodicReport();
  }
}

/*=====================================================*/
/*         Lecture périodique des capteurs              */
/*=====================================================*/
void update_sensors() {
  // IMPORTANT : ne pas lire les capteurs dans les callbacks async
  TempSensor.requestTemperatures();
  float t = TempSensor.getTempCByIndex(0);
  last_temp = String(t, 1);   // 1 décimale

  int l = get_light(LightPin);
  last_light = String(l);

  USE_SERIAL.printf("Sensors updated -> T = %s C, L = %s\n",
                    last_temp.c_str(), last_light.c_str());
}

/*=====================================================*/
/*      Envoi périodique du JSON vers Node-RED         */
/*=====================================================*/
void sendPeriodicReportTask(void *param) {
  if (WiFi.status() == WL_CONNECTED && target_ip != "" && target_sp > 0) {
    HTTPClient http;

    String url = "http://" + target_ip + ":" + String(target_port) +
                 "/esp?mac=" + WiFi.macAddress();

    StaticJsonDocument<256> doc;
    doc["temperature"] = last_temp.toFloat();
    doc["light"] = last_light.toInt();
    doc["cooler"] = LEDState;
    String body;
    serializeJson(doc, body);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(body);
    Serial.printf("HTTP POST done, code = %d\n", code);
    http.end();
  }
  vTaskDelete(NULL); // Supprime la tâche une fois terminée
}

void sendPeriodicReport() {
  // Crée une tâche séparée pour envoyer le rapport sans bloquer le serveur
  xTaskCreatePinnedToCore(
      sendPeriodicReportTask,  // fonction de la tâche
      "SendReportTask",        // nom
      8192,                    // taille de pile
      NULL,                    // paramètre
      1,                       // priorité
      NULL,                    // handle
      1                        // cœur CPU (0 ou 1)
  );
}
