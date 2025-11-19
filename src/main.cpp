#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HerkulexServo.h>
#include "HERKULEX.h"


void setup() {
  Serial.begin(115200);
  init_serial_1_for_herkulex(); // Fonction init de "HERKULEX.h"
  delay(500);
}

void loop() {      // Test sur my_servo (adresse broadcast)
  test_herkulex(); // Test des servos Herkulex (tourner et changer LED de couleur)
  //detect_id(true); // détection des servos sur le bus
}