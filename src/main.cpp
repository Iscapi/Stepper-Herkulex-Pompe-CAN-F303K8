#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <SoftwareSerial.h>
#include <HerkulexServo.h>
#include "SERVOS.h"
#include "STEPPER.h"
#include <STM32_CAN.h>

#define PAS_BAS 0
#define PAS_HAUT 700

STM32_CAN Can(CAN1, DEF); // Use PA11/12 pins for CAN1.

static CAN_message_t CAN_TX_msg;
static CAN_message_t CAN_RX_msg;

HerkulexServo servo_rotae(herkulex_bus, 0x06);
HerkulexServo servo_serer(herkulex_bus, 0x08);

int tab_CAN[8];
int ID = 0;
int etat_rotae = 1;

int num_carte = 0;
int etat_RPI = 0, etat_ESP_RPI = 0, on_pour_rpi = 0;
int action_a_faire = 0, sous_pince = 0, verif_action = 0, ack_action = 0;

int en_cours = 0, pre_action = 0;
void reception(char ch);

String serie;
void setup()
{

  Serial.begin(115200);
  Can.begin();
  // Can.setBaudRateValues(4, 11, 4, 1); // → 500 kbps avec APB1 à 32 MHz
  Can.setBaudRate(500000); // 500KBPS
  // delay(10000);
  // Serial.println("CAN init OK");
  // Serial.print("APB1 clock: ");
  // Serial.println(HAL_RCC_GetPCLK1Freq());

  // initStepper();
  // init_serial_1_for_herkulex(); // Fonction init de "HERKULEX.h"
  // delay(5000);

  /*servo_serer.setLedColor(HerkulexLed::Green);
  servo_rotae.setLedColor(HerkulexLed::Blue);
  change_id(5, servo_rotae, servo_serer);
  delay(1000);*/
  /*servo_rotae.setPosition(300, 150, HerkulexLed::Green); // Ouvre la pince
  servo_serer.setPosition(300, 150, HerkulexLed::Blue); // Ouvre la pince
  delay (1000);
  servo_rotae.setPosition(900, 150, HerkulexLed::Green); // Ouvre la pince
  servo_serer.setPosition(900, 150, HerkulexLed::Blue); // Ouvre la pince
  haut(50);
  desserrer();
  delay(1000);
  rapprocher();*/
  vTaskStartScheduler();
  while (1)
    ;
}

void loop()

{
  while (1)
  {
    //  Serial.println("Test");
    if (Can.read(CAN_RX_msg))
    {
      Serial.printf("%d ", CAN_RX_msg.id);

      if (CAN_RX_msg.id == 0x02)
      {
        for (int i = 0; i < CAN_RX_msg.len; i++)
        {
          tab_CAN[i] = CAN_RX_msg.buf[i]; // met les datas dans le tableau
          Serial.printf("%02X ", tab_CAN[i]);
        }
        Serial.println();
        if (CAN_RX_msg.id == 0x01)
        {
          etat_RPI = CAN_RX_msg.buf[0];
        }

        if (CAN_RX_msg.id == 0x500 + num_carte)
        {
          pre_action = action_a_faire;
          action_a_faire = CAN_RX_msg.buf[0];
        }

        if (CAN_RX_msg.id == 0x502 + num_carte)
        {
          sous_pince = CAN_RX_msg.buf[0];
        }

        if (CAN_RX_msg.id == 0x504 + num_carte)
        {
          ack_action = CAN_RX_msg.buf[0];
        }
      }
    }

    switch (etat_ESP_RPI)
    {
    case 0:
      on_pour_rpi = 1; // Variable pour dire à la RPI que la carte Actionneur fonctionne
      if (etat_RPI == 1)
      {
        Serial.println("RPI lancée");
        etat_ESP_RPI = 1;
      }
      else
      {
        Serial.println("Attente de la RPI");
        verif_action = 0;
      }
      break;

    case 1:
      if (etat_RPI == 0 || etat_RPI == 2)
      {
        etat_ESP_RPI = 0;
        Serial.println("Retour à l'état 0 (RPI arrêtée)");
      }
      Serial.printf("\nverif_action : %d", verif_action);
      if (ack_action == 2)
      {
        verif_action = 0;
      }
      break;
    }

    /*Serial.printf("\nAction : %d",action_a_faire);
    Serial.printf("\nSous_pince %d\n",sous_pince);*/

    static unsigned long lastSend = 0;
    if (millis() - lastSend > 50)
    { // toutes les 100 ms
      lastSend = millis();

      CAN_TX_msg.id = 0x003;           // ID CAN
      CAN_TX_msg.len = 1;              // DLC : Nombre d'octets dans le message
      CAN_TX_msg.buf[0] = on_pour_rpi; // Données a envoyés

      CAN_TX_msg.id = 0x109;            // ID CAN
      CAN_TX_msg.len = 1;               // DLC : Nombre d'octets dans le message
      CAN_TX_msg.buf[0] = verif_action; // Données a envoyés
    }
    if ((en_cours == 0 )|| (action_a_faire == pre_action))
    {
      switch (action_a_faire)
      {
      case 0:
        haut(PAS_HAUT);
        break;
      case 1:
        bas(PAS_BAS);
        break;
      case 2:
        serrer(sous_pince);
        break;
      case 3:
        desserrer(sous_pince);
        break;
      case 4:
        tourner(sous_pince);
        break;
      case 5:
        ecarter();
        break;
      case 6:
        rapprocher();
        break;
      default:
        break;
      }
    }
    // all_servo.reboot();

    /*delay(3000);
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
    desserrer();*/

    /*for (int i = 0x00; i < 0xFE; i++)
     {
       Serial.printf("0x%02X  %d\n",i,i);
       servo_rotae.setID(i);
       servo_rotae.setLedColor(HerkulexLed::Green);
       delay(1000);
       servo_rotae.setLedColor(HerkulexLed::Blue);
       delay(1000);

     }*/
    // delay(1000);
  }
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
