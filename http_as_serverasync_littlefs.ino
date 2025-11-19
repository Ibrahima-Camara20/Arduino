/*
 * Projet : ESP32 HTTP + Dashboard Node-RED
 * Basé sur TPHTTP (ESPAsyncWebServer, LittleFS, HTTPClient…)
 */

#include <ArduinoOTA.h>
#include "ArduinoJson.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "wifi_utils.h"
#include "sensors.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include <FS.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "routes.h"

// Permet d'utiliser makeJSON_fromStatus() défini dans routes.cpp
StaticJsonDocument<1000> makeJSON_fromStatus();

#define USE_SERIAL Serial

/*===== ESP GPIO configuration ==============*/
/* ---- LED / COOLER ---- */
int LEDpin = 19;

/* ---- FIRE SENSOR ---- */
int FIRE_PIN = 27;  // adapte si besoin

/* ---- FAN PWM ---- */
int FAN_PIN = 5;
int fanChannel = 0;

/* ---- Light ----*/
int LightPin = A5;

/* ---- Temperature ----*/
OneWire oneWire(23);
DallasTemperature TempSensor(&oneWire);

/*====== ESP Statut =========================*/
String LEDState = "off";
String last_temp;
String last_light;

bool fireDetected = false;   // AJOUTÉ
int fanSpeed = 0;            // AJOUTÉ

/*====== Séuils de régulation ==================*/
float HT = 30.0;
float LT = 18.0;

short int Light_threshold = 250;

/*====== Reporting (Node-RED) ==================*/
String target_ip   = "172.20.10.2";
int    target_port = 1880;
int    target_sp   = 10;

/*====== Timers ==================*/
unsigned long lastSensorsMillis = 0;
unsigned long sensorsPeriodMs   = 10L * 1000;
unsigned long lastReportMillis  = 0;

/*====== Serveur HTTP =================*/
AsyncWebServer server(80);

/*====== Prototypes =====*/
void update_sensors();
void sendPeriodicReport();

/*=====================================================*/
/*                       setup()                       */
/*=====================================================*/
void setup() {
  USE_SERIAL.begin(9600);
  while (!USE_SERIAL) {}

  USE_SERIAL.println("\nBooting ESP32 HTTP/Async/LittleFS...");

  /* WiFi */
  String hostname = "Mon petit objet ESP32";
  wificonnect_multi(hostname);
  wifi_printstatus(0);

  /* LittleFS */
  LittleFS.begin();

  /* GPIO */
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, LOW);

  pinMode(FIRE_PIN, INPUT);  // AJOUTÉ

  /* FAN PWM */
  ledcSetup(fanChannel, 25000, 8);  // 25 kHz, 8-bit
  ledcAttachPin(FAN_PIN, fanChannel);
  ledcWrite(fanChannel, fanSpeed);

  /* Capteurs */
  TempSensor.begin();
  update_sensors();

  /* Routes HTTP */
  setup_http_routes(&server);
  server.begin();

  USE_SERIAL.println("Async HTTP server started on port 80");
}

/*=====================================================*/
/*                       loop()                        */
/*=====================================================*/
void loop() {
  unsigned long now = millis();

  if (now - lastSensorsMillis >= sensorsPeriodMs) {
    lastSensorsMillis = now;
    update_sensors();
  }

  if (target_sp > 0 &&
      (now - lastReportMillis >= (unsigned long)target_sp * 1000UL)) {
    lastReportMillis = now;
    sendPeriodicReport();
  }
}

/*=====================================================*/
/*                     update_sensors()                */
/*=====================================================*/
void update_sensors() {
  TempSensor.requestTemperatures();
  float t = TempSensor.getTempCByIndex(0);

  last_temp  = String(t, 1);
  last_light = String(get_light(LightPin));

  /* FIRE DETECTION */
  fireDetected = digitalRead(FIRE_PIN) == HIGH;

  /* AUTOMATIC FAN CONTROL */
  if (t > HT) fanSpeed = 255;
  else if (t < LT) fanSpeed = 0;

  ledcWrite(fanChannel, fanSpeed);

  USE_SERIAL.printf("Sensors updated -> T=%s C, L=%s, Fire=%d, Fan=%d\n",
                    last_temp.c_str(),
                    last_light.c_str(),
                    fireDetected,
                    fanSpeed);
}

/*=====================================================*/
/*      Envoi périodique du JSON COMPLET vers Node-RED */
/*=====================================================*/
void sendPeriodicReportTask(void *param) {
  if (WiFi.status() == WL_CONNECTED &&
      target_ip != "" &&
      target_sp > 0) 
  {
    HTTPClient http;

    String url = "http://" + target_ip + ":" + String(target_port)
                 + "/esp?mac=" + WiFi.macAddress();

    // JSON COMPLET identique à /status.json
    auto doc = makeJSON_fromStatus();
    String body;
    serializeJson(doc, body);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    int code = http.POST(body);
    Serial.printf("HTTP POST done, code = %d\n", code);

    http.end();
  }

  vTaskDelete(NULL);
}

void sendPeriodicReport() {
  xTaskCreatePinnedToCore(
      sendPeriodicReportTask,
      "SendReportTask",
      8192,
      NULL,
      1,
      NULL,
      1
  );
}
