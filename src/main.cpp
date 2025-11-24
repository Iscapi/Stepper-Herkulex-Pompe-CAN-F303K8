#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HerkulexServo.h>
#include "SERVOS.h"

void setup() {
  Serial.begin(115200);
  init_serial_1_for_herkulex(); // Fonction init de "HERKULEX.h"
  delay(500);
  //detect_id(true);
}

void loop() 
{
  serrer();
  delay(2000);
  tourner();
  delay(2000);
  desserrer();
  delay(2000);
}