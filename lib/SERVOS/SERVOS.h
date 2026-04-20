#ifndef _SERVOS_H
#define _SERVOS_H

#include <HardwareSerial.h>
#include <HerkulexServo.h>

// Définitions d'ID et positions
#define HERKULEX_BROADCAST_ID 0xFE
#define SERVO_SERRAGE 0x00
#define SERVO_ROTATION 0x00

#define position_ecarter            300
#define position_ecarter_pince      350
#define position_defaut             300
#define position_retourner          785
#define position_attraper           600
#define position_attraper_interieur 610
#define position_rapprocher         275

// Broches série
#define PIN_SW_RX PB7
#define PIN_SW_TX PB6

// Déclarations extern des objets définis dans SERVOS.cpp
extern HardwareSerial Serial1;
extern HerkulexServoBus herkulex_bus;
extern HerkulexServo all_servo;

// Variables d'état des pinces (1 = objet présent, 0 = vide)
extern int objet_pince_int;
extern int objet_pince_ext;

// Prototypes de fonctions
void serrer(int pince);
void desserrer(int pince);
void tourner(int pince);
void rapprocher(void);
void ecarter(void);

void change_id(uint8_t id, HerkulexServo old_, HerkulexServo new_);
void init_serial_1_for_herkulex();
void test_herkulex();
void test_connexion();
int  detect_id(bool activate);
void rotation_moteur(void);
void display_servo_position(void);
int16_t get_servo_pos(HerkulexServo servo);
void restart_all_servo(void);
void get_all_servo_pos(
    short *pos_servo_pivot_gauche,
    short *pos_servo_pivot_droit,
    short *pos_servo_aimant_droit,
    short *pos_servo_aimant_gauche,
    short *pos_servo_aimant_centre,
    short *pos_servo_pince,
    short *pos_servo_pivot_pince);

// Nouveaux prototypes
void maj_etat_pince(int pince, int etat);   // Met à jour l'état des pinces selon l'action
void check_herkulex_errors(void);           // Surveillance erreurs + reboot auto Herkulex

#endif // _SERVOS_H