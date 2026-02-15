#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HerkulexServo.h>
#include "SERVOS.h"
#include "STEPPER.h"

#define PAS_BAS 0
#define PAS_HAUT 700

HerkulexServo servo_rotae(herkulex_bus, 0x09);
HerkulexServo servo_serer(herkulex_bus, 0x05);
// FastAccelStepperEngine engine;
// FastAccelStepper *stepper = nullptr;

// FastAccelStepperEngine engine = FastAccelStepperEngine();
// FastAccelStepper *stepper = NULL;
int ID = 0;
int etat_rotae = 1;
void reception(char ch);


String serie;
void setup()
{

  Serial.begin(115200);
  
  initStepper();
  init_serial_1_for_herkulex(); // Fonction init de "HERKULEX.h"
  delay(5000);

  /*servo_serer.setLedColor(HerkulexLed::Green);
  servo_rotae.setLedColor(HerkulexLed::Blue);
  change_id(5, servo_rotae, servo_serer);
  delay(1000);*/
 haut(50);
  desserrer();
  delay(1000);
  rapprocher();
}

void loop()

{
  // all_servo.reboot();

  delay(3000);
  serrer();
  delay(2000);
  ecarter();
  haut(700);
  delay(1000);
  tourner();
  delay(1000);
  rapprocher();
  delay(1000);
  bas(700);

  delay(1000);
  desserrer();


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
