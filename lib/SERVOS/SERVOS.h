#ifndef _SERVOS_H
#define _SERVOS_H

#include <HardwareSerial.h>
#include <HerkulexServo.h>

// Définitions d'ID et positions
#define HERKULEX_BROADCAST_ID 0xFE
#define SERVO_SERRAGE 0x00
#define SERVO_ROTATION 0x00

#define ATTRAPE 550 //350
#define RELACHE 400
#define ANGLE0 276
#define ANGLE90 552
#define ANGLE180 829
#define HAUT 500
#define BAS 300


// Broches série
#define PIN_SW_RX PB7
#define PIN_SW_TX PB6

// Déclarations extern des objets définis dans SERVOS.cpp
extern HardwareSerial Serial1;
extern HerkulexServoBus herkulex_bus;
extern HerkulexServo all_servo;
extern HerkulexServo servo_rota;
extern HerkulexServo servo_serr;

// Prototypes de fonctions
void serrer(void);
void desserrer(void);
void tourner(void);
void rapprocher(void);
void ecarter(void);

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

#endif // _SERVOS_H
