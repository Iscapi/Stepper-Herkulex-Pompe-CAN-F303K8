#ifndef _HERKULEX_H
#define _HERKULEX_H
#include <HardwareSerial.h>
#include <HerkulexServo.h>

// Définition des identifiants des servos Herkulex utilisés dans le projet
#define HERKULEX_BROADCAST_ID 0xFE // ID de diffusion pour tous les servos
#define SERVO_SERRAGE 0x04 
#define SERVO_ROTATION 0x06

// Définition des positions et angles pour les mouvements des servos
#define ATTRAPE 200 
#define RELACHE 800 // à vérifier





void change_id(uint8_t id, HerkulexServo old_, HerkulexServo new_);
void init_serial_1_for_herkulex();
void test_herkulex();
void test_connexion();
int detect_id(bool activate);
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

#endif

// Définition des broches utilisées pour la communication série avec les servos Herkulex
#define PIN_SW_RX PB7 // Broche utilisée pour la réception (RX)
#define PIN_SW_TX PB6 // Broche utilisée pour la transmission (TX)


/*
    J'ai décalé les créations d'objet et define ici pour y avoir accès partout
    Comme ça je peux aussi m'en servir dans le main au lieu de recréer des objets
    et créer un problème avec 2 fois le même objet mais qui n'a pas le même nom
*/


// Création d'un objet servo avec l'adresse de diffusion (HERKULEX_BROADCAST_ID signifie tous les servos connectés)
