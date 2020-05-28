//
// Created by danielyeste on 28/5/20.
//

#ifndef PAE_DYN_APP_MOTORS_H
#define PAE_DYN_APP_MOTORS_H
#include <stdbool.h>
#include <stdint.h>

void endlessTurn();
int dyn_left_motor_control_endlessTurn(uint8_t id,uint16_t setSpeed);
int dyn_right_motor_control_endlessTurn(uint8_t id,uint16_t setSpeed);
int dyn_right_motor_read(uint8_t id,uint16_t *val);
int dyn_left_motor_read(uint8_t id,uint16_t *val);
int dyn_right_motor_control(uint8_t id,uint8_t *val);
int dyn_left_motor_control(uint8_t id,uint8_t *val);
void bitStringControl(int status, uint8_t *bitString);
#endif //PAE_DYN_APP_MOTORS_H
