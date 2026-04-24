#include <Arduino.h>
#include "SERVOS.h"
#include <STM32FreeRTOS.h>

HardwareSerial Serial1(USART1);
HerkulexServoBus herkulex_bus(Serial1);
// Initialisation de la liaison série matérielle sur l'UART1

HerkulexServo all_servo(herkulex_bus, HERKULEX_BROADCAST_ID);

// pince avant 0
/*HerkulexServo servo_attraper_interieur(herkulex_bus, 0x0B);
HerkulexServo servo_retourner_interieur(herkulex_bus, 0x02);
HerkulexServo servo_retourner_exterieur(herkulex_bus, 0x03);
HerkulexServo servo_attraper_exterieur(herkulex_bus, 0x04);
HerkulexServo servo_ecarter_pince(herkulex_bus, 0x05);*/
// pince arrière 1
HerkulexServo servo_attraper_interieur(herkulex_bus, 0x06);
HerkulexServo servo_retourner_interieur(herkulex_bus, 0x07);
HerkulexServo servo_retourner_exterieur(herkulex_bus, 0x09);
HerkulexServo servo_attraper_exterieur(herkulex_bus, 0x08);
HerkulexServo servo_ecarter_pince(herkulex_bus, 0x0A);

// Variables d'état des pinces (1 = objet présent, 0 = vide)
int objet_pince_int = 0;
int objet_pince_ext = 0;

// Tableau des servos surveillés par check_herkulex_errors
static HerkulexServo* servos_surveilles[] = {
  &servo_attraper_interieur,
  &servo_attraper_exterieur,
  &servo_retourner_interieur,
  &servo_retourner_exterieur,
  &servo_ecarter_pince
};
static const char* noms_servos[] = {
  "ATT_INT", "ATT_EXT", "ROT_INT", "ROT_EXT", "ECART"
};
static const uint8_t NB_SERVOS = 5;

// Variables pour gérer l'intervalle de mise à jour
unsigned long last_update = 0; // Stocke le temps de la dernière mise à jour
unsigned long now         = 0; // Stocke le temps actuel
bool toggle               = false; // Booléen pour alterner entre deux positions
char etat_rota            = 0; // État actuel de la rotation du servo (0 -> 0°, 1 -> 90°, 2 -> 180°)

// Variables pour la position du servo
int pos, pos_angle;
void curseur(void){
  servo_ecarter_pince.setPosition(560, 100, HerkulexLed::Yellow);
}
void init_serial_1_for_herkulex()
{
  Serial1.setRx(PIN_SW_RX); // Associe la broche RX à l'UART1
  Serial1.setTx(PIN_SW_TX); // Associe la broche TX à l'UART1
  Serial1.begin(115200);    // Initialise la communication série à 115200 bauds

  servo_attraper_interieur.setLedColor(HerkulexLed::Blue);
  servo_attraper_interieur.setTorqueOn();
  delay(100);
  servo_attraper_exterieur.setLedColor(HerkulexLed::Blue);
  servo_attraper_exterieur.setTorqueOn();
  delay(100);
  servo_retourner_interieur.setLedColor(HerkulexLed::Green);
  servo_retourner_interieur.setTorqueOn();
  delay(100);
  servo_retourner_exterieur.setLedColor(HerkulexLed::Green);
  servo_retourner_exterieur.setTorqueOn();
  delay(100);

  servo_ecarter_pince.setLedColor(HerkulexLed::Cyan);
  servo_ecarter_pince.setTorqueOn();
    // Vide le buffer RX après toutes les initialisations
  while (Serial1.available()) Serial1.read();
}

// ─────────────────────────────────────────────────────────────
// Met à jour l'état des pinces selon l'action effectuée.
// pince : 1 = intérieure, 2 = extérieure, 12 = les deux
// etat  : 1 = objet présent, 0 = vide
// ─────────────────────────────────────────────────────────────
void maj_etat_pince(int pince, int etat)
{
  if (pince == 1 || pince == 12) objet_pince_int = etat;
  if (pince == 2 || pince == 12) objet_pince_ext = etat;
  //Serial.printf("[PINCE] int=%d  ext=%d\n", objet_pince_int, objet_pince_ext);
}
void restart_retourner(void){
  servo_retourner_exterieur.reboot();
  delay(300);
  servo_retourner_interieur.reboot();
  delay(300);
  servo_retourner_exterieur.setTorqueOn();
  delay(300);
  servo_retourner_interieur.setTorqueOn();
}
// ─────────────────────────────────────────────────────────────
// Surveillance des erreurs Herkulex (à appeler dans la boucle).
// Vérifie chaque servo toutes les 500 ms.
// Si une erreur est détectée : reboot du servo fautif uniquement
// + réactivation du couple + LED rouge pour identification.
// ─────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────
// Surveillance passive : surveille uniquement les bits de statut
// renvoyés naturellement lors des moves, sans polluer le bus
// avec des getStatus() actifs qui corrompent le buffer RX.
// À appeler uniquement hors action comme avant.
// ─────────────────────────────────────────────────────────────
void check_herkulex_errors(void)
{
  static unsigned long derniere_verif = 0;
  static uint8_t index_servo = 0;

  if (millis() - derniere_verif < 500) return;
  derniere_verif = millis();

  uint8_t status_raw = servos_surveilles[index_servo]
                         ->readRam(HerkulexRamRegister::StatusError);

  // Vide le buffer RX de l'UART pour éviter que les octets
  // de réponse résiduels corrompent la prochaine commande de mouvement
  while (Serial1.available()) Serial1.read();

  HerkulexStatusError err = static_cast<HerkulexStatusError>(status_raw);

  if (err != HerkulexStatusError::None)
  {
    Serial.printf("[HERKULEX] Erreur servo %s : 0x%02X -> reboot\n",
                  noms_servos[index_servo], status_raw);

    if ((err & HerkulexStatusError::Overload)         != HerkulexStatusError::None)
      Serial.println("  -> Surcharge");
    if ((err & HerkulexStatusError::TemperatureLimit) != HerkulexStatusError::None)
      Serial.println("  -> Temperature excessive");
    if ((err & HerkulexStatusError::InputVoltage)     != HerkulexStatusError::None)
      Serial.println("  -> Tension hors limites");
    if ((err & HerkulexStatusError::DriverFault)      != HerkulexStatusError::None)
      Serial.println("  -> Defaut driver");

    servos_surveilles[index_servo]->reboot();
    delay(300);
    servos_surveilles[index_servo]->setTorqueOn();
    servos_surveilles[index_servo]->setLedColor(HerkulexLed::Red);

    // Flush aussi après le reboot
    while (Serial1.available()) Serial1.read();
  }

  index_servo = (index_servo + 1) % NB_SERVOS;
}

void tourner(int pince)
{
  static int etat_rotae = 1, position,position_ext;
  switch (etat_rotae)
  {
  case 0:
    position = position_defaut;
    position_ext = position_defaut;
    etat_rotae = 1;
    break;
  case 1:
    position = position_retourner;
    position_ext = position_retourner_exterieur;
    etat_rotae = 0;
    break;
  }
  switch (pince)
  {
  case 1:
    servo_retourner_interieur.setPosition(position, 100, HerkulexLed::Blue); // Position 90°
    break;
  case 2:
    servo_retourner_exterieur.setPosition(position_ext, 100, HerkulexLed::Green); // Ouvre la pince
    break;
  case 12:
    servo_retourner_interieur.setPosition(position, 100, HerkulexLed::Blue); // Position 90°
    servo_retourner_exterieur.setPosition(position_ext, 100, HerkulexLed::Blue); // Position 90°
    break;
  default:
    break;
  }
}

void serrer(int pince)
{
  switch (pince)
  {
  case 1:
    servo_attraper_interieur.setPosition(position_attraper_interieur, 100, HerkulexLed::Green); // Ferme la pince
    break;
  case 2:
    servo_attraper_exterieur.setPosition(position_attraper, 100, HerkulexLed::Green); // Ferme la pince
    break;
  case 12:
    servo_attraper_interieur.setPosition(position_attraper_interieur, 100, HerkulexLed::Green); // Ferme la pince
    servo_attraper_exterieur.setPosition(position_attraper,           100, HerkulexLed::Green); // Ferme la pince
    break;
  default:
    break;
  }
}

void desserrer(int pince)
{
  switch (pince)
  {
  case 1:
    servo_attraper_interieur.setPosition(position_ecarter, 100, HerkulexLed::Blue); // Ouvre la pince
    break;
  case 2:
    servo_attraper_exterieur.setPosition(position_ecarter, 100, HerkulexLed::Green); // Ouvre la pince
    break;
  case 12:
    servo_attraper_interieur.setPosition(position_ecarter, 100, HerkulexLed::Blue); // Ouvre la pince
    servo_attraper_exterieur.setPosition(position_ecarter, 100, HerkulexLed::Blue); // Ouvre la pince
    break;
  default:
    break;
  }
}

void ecarter(void)
{
  servo_ecarter_pince.setPosition(position_ecarter_pince, 60, HerkulexLed::Green); // Ouvre la pince
  herkulex_bus.executeMove();
  // servo_ecarter_pince.reboot();
}

void rapprocher(void)
{
  servo_ecarter_pince.setPosition(position_rapprocher, 60, HerkulexLed::Blue); // Ferme la pince
  herkulex_bus.executeMove();
  // servo_ecarter_pince.reboot();
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
  servo_attraper_interieur.reboot();
  delay(300);
  servo_attraper_exterieur.reboot();
  delay(300);
  servo_retourner_interieur.reboot();
  delay(300);
  servo_retourner_exterieur.reboot();
  delay(300);
  servo_ecarter_pince.reboot();
  delay(300);
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
  // Serial.print(1);
  Serial.printf("%d", old_.readEep(HerkulexEepRegister::ID));
  // Serial.print(1);
  delay(300); // un peu plus long
  // Serial.print(1);
  new_.reboot();
  delay(300); // pour être certain
  // Serial.print(1);
  new_.setLedColor(HerkulexLed::White);

}