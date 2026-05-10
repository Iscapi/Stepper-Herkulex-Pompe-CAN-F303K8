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
int action_suivante = 0;  // Pour stocker la prochaine action

void reception(char ch);
// void CAN_(void *);
String serie;

void setup()
{
  Serial.begin(115200);
  Can.begin();
  Can.setBaudRate(500000); // 500KBPS

  Can.setFilter(0, 0x01,              0x1FFFFFFF);
  Can.setFilter(1, 0x502 + num_carte, 0x1FFFFFFF);
  Can.setFilter(2, 0x500 + num_carte, 0x1FFFFFFF);
  Can.setFilter(3, 0x504 + num_carte, 0x1FFFFFFF);
  Can.setFilter(4, 0x206,             0x1FFFFFFF);
  Can.setFilter(5, 0x008,             0x1FFFFFFF);

  delay(5000);

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

    // ── Lecture CAN ───────────────────────────────────────────
    if (Can.read(CAN_RX_msg))
    {
      if (CAN_RX_msg.id == 0x01)
      {
        etat_RPI = CAN_RX_msg.buf[0];
      }

      if ((CAN_RX_msg.id == 0x500 + num_carte)&&action_a_faire !=2)
      {
        int nouvelle_action = CAN_RX_msg.buf[0];

        // Si aucune action en cours (E == 0), accepter directement
        if (E == 0)
        {
          action_a_faire = nouvelle_action;
          //Serial.printf("Nouvelle action acceptee: %d\n", action_a_faire);
        }
      }

      if ((CAN_RX_msg.id == 0x502 + num_carte)&&action_a_faire != 2)
      {
        sous_pince = CAN_RX_msg.buf[0];
      }

      if ((CAN_RX_msg.id == 0x504 + num_carte))
      {
        ack_action = CAN_RX_msg.buf[0];
        //Serial.printf("ACK CAN recu: %d (action=%d, E=%d)\n",
        //             ack_action, action_a_faire, E);
      }

      if ((CAN_RX_msg.id == 0x206) && num_carte == 0)
      {
        mouv = CAN_RX_msg.buf[0];
      }

      if ((CAN_RX_msg.id == 0x008) && num_carte == 0)
      {
        verif_curseur = CAN_RX_msg.buf[0];
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
        etat_ESP_RPI = 1;
        delay(500 * num_carte);
        initStepper();
        delay(1000);
        haut(abs(pas_act - (PAS_HAUT)));
        pas_act = PAS_HAUT;
        delay(15000);
        init_serial_1_for_herkulex(); // Fonction init de "HERKULEX.h"
        // init_serial_1_for_herkulex() envoie déjà les positions de repos
        // et attend 1200ms, on attend ensuite la confirmation de mouvement
        while (!desserrer(12));
        delay(1000);
        while (!rapprocher());
        delay(1000);
        while (!tourner(12));
        delay(1200);
      }
      else
      {
        verif_action   = 0;
        sous_pince     = 0;
        action_a_faire = 0;
        action_suivante = 0;
        ack_action     = 0;
        E = 0;
      }
      break;

    case 1:
      if ((etat_RPI == 0 || etat_RPI == 2))
      {
        etat_ESP_RPI = 0;
      }
      break;
    }

    // ── Envoi CAN périodique (toutes les 100 ms) ─────────────
    if (millis() - lastSend > 100)
    {
      lastSend = millis();

      CAN_TX_msg.id     = 0x003 + num_carte; // ID CAN
      CAN_TX_msg.len    = 1;                 // DLC : Nombre d'octets dans le message
      CAN_TX_msg.buf[0] = on_pour_rpi;       // Données a envoyés
      Can.write(CAN_TX_msg);

      CAN_TX_msg.id     = 0x10C + num_carte; // ID CAN
      CAN_TX_msg.len    = 1;                 // DLC : Nombre d'octets dans le message
      CAN_TX_msg.buf[0] = verif_action;      // Données a envoyés
      Can.write(CAN_TX_msg);
    }

    // ── Exécution des actions ─────────────────────────────────
    if ((etat_RPI == 1) && (mouv != 9 && mouv != 10))
    {
      switch (action_a_faire)
      {
      case 0:
        // Aucune action en cours, charger l'action suivante si disponible
        if (action_suivante != 0)
        {
          action_a_faire  = action_suivante;
          action_suivante = 0;
          //Serial.printf("Chargement action suivante: %d\n", action_a_faire);
        }
        break;

      // ── ACTION 1 : ATTRAPER ──────────────────────────────
      // E=0 : reset ack + descente
      // E=1 : serrage (non-bloquant, attend true)
      // E=2 : attente fin serrage (1800ms depuis E1) + ACK + montée + maj pince
      // E=3 : attente ACK RPI (ack_action==2) + reset
      case 1:
        switch (E)
        {
        case 0:
          // CRUCIAL: Reset ack_action au début de l'action
          ack_action = 0;
          //Serial.println("Action 1 - E0: Descente (ack_action reset)");
          bas(abs(pas_act - PAS_BAS));
          pas_act = PAS_BAS;
          E = 1;
          temp = millis();
          break;

        case 1:
          if ((millis() - temp) > 00)
          {
            // serrer() non-bloquant : retourne true dès que mouvement détecté
            // (ou si déjà en position), reboot si pas de mouvement après 100ms
            if (serrer(sous_pince))
            {
              //Serial.printf("Action 1 - E1: Serrage confirme pince %d\n", sous_pince);
              E = 2;
              temp = millis();
            }
          }
          break;

        case 2:
          // Laisse le serrage se compléter (1800ms depuis détection du mouvement)
          if ((millis() - temp) > 00)
          {
            //Serial.printf("Action 1 - E2: Montee + ACK\n");
            verif_action      = 1;
            CAN_TX_msg.id     = 0x10C + num_carte; // ID CAN
            CAN_TX_msg.len    = 1;                 // DLC : Nombre d'octets dans le message
            CAN_TX_msg.buf[0] = verif_action;      // Données a envoyés
            Can.write(CAN_TX_msg);

            haut(abs(pas_act - PAS_HAUT));
            pas_act = PAS_HAUT;
            maj_etat_pince(sous_pince, 1);

            E = 3;
            temp = millis();
          }
          break;

        case 3:
          // Attend l'ACK de la RPI (ack_action == 2) avant de reset
          if (ack_action == 2)
          {
            //Serial.println("Action 1 - E3: ACK confirme, reset complet");
            verif_action   = 0;
            sous_pince     = 0;
            action_a_faire = 0;
            ack_action     = 0;
            E = 0;
          }
          else
          {
            // Debug : affiche toutes les secondes si on attend encore
            static unsigned long dernierDebug = 0;
            if ((millis() - dernierDebug) > 1000)
            {
              Serial.printf("Action 1 - E3: En attente ACK (ack_action=%d)\n", ack_action);
              dernierDebug = millis();
            }
          }
          break;
        }
        break;

      // ── ACTION 2 : RETOURNER ─────────────────────────────
      // On envoie immédiatement verif_action=1 à la réception de
      // l'action pour signaler à la RPI qu'elle est prise en charge.
      // On attend ensuite l'ACK de la RPI avant de commencer le mouvement.
      //
      // E=0 : reset ack + ACK immédiat vers RPI
      // E=1 : attente ACK RPI (ack_action==2)
      // E=2 : ecarter (non-bloquant) + descente
      // E=3 : attente 500ms + tourner (non-bloquant, attend true)
      // E=4 : attente 1500ms (rotation complète) + montée + rapprocher + reset
      case 2:
        switch (E)
        {
        case 0:
          // CRUCIAL: Reset ack_action au début de l'action
          ack_action = 0;
         // Serial.println("Action 2 - E0: Envoi ACK immediat (ack_action reset)");
          // Envoi ACK immédiat
          verif_action      = 1;
          CAN_TX_msg.id     = 0x10C + num_carte; // ID CAN
          CAN_TX_msg.len    = 1;                 // DLC : Nombre d'octets dans le message
          CAN_TX_msg.buf[0] = verif_action;      // Données a envoyés
          Can.write(CAN_TX_msg);

          E = 1;
          temp = millis();
          break;

        case 1:
          // Attente ACK de la RPI avant de commencer le mouvement
          if (ack_action == 2)
          {
          //  Serial.println("Action 2 - E1: ACK confirme, debut action");
            ack_action = 0;
            E = 2;
            temp = millis();
          }
          else
          {
            static unsigned long dernierDebug = 0;
            if ((millis() - dernierDebug) > 1000)
            {
           //   Serial.printf("Action 2 - E1: En attente ACK (ack_action=%d)\n", ack_action);
              dernierDebug = millis();
            }
          }
          break;

        case 2:
          if ((millis() - temp) > 00)
          {
            // ecarter() non-bloquant : retourne true dès que mouvement détecté
            if (ecarter())
            {
              //Serial.println("Action 2 - E2: Ecarter confirme + Descente");
              bas(abs(pas_act - PAS_Retourner));
              pas_act = PAS_Retourner;
              E = 3;
              temp = millis();
            }
          }
          break;

        case 3:
          if ((millis() - temp) > 00)
          {
            // tourner() non-bloquant : retourne true dès que mouvement détecté
            if (tourner(sous_pince))
            {
              //Serial.printf("Action 2 - E3: Rotation confirmee pince %d\n", sous_pince);
              E = 4;
              temp = millis();
            }
          }
          break;

        case 4:
          // Laisse la rotation se compléter (1500ms depuis détection du mouvement)
          if ((millis() - temp) > 00)
          {
            //Serial.println("Action 2 - E4: Montee + Rapprocher + Reset");
            haut(abs(pas_act - PAS_HAUT));
            pas_act = PAS_HAUT;

            // rapprocher() non-bloquant : attend confirmation avant de reset
            if (rapprocher())
            {
              verif_action   = 0;
              sous_pince     = 0;
              action_a_faire = 0;
              E = 0;
              temp = millis();
            }
          }
          break;
        }
        break;

      // ── ACTION 3 : RELACHER ──────────────────────────────
      // E=0 : reset ack + descente
      // E=1 : desserrage (non-bloquant, attend true)
      // E=2 : attente fin desserrage (300ms) + ACK + montée + reboot + réinit position
      // E=3 : attente ACK RPI (ack_action==2) + reset
      case 3:
        switch (E)
        {
        case 0:
          // CRUCIAL: Reset ack_action au début de l'action
          ack_action = 0;
          //Serial.println("Action 3 - E0: Descente (ack_action reset)");
          bas(abs(pas_act - PAS_BAS));
          pas_act = PAS_BAS;
          E = 1;
          temp = millis();
          break;

        case 1:
          if ((millis() - temp) > 00)
          {
            // desserrer() non-bloquant : retourne true dès que mouvement détecté
            // (ou si déjà en position), reboot si pas de mouvement après 100ms
            if (desserrer(sous_pince))
            {
              //Serial.printf("Action 3 - E1: Desserrage confirme pince %d\n", sous_pince);
              E = 2;
              temp = millis();
            }
          }
          break;

        case 2:
          // Laisse le desserrage se compléter (300ms depuis détection du mouvement)
          if ((millis() - temp) > 00)
          {
            //Serial.printf("Action 3 - E2: Montee + ACK + Reboot\n");
            verif_action      = 1;
            CAN_TX_msg.id     = 0x10C + num_carte; // ID CAN
            CAN_TX_msg.len    = 1;                 // DLC : Nombre d'octets dans le message
            CAN_TX_msg.buf[0] = verif_action;      // Données a envoyés
            Can.write(CAN_TX_msg);

            haut(abs(pas_act - PAS_HAUT));
            pas_act = PAS_HAUT;
            maj_etat_pince(sous_pince, 0);

            // Reboot + vérification erreurs + réinit (moment sûr)
            //restart_all_servo();
            //check_herkulex_errors_once();
            //init_serial_1_for_herkulex();
            // init_serial_1_for_herkulex() envoie les positions de repos + attend 1200ms

            // Retour position repos avec confirmation de mouvement
            while (!rapprocher());
            delay(1000);
            while (!tourner(12));
            delay(1200);
            while (!desserrer(12));
            delay(1000);

            E = 3;
            temp = millis();
          }
          break;

        case 3:
          // Attend l'ACK de la RPI (ack_action == 2) avant de reset
          if (ack_action == 2)
          {
            //Serial.println("Action 3 - E3: ACK confirme, reset complet");
            verif_action   = 0;
            sous_pince     = 0;
            action_a_faire = 0;
            ack_action     = 0;
            E = 0;
          }
          else
          {
            static unsigned long dernierDebug = 0;
            if ((millis() - dernierDebug) > 1000)
            {
             // Serial.printf("Action 3 - E3: En attente ACK (ack_action=%d)\n", ack_action);
              dernierDebug = millis();
            }
          }
          break;
        }
        break;

      default:
        break;
      }
    }
    else if ((mouv == 9 || mouv == 10) && num_carte == 0)
    {
      if (verif_curseur == 0)
      {
        // curseur() non-bloquant : attend confirmation
        while (!curseur());
        bas(pas_act);
        pas_act = 0;
      }
      else if (verif_curseur != 0)
      {
        haut(abs(pas_act - PAS_HAUT));
        pas_act = PAS_HAUT;
        while (!rapprocher());
      }
    }
    else if (verif_curseur != 0 && num_carte == 0 && (mouv == 9 || mouv == 10))
    {
      haut(abs(pas_act - PAS_HAUT));
      pas_act = PAS_HAUT;
      while (!rapprocher());
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
      on_pour_rpi = 1;
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
    {
      lastSend = millis();

      CAN_TX_msg.id = 0x003 + num_carte;
      CAN_TX_msg.len = 1;
      CAN_TX_msg.buf[0] = on_pour_rpi;

      CAN_TX_msg.id = 0x109;
      CAN_TX_msg.len = 1;
      CAN_TX_msg.buf[0] = verif_action;
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