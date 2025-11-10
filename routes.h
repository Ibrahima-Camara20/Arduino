#pragma once
#include "ESPAsyncWebServer.h"

// Déclaration des fonctions de routing
void   setup_http_routes(AsyncWebServer* server);
String processor(const String& var);
