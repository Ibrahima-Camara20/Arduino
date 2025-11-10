#pragma once
#include <Arduino.h>
#include <DallasTemperature.h>

String get_temperature(DallasTemperature &sensor);
int    get_light(int pin);
