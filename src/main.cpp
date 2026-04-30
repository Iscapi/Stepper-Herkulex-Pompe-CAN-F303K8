#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <SoftwareSerial.h>
#include <HerkulexServo.h>
#include "SERVOS.h"
#include "STEPPER.h"
#include <STM32_CAN.h>

#define PAS_BAS 10
#define PAS_Retourner 700
#define PAS_HAUT 1150

STM32_CAN Can(CAN1, DEF); // Use PA11/12 pins for CAN1.

static CAN_message_t CAN_TX_msg;
static CAN_message_t CAN_RX_msg;

HerkulexServo servo_rotae(herkulex_bus, 0x02);
HerkulexServo servo_serer(herkulex_bus, 0x05);
TaskHandle_t _CAN_;

int tab_CAN[8];
int ID = 0;
int etat_rotae = 1;

int num_carte = 0;
int etat_RPI = 0, etat_ESP_RPI = 0, on_pour_rpi = 0;
int action_a_faire = 0, sous_pince = 0, verif_action = 0, ack_action = 0, pas_act = 0;

int en_cours = 0, pre_action = 0;
static unsigned long lastSend = 0;
int temp = 0, E = 0, mouv = 0, verif_curseur = 0;

void reception(char ch);
// void CAN_(void *);
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

  Can.setFilter(0, 0x01, 0x1FFFFFFF);
  Can.setFilter(1, 0x502 + num_carte, 0x1FFFFFFF);
  Can.setFilter(2, 0x500 + num_carte, 0x1FFFFFFF);
  Can.setFilter(3, 0x504 + num_carte, 0x1FFFFFFF);
  Can.setFilter(4, 0x206, 0x1FFFFFFF);
  Can.setFilter(5, 0x008, 0x1FFFFFFF);

  delay(5000);

  /*servo_serer.setLedColor(HerkulexLed::Green);
  servo_rotae.setLedColor(HerkulexLed::Blue);
  change_id(5, servo_rotae, servo_serer);
  delay(1000);*/
  /*servo_rotae.setPosition(300, 150, HerkulexLed::Green); // Ouvre la pince
  servo_serer.setPosition(300, 150, HerkulexLed::Blue); // Ouvre la pince
  delay (1000);
  servo_rotae.setPosition(900, 150, HerkulexLed::Green); // Ouvre la pince
  servo_serer.setPosition(900, 150, HerkulexLed::Blue); // Ouvre la pince
  */

  Serial.println("Test1");
  // xTaskCreate(CAN_, "CAN", 16000, NULL, 13, &_CAN_);
  Serial.println("Tes2");
  temp = millis();
  vTaskStartScheduler();
  Serial.println("Insufficient RAM");
  while (1)
    ;
}

void loop()
{
  while (1)
  {
    vTaskDelay(1);
    // Serial.print("test");

    // ── Lecture CAN ───────────────────────────────────────────
    if (Can.read(CAN_RX_msg))
    {
      // Serial.printf("ID:%03X ", CAN_RX_msg.id);
      if (CAN_RX_msg.id == 0x01)
      {
        etat_RPI = CAN_RX_msg.buf[0];
        // Serial.printf("RPI%d\n", etat_RPI);
      }

      if ((CAN_RX_msg.id == 0x500 + num_carte))
      {
        pre_action = action_a_faire;
        if (verif_action == 2)
        {
          tab_CAN[0] = CAN_RX_msg.buf[0];
          if (tab_CAN[0] != 2)
          {
            verif_action = 0;
            E = 0;
          }
        }
        else
        {
          action_a_faire = CAN_RX_msg.buf[0];
        }
        if (pre_action != 0 && action_a_faire != 0 && action_a_faire != pre_action)
        {
          E = 0;
          verif_action = 0;
        }

        // Serial.printf("action%d\n", action_a_faire);
      }

      if ((CAN_RX_msg.id == 0x502 + num_carte))
      {
        sous_pince = CAN_RX_msg.buf[0];
        // Serial.printf("pince%d\n", sous_pince);
      }

      if ((CAN_RX_msg.id == 0x504 + num_carte))
      {
        ack_action = CAN_RX_msg.buf[0];
        // Serial.printf("aquser%d\n", ack_action);
      }
      if ((CAN_RX_msg.id == 0x206) && num_carte == 0)
      {
        mouv = CAN_RX_msg.buf[0];
        // Serial.printf("mouv%d\n", mouv);
      }
      if ((CAN_RX_msg.id == 0x008) && num_carte == 0)
      {
        verif_curseur = CAN_RX_msg.buf[0];
        // Serial.printf("mouv%d\n", mouv);
      }
    }

    // ── Machine d'état RPI ────────────────────────────────────
    switch (etat_ESP_RPI)
    {
    case 0:
      on_pour_rpi = 1; // Variable pour dire à la RPI que la carte Actionneur fonctionne
      if (etat_RPI == 1)
      {
        pas_act = 0;
        // Serial.println("RPI lancée");
        etat_ESP_RPI = 1;
        delay(500 * num_carte);
        initStepper();
        delay(1000);
        haut(abs(pas_act - (PAS_HAUT)));
        pas_act = PAS_HAUT;
        delay(10000);
        init_serial_1_for_herkulex(); // Fonction init de "HERKULEX.h"
        delay(1000);
        
        desserrer(12);
        delay(1000);
        rapprocher();
        delay(1000);
        tourner(12);
        // change_id(5, servo_rotae, servo_serer);
        /* for (int i = 0x00; i < 0xFE; i++)
        {
          Serial.printf("0x%02X  %d\n",i,i);
          servo_rotae.setID(i);
          servo_rotae.setLedColor(HerkulexLed::Green);
          delay(000);
          servo_rotae.setLedColor(HerkulexLed::Blue);
          delay(000);
        }*/
        // servo_rotae.setPosition(300, 150, HerkulexLed::Green);
      }
      else
      {
        // Serial.println("Attente de la RPI");
        verif_action = 0;
        sous_pince = 0;
        action_a_faire = 0;
        E = 0;
      }
      break;

    case 1:
      if ((etat_RPI == 0 || etat_RPI == 2))
      {
        etat_ESP_RPI = 0;
        // Serial.println("Retour à l'état 0 (RPI arrêtée)");
      }
      // Serial.printf("\nverif_action : %d", verif_action);
      if (ack_action == 2 && action_a_faire != 2 && verif_action == 1)
      {
        verif_action = 0;
        sous_pince = 0;
        action_a_faire = 0;
        ack_action = 0;
        E = 0;
        // ordre de l'action = 0, donc on fait rien
      }

      break;
    }

    /*Serial.printf("\nAction : %d",action_a_faire);
    Serial.printf("\nSous_pince %d\n",sous_pince);*/

    // ── Envoi CAN périodique (toutes les 100 ms) ─────────────
    if (millis() - lastSend > 100)
    {
      lastSend = millis();

      CAN_TX_msg.id = 0x003 + num_carte; // ID CAN
      CAN_TX_msg.len = 1;                // DLC : Nombre d'octets dans le message
      CAN_TX_msg.buf[0] = on_pour_rpi;   // Données a envoyés
      Can.write(CAN_TX_msg);
      // Serial.println(lastSend);
      if (verif_action == 1)
      {
        CAN_TX_msg.id = 0x10C + num_carte; // ID CAN
        CAN_TX_msg.len = 1;                // DLC : Nombre d'octets dans le message
        CAN_TX_msg.buf[0] = verif_action;  // Données a envoyés
        Can.write(CAN_TX_msg);
      }
      
    }
    
    // Serial.println();
    // Serial.println(lastSend);

    // ── Exécution des actions ─────────────────────────────────
    if ((verif_action != 1) && (etat_RPI == 1) && mouv != 9)
    {
      switch (action_a_faire)
      {
      case 0:
        break;

      // ── ACTION 1 : ATTRAPER ──────────────────────────────
      case 1:
        switch (E)
        {
        case 0:
          if ((millis() - temp) > 00)
          {
            // Serial.print("bas");
            bas(abs(pas_act - PAS_BAS));
            pas_act = PAS_BAS;
            E++;
            temp = millis();
          }
          break;
        case 1:
          if ((millis() - temp) > 00)
          {
            // Serial.printf("serrer%d", sous_pince);
            serrer(sous_pince);
            E++;
            temp = millis();
          }
          break;
        case 2:
          if ((millis() - temp) > 1000)
          {
            // Serial.print("haut");
            verif_action = 1;
            CAN_TX_msg.id = 0x10C + num_carte; // ID CAN
        CAN_TX_msg.len = 1;                // DLC : Nombre d'octets dans le message
        CAN_TX_msg.buf[0] = verif_action;  // Données a envoyés
        Can.write(CAN_TX_msg);
            haut(abs(pas_act - PAS_HAUT));
            pas_act = PAS_HAUT;
            E++;
            temp = millis();
            verif_action = 1;
            CAN_TX_msg.id = 0x10C + num_carte; // ID CAN
        CAN_TX_msg.len = 1;                // DLC : Nombre d'octets dans le message
        CAN_TX_msg.buf[0] = verif_action;  // Données a envoyés
        Can.write(CAN_TX_msg);

            // Objet attrapé avec sous_pince → on met les pinces concernées à 1
            maj_etat_pince(sous_pince, 1);
          }
          break;
        case 3:
          // restart_retourner();
          // verif_action = 1;
          E = 4;
          break;
        case 4:
          break;
        }
        break;

      // ── ACTION 2 : RETOURNER ─────────────────────────────
      // On envoie immédiatement verif_action=1 à la réception de
      // l'action pour signaler à la RPI qu'elle est prise en charge,
      // puis on passe verif_action=2 pour continuer l'exécution
      // complète (la condition d'entrée est verif_action != 1).
      case 2:
        switch (E)
        {
        case 0:
          if ((millis() - temp) > 00)
          {
            // Accusé de réception immédiat vers la RPI
            verif_action = 1;
            CAN_TX_msg.id = 0x10C + num_carte; // ID CAN
            CAN_TX_msg.len = 1;                // DLC : Nombre d'octets dans le message
            CAN_TX_msg.buf[0] = verif_action;  // Données a envoyés
            Can.write(CAN_TX_msg);
            // On passe à 2 pour continuer l'exécution de l'action
            verif_action = 2;

            // Serial.print("ecarter");
            ecarter();
            bas(abs(pas_act - PAS_Retourner));
            pas_act = PAS_Retourner;

            E++;
            temp = millis();
          }
          break;
        case 1:
          if ((millis() - temp) > 500)
          {
            // Serial.printf("tourner%d", sous_pince);
            tourner(sous_pince);
            E++;
            temp = millis();
          }
          break;
        case 2:
          if ((millis() - temp) > 1500)
          {
            // Serial.print("rapprocher");
            haut(abs(pas_act - PAS_HAUT));
            pas_act = PAS_HAUT;
            rapprocher();
            verif_action = 0;
            sous_pince = 0;
            action_a_faire = 0;
            E = 0;
            temp = millis();
            //restart_all_servo();
            //init_serial_1_for_herkulex();
          }
          break;
        case 3:
          E = 0;
          break;
        case 4:
          break;
        }
        break;

      // ── ACTION 3 : RELACHER ──────────────────────────────
      case 3:
        switch (E)
        {
        case 0:
          if ((millis() - temp) > 00)
          {
            // Serial.print("bas");
            bas(abs(pas_act - PAS_BAS));
            pas_act = PAS_BAS;
            E++;
            temp = millis();
          }
          break;
        case 1:
          if ((millis() - temp) > 00)
          {
            // Serial.printf("desserer%d", sous_pince);
            desserrer(sous_pince);
            E++;
            temp = millis();
          }
          break;
        case 2:
          if ((millis() - temp) > 250)
          {verif_action = 1;
            CAN_TX_msg.id = 0x10C + num_carte; // ID CAN
        CAN_TX_msg.len = 1;                // DLC : Nombre d'octets dans le message
        CAN_TX_msg.buf[0] = verif_action;  // Données a envoyés
        Can.write(CAN_TX_msg);
            // Serial.print("haut");
            haut(abs(pas_act - PAS_HAUT));
            pas_act = PAS_HAUT;
            E++;
            verif_action = 1;
            temp = millis();

            // Objet déposé → on remet les pinces concernées à 0
            verif_action = 1;
            CAN_TX_msg.id = 0x10C + num_carte; // ID CAN
            CAN_TX_msg.len = 1;                // DLC : Nombre d'octets dans le message
            CAN_TX_msg.buf[0] = verif_action;  // Données a envoyés
            Can.write(CAN_TX_msg);
            maj_etat_pince(sous_pince, 0);
            restart_all_servo();
            init_serial_1_for_herkulex();
          }
          break;
        case 3:
          // verif_action = 1;
          //  Reboot des Herkulex après chaque dépôt

          E = 4;
          break;
        case 4:
          break;
        }
        break;

      default:
        break;
      }
    }
    else if (mouv == 9 && num_carte == 0)
    {
      if (verif_curseur == 0)
      {
        curseur();
        bas(pas_act);
        pas_act = 0;
      }
      else if (verif_curseur != 0)
      {
        haut(abs(pas_act - PAS_HAUT));
            pas_act = PAS_HAUT;
        rapprocher();
      }
    }
    else if (verif_curseur != 0 && num_carte == 0 && mouv == 9)
    {
      haut(abs(pas_act - PAS_HAUT));
            pas_act = PAS_HAUT;
      rapprocher();
    }
    // ── Surveillance erreurs Herkulex (toutes les 500 ms) ────

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

/*void CAN_(void *)
{
  TickType_t xLastWakeTime2;
  xLastWakeTime2 = xTaskGetTickCount();
  while (1)
  {
    Serial.println("Test");

    if (Can.read(CAN_RX_msg))
    {
      Serial.printf("%d ", CAN_RX_msg.id);

      if (CAN_RX_msg.id == 0x01)
      {
        etat_RPI = CAN_RX_msg.buf[0];
      }

      if (CAN_RX_msg.id == 0x500 + num_carte)
      {
        pre_action = action_a_faire;
        action_a_faire = CAN_RX_msg.buf[0];
        Serial.printf("action%d", action_a_faire);
      }

      if (CAN_RX_msg.id == 0x502 + num_carte)
      {
        sous_pince = CAN_RX_msg.buf[0];
        Serial.printf("pince%d", sous_pince);
      }

      if (CAN_RX_msg.id == 0x504 + num_carte)
      {
        ack_action = CAN_RX_msg.buf[0];
        Serial.printf("aquser%d", ack_action);
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
        sous_pince = 0;
        action_a_faire = 0;
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
        // ordre de l'action = 0, donc on fait rien
      }
      break;
    }

    static unsigned long lastSend = 0;
    if (millis() - lastSend > 50)
    { // toutes les 100 ms
      lastSend = millis();

      CAN_TX_msg.id = 0x003 + num_carte; // ID CAN
      CAN_TX_msg.len = 1;                // DLC : Nombre d'octets dans le message
      CAN_TX_msg.buf[0] = on_pour_rpi;   // Données a envoyés

      CAN_TX_msg.id = 0x109;            // ID CAN
      CAN_TX_msg.len = 1;               // DLC : Nombre d'octets dans le message
      CAN_TX_msg.buf[0] = verif_action; // Données a envoyés
    }
    Serial.println();
    vTaskDelayUntil(&xLastWakeTime2, pdMS_TO_TICKS(1));
  }
}*/

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

/*Pour les actionneurs on aura deux cartes
Pour moi la N°0 ça sera celle de devant, et le N°1 celle de derrière.
Pour envoyer des infos avec le CAN, en gros j'ajoute à l'ID un offset correspondant au N° de la carte

Type mouvement :
1 : Attraper
2 : Retourner  ecarter quand on retrourne
3 : Relacher

Noisette a manipulee :
1 : La pince fixe
2 : La pince qui peut se lever
12 : Les deux

Quand la pince aura fini son action, il faudra envoyé une confirmation d'une valeur de 1 sur l'ID 0x109, et j'enverrai un accusé de réception égal à 2 sur les ID 0x504 ou 0x505 en fonction du N° de la carte.  Là c'est exactement le même principe d'accusé de réception que sur la carte de l'Asservissement
*/