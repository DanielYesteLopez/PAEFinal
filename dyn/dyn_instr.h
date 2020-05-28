/*
 * hal_dyn.h
 *
 *  Created on: 18 mar. 2020
 *      Author: droma
 */

#ifndef DYN_INSTR_H_
#define DYN_INSTR_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum _dyn_instr_type {
    DYN_INSTR__READ = 2,
    DYN_INSTR__WRITE = 3,
} DYN_INSTR_t;

typedef enum _dyn_reg_type {
    DYN_REG__LED = 0x19,
    //Registres motors
    DYN_CCW_REGISTER = 0X08,
    DYN_CW_REGISTER = 0X06,
    DYN_REG_SPEED = 0x20,
    DYN_REGMAX = 0x32,
    //Registres sensor infraroig
    DYN_REG_LEFTDISTANCE = 0x1A,
    DYN_REG_FRONTDISTANCE = 0x1B,
    DYN_REG_RIGHTDISTANCE = 0x1C,
    IDSENSOR_INFRA = 0x03,
    //Registres sensor llum
    DYN_REG_LEFTLIGHT = 0x1D,
    DYN_REG_FRONTLIGHT = 0x1E,
    DYN_REG_RIGHTLIGHT = 0x1F,
    //Registre sensor so
    DYN_REG_SOUND = 0x25,
    DYN_REG__GOAL_SPEED_L = 0x20,
    DYN_REG__GOAL_SPEED_H = 0x021,
    DYN_REG__IR_LEFT = 0x1A,
    DYN_REG__IR_CENTER = 0x1B,
    DYN_REG__IR_RIGHT = 0x1C,
} DYN_REG_t;

int dyn_write_byte(uint8_t module_id, DYN_REG_t reg_addr, uint8_t reg_write_val);

int dyn_read_byte(uint8_t module_id, DYN_REG_t reg_addr, uint8_t *reg_read_val);
int dyn_read(uint8_t module_id, DYN_REG_t reg_addr, uint16_t* reg_read_val);
int dyn_write(uint8_t module_id, DYN_REG_t reg_addr, uint8_t *val, uint8_t len);

#endif /* DYN_INSTR_H_ */
