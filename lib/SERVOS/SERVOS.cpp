#include <Arduino.h>
#include "SERVOS.h"
#include <STM32FreeRTOS.h>

HardwareSerial Serial1(USART1);
HerkulexServoBus herkulex_bus(Serial1);
// Initialisation de la liaison série matérielle sur l'UART1

HerkulexServo all_servo(herkulex_bus, HERKULEX_BROADCAST_ID);
HerkulexServo servo_rota(herkulex_bus, SERVO_ROTATION);
HerkulexServo servo_serr(herkulex_bus, SERVO_SERRAGE);

// Variables pour gérer l'intervalle de mise à jour
unsigned long last_update = 0; // Stocke le temps de la dernière mise à jour
unsigned long now = 0;         // Stocke le temps actuel
bool toggle = false;           // Booléen pour alterner entre deux positions
char etat_rota = 0;            // État actuel de la rotation du servo (0 -> 0°, 1 -> 90°, 2 -> 180°)

// Variables pour la position du servo
int pos, pos_angle;

void init_serial_1_for_herkulex()
{
  Serial1.setRx(PIN_SW_RX); // Associe la broche RX à l'UART1
  Serial1.setTx(PIN_SW_TX); // Associe la broche TX à l'UART1
  Serial1.begin(115200);    // Initialise la communication série à 115200 bauds
  all_servo.setTorqueOn();  // Active le couple du servo (mise sous tension)
}

void monter(void)
{
  all_servo.setPosition(HAUT, 150, HerkulexLed::Green); // Lève la pince
  herkulex_bus.executeMove();
  delay(200); // Attente de 225 ms
}

void descendre(void)
{
  all_servo.setPosition(BAS, 150, HerkulexLed::Green); // descend la pince
  herkulex_bus.executeMove();
  delay(200); // Attente de 225 ms
}

void serrer(void)
{
  servo_serr.setPosition(ATTRAPE, 50, HerkulexLed::Green); // Ouvre la pince
  herkulex_bus.executeMove();
  delay(200); // Attente de 225 ms
}

void desserrer(void)
{
  servo_serr.setPosition(RELACHE, 50, HerkulexLed::Blue); // Ouvre la pince
  herkulex_bus.executeMove();
  delay(200); // Attente de 225 ms
}

void tourner(void)
{
  switch (etat_rota)
  {
  case 0:
    servo_rota.setPosition(ANGLE0, 50, HerkulexLed::Green); // Position 0°
    etat_rota = 1;
    break;
  case 1:
    servo_rota.setPosition(ANGLE180, 50, HerkulexLed::Blue); // Position 90°
    etat_rota = 0;
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

void test_herkulex()
{
  herkulex_bus.update(); // Met à jour les servos connectés
  now = millis();        // Récupère le temps actuel en millisecondes

  // Commenté : récupération de la position du servo
  // pos = all_servo.getPosition();
  // pos_angle = (pos-512)*0.325;  // Conversion en degrés
  // Serial.printf("pos : %4d | %4d °\n", pos, pos_angle);

  // Si 5000 ms (5 secondes) se sont écoulées depuis la dernière mise à jour
  if ((now - last_update) > 5000)
  {
    // called every 1000 ms
    if (toggle)
    {
      // Déplace le servo à -90° en 50 cycles, allume la LED verte
      // 512 - 90°/0.325 = 235
      // all_servo.reboot();
      all_servo.setPosition(512 - 0 / 0.325, 50, HerkulexLed::Green);
      // Possibilité d'ajouter d'autres servos avec différentes positions
      // my_servo_2.setPosition(512-10/0.325, 50, HerkulexLed::Blue);
      // my_servo_3.setPosition(512+30/0.325, 50, HerkulexLed::Yellow);
    }
    else
    {
      // Déplace le servo à +45° en 50 cycles, allume la LED bleue
      // 512 + (45° / 0.325) = 650
      // 512 + 90°/0.325 = 789
      all_servo.setPosition(512 + 90 / 0.325, 100, HerkulexLed::Blue);
      all_servo.setTorqueOn();
      // my_servo_2.setPosition(512+10/0.325, 50, HerkulexLed::Blue);
      // my_servo_3.setPosition(512-30/0.325, 50, HerkulexLed::Yellow);
    }
    last_update = now; // Met à jour le dernier temps d'exécution
    toggle = !toggle;  // Alterne entre les deux positions
  }
}

int detect_id(bool activate)
{
  if (activate)
  {
    herkulex_bus.update(); // Met à jour les servos sur le bus

    uint8_t servos_found = 0;
    // Boucle sur tous les ID possibles (0x00 à 0xFD)
    for (uint8_t id = 0; id <= 0xFD; id++)
    {
      HerkulexPacket resp; // Stocke la réponse du servo
      bool success = herkulex_bus.sendPacketAndReadResponse(resp, id, HerkulexCommand::Stat);
      if (success)
      {
        servos_found++; // Incrémente le compteur si un servo est trouvé
        // Affichage de l'ID au format hexadécimal
        if (id <= 0x0F)
        {
          Serial.print("0");
        } // Ajoute un "0" pour l'alignement des nombres
        Serial.print(id, HEX);
      }
      else
      {
        Serial.print("--");
      } // Affiche "--" si aucun servo n'est détecté
      // Saut de ligne toutes les 15 adresses affichées
      if (((id + 1) % 0x0F) == 0)
      {
        Serial.println();
      }
      else
      {
        Serial.print(" ");
      }
    }
    // Affichage du nombre total de servos trouvés
    Serial.println();
    Serial.println("Done!");
    Serial.print("Found ");
    Serial.print(servos_found);
    Serial.println(" servos.");

    return 0xFD; // Retourne l'adresse de diffusion (broadcast ID)
  }
  else
  {
    return 0;
  } // Retourne 0 si la détection est désactivée
}

// affiche la position en °
void display_servo_position(void)
{
  Serial.println(all_servo.readRam(HerkulexRamRegister::CalibratedPosition));
  Serial.print("Aimant_centre : ");
  // Serial.print(Pivot_pince.getPosition());
}

// Allume la led en bleu pour les herkulex connectées
void test_connexion()
{
  // test pour voir lequel est connectée
  all_servo.setLedColor(HerkulexLed::Green); // allume la led des herkulex connectées
}

// si on veut la position d'un servo en particulier
int16_t get_servo_pos(HerkulexServo servo)
{
  return (servo.getPosition() - 512) * 0.325; // on retrun la position du servo voulu
}

// donne la position de tout les servos en °, range tout des les variables
void get_all_servo_pos(
    short *pos_servo_pivot_gauche,
    short *pos_servo_pivot_droit,
    short *pos_servo_aimant_droit,
    short *pos_servo_aimant_gauche,
    short *pos_servo_aimant_centre,
    short *pos_servo_pince,
    short *pos_servo_pivot_pince)
{
  // range les pos des servos en ° dans les variables
  /**pos_servo_pivot_gauche  = (Pivot_gauche.getPosition() - 512) * 0.325;
   *pos_servo_pivot_droit   = (Pivot_droit.getPosition() - 512) * 0.325;
   *pos_servo_aimant_droit  = (Aimant_droit.getPosition() - 512) * 0.325;
   *pos_servo_aimant_gauche = (Aimant_gauche.getPosition() - 512) * 0.325;
   *pos_servo_aimant_centre = (Aimant_centre.getPosition() - 512) * 0.325;
   *pos_servo_pince = (Pince.getPosition() - 512) * 0.325;
   *pos_servo_pivot_pince   = (Pivot_pince.getPosition() - 512) * 0.325;*/
}

void restart_all_servo(void)
{
  all_servo.reboot();
  vTaskDelay(pdMS_TO_TICKS(225));
}

void change_id(uint8_t id, HerkulexServo old_, HerkulexServo new_)
{

  old_.setLedColor(HerkulexLed::Blue);
  delay(10000);
  Serial.print(1);
  old_.reboot();
  delay(500); // OK
  Serial.print(1);
  // lis pour lever temporairement la protection de la rom
  Serial.printf("%d", old_.readEep(HerkulexEepRegister::ID));
  Serial.print(1);
  old_.writeEep(HerkulexEepRegister::ID, id);
  Serial.print(1);
  Serial.printf("%d", old_.readEep(HerkulexEepRegister::ID));
  Serial.print(1);
  delay(300); // un peu plus long
  Serial.print(1);
  new_.reboot();
  delay(300); // pour être certain
  Serial.print(1);
  new_.setLedColor(HerkulexLed::White);
  while (1)
  {
    if (Serial.available() > 0)
    {
      char c = Serial.read();

      if (c == 'f')
        break;
    }
  }
}