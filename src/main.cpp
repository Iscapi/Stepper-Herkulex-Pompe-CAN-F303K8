#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HerkulexServo.h>
#include "SERVOS.h"
#include "STEPPER.h"

// As in StepperDemo for Motor 1 on AVR
#define position_ecarter 300

#define position_ecarter_pince 430

#define position_defaut 300

#define position_retourner 700

#define position_attraper 610

#define position_rapprocher 280

#define PIN_STBY PA5
#define PIN_STEP PA4
#define PIN_DIR PA3
#define PIN_EN PB1
#define PIN_M0 PB0
#define PIN_M1 PA7
#define PIN_M2 PA6
#define PAS_BAS 0
#define PAS_HAUT 700

// pince gauche
HerkulexServo servo_attraper_interieur(herkulex_bus, 0x01);
HerkulexServo servo_retourner_interieur(herkulex_bus, 0x02);
HerkulexServo servo_retourner_exterieur(herkulex_bus, 0x03);
HerkulexServo servo_attraper_exterieur(herkulex_bus, 0x04);
HerkulexServo servo_ecarter_pince(herkulex_bus, 0x05);
/* pince droit
HerkulexServo servo_attraper_interieur(herkulex_bus, 0x06);
HerkulexServo servo_retourner_interieur(herkulex_bus, 0x07);
HerkulexServo servo_retourner_exterieur(herkulex_bus, 0x08);
HerkulexServo servo_attraper_exterieur(herkulex_bus, 0x09);
HerkulexServo servo_ecarter_pince(herkulex_bus, 0x0A);
*/

HerkulexServo servo_rotae(herkulex_bus, 0x09);
HerkulexServo servo_serer(herkulex_bus, 0x05);
// FastAccelStepperEngine engine;
// FastAccelStepper *stepper = nullptr;

// FastAccelStepperEngine engine = FastAccelStepperEngine();
// FastAccelStepper *stepper = NULL;
int ID = 0;
int etat_rotae = 1;
void reception(char ch);
void tournere(void)
{

  switch (etat_rotae)
  {
  case 0:
    //servo_retourner_interieur.setPosition(position_defaut, 150, HerkulexLed::Green); // Position 0°
    servo_retourner_exterieur.setPosition(position_defaut, 150, HerkulexLed::Green); // Position 0°
    etat_rotae = 1;
    break;
  case 1:
    //servo_retourner_interieur.setPosition(position_retourner, 150, HerkulexLed::Blue); // Position 90°
    servo_retourner_exterieur.setPosition(position_retourner, 150, HerkulexLed::Blue); // Position 90°
    etat_rotae = 0;
    break;
    /*case 2:
      servo_rota.setPosition(ANGLE180, 50, HerkulexLed::Red); // Position 180°
      etat_rota = 3;
      break;
    case 3:
      servo_rota.setPosition(ANGLE90, 50, HerkulexLed::Red); // Position 180°
      etat_rota = 0;
      break;*/
  }
}
void serrere(void)
{
  servo_attraper_interieur.setPosition(position_attraper, 150, HerkulexLed::Green); // Ouvre la pince
  servo_attraper_exterieur.setPosition(position_attraper, 150, HerkulexLed::Green); // Ouvre la pince
}

void deserrere(void)
{
  servo_attraper_interieur.setPosition(position_ecarter, 150, HerkulexLed::Blue); // Ouvre la pince
  servo_attraper_exterieur.setPosition(position_ecarter, 150, HerkulexLed::Blue); // Ouvre la pince
}
void ecarter(void)
{
  servo_ecarter_pince.setPosition(position_ecarter_pince, 150, HerkulexLed::Green); // Ouvre la pince
  herkulex_bus.executeMove();
  // servo_ecarter_pince.reboot();
}
void rapprocher(void)
{
  servo_ecarter_pince.setPosition(position_rapprocher, 150, HerkulexLed::Blue); // Ouvre la pince
  herkulex_bus.executeMove();
  // servo_ecarter_pince.reboot();
}
void haut(void)
{
  digitalWrite(PIN_DIR, LOW);
  for (int i = 0; i < 700; i++)
  {
    digitalWrite(PIN_STEP, HIGH);
    delayMicroseconds(600);
    digitalWrite(PIN_STEP, LOW);
    delayMicroseconds(1000);
  }
}

void statique(void)
{

  for (int i = 0; i < 1000; i++)
  {
    digitalWrite(PIN_DIR, LOW);

    digitalWrite(PIN_STEP, HIGH);
    delayMicroseconds(5);
    digitalWrite(PIN_STEP, LOW);
    delayMicroseconds(10000);

    digitalWrite(PIN_DIR, HIGH);

    digitalWrite(PIN_STEP, HIGH);
    delayMicroseconds(5);
    digitalWrite(PIN_STEP, LOW);
    delayMicroseconds(10000);
  }
}

void bas(void)
{
  digitalWrite(PIN_DIR, HIGH);
  for (int i = 0; i < 700; i++)
  {
    digitalWrite(PIN_STEP, HIGH);
    delayMicroseconds(600);
    digitalWrite(PIN_STEP, LOW);
    delayMicroseconds(1000);
  }
}
String serie;
void setup()
{

  Serial.begin(115200);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_EN, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);

  digitalWrite(PIN_DIR, LOW);

  digitalWrite(PIN_STBY, HIGH); // sortir de veille
  delay(5);

  digitalWrite(PIN_EN, HIGH); // enable moteur

  init_serial_1_for_herkulex(); // Fonction init de "HERKULEX.h"
  delay(1000);
  // servo_ecarter_pince.setPosition(512,100,HerkulexLed::Green);
  //  all_servo.setPosition(300,150,HerkulexLed::Green);
  //   detect_id(true);
  /*servo_serer.setLedColor(HerkulexLed::Green);
  servo_rotae.setLedColor(HerkulexLed::Blue);
  change_id(5, servo_rotae, servo_serer);
  delay(1000);*/
  // servo_serer.setLedColor(HerkulexLed::Red);
  //  change_id(20,servo_rotae,servo_serer);
  //  servo_rotae.setLedColor(HerkulexLed::Red);
  //  servo_serer.setPosition(BAS, 50, HerkulexLed::Green); // descend la pince
  //  servo_serer.setPosition(HAUT, 50, HerkulexLed::Green); // descend la pince
  digitalWrite(PIN_DIR, LOW);
  for (int i = 0; i < 30; i++)
  {
    digitalWrite(PIN_STEP, HIGH);
    delayMicroseconds(600);
    digitalWrite(PIN_STEP, LOW);
    delayMicroseconds(1000);
  }
  deserrere();
  delay(1000);
  rapprocher();
}

void loop()

{
  // all_servo.reboot();

  delay(3000);
  serrere();
  delay(1000);
  ecarter();
  haut();
  delay(1000);
  tournere();
  delay(1000);
  rapprocher();
  delay(1000);
  bas();

  delay(1000);
  deserrere();

  /*servo_serer.setLedColor(HerkulexLed::Green);
     delay(1000);
     servo_serer.setLedColor(HerkulexLed::Blue);
     delay(1000);*/

  /* for (int i = 0x00; i < 0xFE; i++)
   {
     Serial.printf("0x%02X  %d\n",i,i);
     servo_rotae.setID(i);
     servo_rotae.setLedColor(HerkulexLed::Green);
     delay(1000);
     servo_rotae.setLedColor(HerkulexLed::Blue);
     delay(1000);

   }*/
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
