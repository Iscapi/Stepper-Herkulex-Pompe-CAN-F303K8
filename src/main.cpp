#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HerkulexServo.h>
#include "SERVOS.h"
/*
HardwareSerial Serial1(USART1);
HerkulexServoBus herkulex_bus(Serial1);

HerkulexServo my_servo(herkulex_bus, HERKULEX_BROADCAST_ID);
HerkulexServo Servo_rota(herkulex_bus, SERVO_ROTATION);*/

void setup() {
  Serial.begin(115200);
  init_serial_1_for_herkulex(); // Fonction init de "HERKULEX.h"
  delay(500);
  detect_id(true);
}

void loop() {

}