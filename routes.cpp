#include <ArduinoJson.h>
#include <Arduino.h>
#include "ESPAsyncWebServer.h"
#include <LittleFS.h>
#include "routes.h"
#include "wifi_utils.h"

// On récupère les variables globales définies dans le .ino
extern String LEDState;
extern String last_temp;
extern String last_light;
extern float  HT;
extern float  LT;

extern String target_ip;
extern int    target_port;
extern int    target_sp;

extern const int LEDpin;

/*=====================================================*/
/*  Remplacement des PLACEHOLDERS dans index.html      */
/*=====================================================*/
String processor(const String& var) {
  if (var == "UPTIME") {
    return String(millis() / 1000);
  }
  if (var == "WHERE") {
    return String("ESP32 Lab HTTP");
  }
  if (var == "SSID") {
    return WiFi.SSID();
  }
  if (var == "MAC") {
    return WiFi.macAddress();
  }
  if (var == "IP") {
    return WiFi.localIP().toString();
  }

  if (var == "TEMPERATURE") {
    return last_temp;
  }
  if (var == "LIGHT") {
    return last_light;
  }
  if (var == "COOLER") {
    return LEDState;
  }
  if (var == "HEATER") {
    // si tu as un heater, sinon "N/A"
    return String("N/A");
  }

  if (var == "LT") {
    return String(LT, 1);
  }
  if (var == "HT") {
    return String(HT, 1);
  }

  // Infos de reporting
  if (var == "PRT_IP") {
    return (target_ip == "" ? String("not set") : target_ip);
  }
  if (var == "PRT_PORT") {
    return String(target_port);
  }
  if (var == "PRT_T") {
    return String(target_sp);
  }

  // Si placeholder inconnu
  return String();
}

/*=====================================================*/
/*              Setup des routes HTTP                  */
/*=====================================================*/
void setup_http_routes(AsyncWebServer* server) {

  // Route racine -> index.html avec processor()
  server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("Root route requested");
    request->send(LittleFS, "/index.html", String(), false, processor);
  });

  /*------------------ Routes pour JS Fetch -----------*/
  // GET /temperature -> texte brut
  server->on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", last_temp);
  });

  // GET /light -> texte brut
  server->on("/light", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", last_light);
  });

  /*------------------ GET /value ---------------------*/
  // /value?temperature&light&ht...
  server->on("/value", HTTP_GET, [](AsyncWebServerRequest* request) {
    int n = request->params();
    if (n == 0) {
      request->send(400, "text/plain", "No parameter");
      return;
    }

    StaticJsonDocument<256> doc;
    bool hasKnownParam = false;
    bool hasUnknown    = false;

    for (int i = 0; i < n; i++) {
      AsyncWebParameter* p = request->getParam(i);
      String name = p->name();

      if (name == "temperature") {
        doc["temperature"] = last_temp.toFloat();
        hasKnownParam = true;
      } else if (name == "light") {
        doc["light"] = last_light.toInt();
        hasKnownParam = true;
      } else if (name == "fire") {
        // si tu as un détecteur de feu, sinon valeur bidon
        doc["fire"] = false;
        hasKnownParam = true;
      } else if (name == "ht") {
        doc["ht"] = HT;
        hasKnownParam = true;
      } else if (name == "lt") {
        doc["lt"] = LT;
        hasKnownParam = true;
      } else {
        hasUnknown = true;
      }
    }

    if (!hasKnownParam || hasUnknown) {
      request->send(404, "text/plain", "Unknown parameter in /value");
      return;
    }

    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  /*------------------ GET /set -----------------------*/
  // /set?cool=on  ou  /set?ht=25.0&lt=18.0
  server->on("/set", HTTP_GET, [](AsyncWebServerRequest* request) {
    int n = request->params();
    if (n == 0) {
      request->send(400, "text/plain", "No parameter");
      return;
    }

    bool hasKnown = false;
    bool hasUnknown = false;

    for (int i = 0; i < n; i++) {
      AsyncWebParameter* p = request->getParam(i);
      String name = p->name();
      String value = p->value();

      if (name == "cool") {
        if (value == "on") {
          digitalWrite(LEDpin, HIGH);
          LEDState = "on";
        } else if (value == "off") {
          digitalWrite(LEDpin, LOW);
          LEDState = "off";
        }
        hasKnown = true;
      } else if (name == "ht") {
        HT = value.toFloat();
        hasKnown = true;
      } else if (name == "lt") {
        LT = value.toFloat();
        hasKnown = true;
      } else {
        hasUnknown = true;
      }
    }

    if (!hasKnown || hasUnknown) {
      request->send(404, "text/plain", "Unknown parameter in /set");
      return;
    }

    request->send(200, "text/plain", "OK");
  });

  /*------------------ POST /target -------------------*/
  // Formulaire HTML : ip, port, sp
  server->on("/target", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (request->hasParam("ip", true)) {
      target_ip = request->getParam("ip", true)->value();
    }
    if (request->hasParam("port", true)) {
      target_port = request->getParam("port", true)->value().toInt();
    }
    if (request->hasParam("sp", true)) {
      target_sp = request->getParam("sp", true)->value().toInt();
    }

    Serial.printf("Reporting updated: ip=%s port=%d sp=%d\n",
                  target_ip.c_str(), target_port, target_sp);

    // simple réponse
    request->send(200, "text/html",
                  "<html><body><h2>Reporting updated</h2>"
                  "<a href=\"/\">Back to status page</a></body></html>");
  });

  /*------------------ 404 par défaut -----------------*/
  server->onNotFound([](AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not found");
  });
}
