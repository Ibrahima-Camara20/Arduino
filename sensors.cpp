#include "sensors.h"

String get_temperature(DallasTemperature &sensor) {
  sensor.requestTemperatures();
  float t = sensor.getTempCByIndex(0);
  return String(t, 1);
}

int get_light(int pin) {
  int v = analogRead(pin);
  return v;
}
