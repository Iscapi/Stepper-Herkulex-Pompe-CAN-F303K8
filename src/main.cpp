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

void loop() // On test les fonctions une par une
{           // La boucle simule un attrapage d'objet, un retournement et un relachement
  descendre();
  delay(2000);
  monter();
  delay(2000);
}