#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HerkulexServo.h>
#include "SERVOS.h"

// As in StepperDemo for Motor 1 on AVR
#define dirPinStepper 17
#define enablePinStepper 25
#define stepPinStepper 16
HerkulexServo servo_rotae(herkulex_bus, 253);
HerkulexServo servo_serer(herkulex_bus, 12);
// FastAccelStepperEngine engine;
// FastAccelStepper *stepper = nullptr;

// FastAccelStepperEngine engine = FastAccelStepperEngine();
// FastAccelStepper *stepper = NULL;
int ID = 0;
void reception(char ch);

String serie;
void setup()
{
  Serial.begin(115200);
  init_serial_1_for_herkulex(); // Fonction init de "HERKULEX.h"
  delay(500);
  /*stepper = engine.stepperConnectToPin(10);
stepper->setDirectionPin(9);
stepper->setSpeedInHz(2000);
stepper->setAcceleration(50000)*/

  servo_rotae.setLedColor(HerkulexLed::Blue);
  //change_id(12,servo_rotae,servo_serer);
  servo_serer.setLedColor(HerkulexLed::Red);
  // change_id(20,servo_rotae,servo_serer);
  // servo_rotae.setLedColor(HerkulexLed::Red);
  // servo_serer.setPosition(BAS, 50, HerkulexLed::Green); // descend la pince
  // servo_serer.setPosition(HAUT, 50, HerkulexLed::Green); // descend la pince

  // monter();
  // delay(1000);
  // descendre();
  // delay(1000);
  // detect_id(true);
}

void loop()
{
  /*for (int i = 0; i < 255; i++)
  {
    Serial.println(i);
    servo_rotae.setID(i);
    servo_rotae.setLedColor(HerkulexLed::Green);
    delay(200);
    servo_rotae.setLedColor(HerkulexLed::Blue);
    delay(200);

  }*/
  // On test les fonctions une par une
  // La boucle simule un attrapage d'objet, un retournement et un relachement
  // descendre();
  // delay(1000);
  // monter();
  // delay(1000);

  /*for (int i = 0; i < 150; i++)
  {

    all_servo.setPosition(BAS, 50, HerkulexLed::Green); // descend la pince
    herkulex_bus.executeMove();
    delay(500);
    all_servo.setPosition(HAUT, 50, HerkulexLed::Green); // descend la pince
    herkulex_bus.executeMove();
    delay(500);
    Serial.println(i);
  }*/
  // stepper->move(200);
}

void reception(char ch)
{

  static int i = 0;
  static String chaine = "";
  String commande;
  String valeur;
  int index, length;

  if ((ch == 13) or (ch == 10))
  {
    index = chaine.indexOf(' ');
    length = chaine.length();
    if (index == -1)
    {
      commande = chaine;
      valeur = "";
    }
    else
    {
      commande = chaine.substring(0, index);
      valeur = chaine.substring(index + 1, length);
    }

    if (commande == "ID")
    {
    }

    chaine = "";
  }
  else
  {
    chaine += ch;
  }
}
