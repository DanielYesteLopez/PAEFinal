/*
 * dyn_sensor.h
 *
 *  Created on: 18 mar. 2020
 *      Author: droma
 */

#ifndef DYN_SENSOR_H_
#define DYN_SENSOR_H_
#include <stdio.h>
#include <stdint.h>

struct data_values {
    uint8_t  left_data;
    uint8_t front_data;
    uint8_t right_data;
};

//Sensor llum
int dyn_left_sensor_light(uint8_t id, uint8_t *val);
int dyn_front_sensor_light(uint8_t id, uint8_t *val);
int dyn_right_sensor_light(uint8_t id, uint8_t *val);
//Sensor infraroig
int dyn_left_distance(uint8_t id, uint8_t *val);
int dyn_front_distance(uint8_t id, uint8_t *val);
int dyn_right_distance(uint8_t id, uint8_t *val);

extern void calc_distance(uint8_t tmp1, uint8_t tmp2, uint8_t tmp3);




#endif /* DYN_SENSOR_H_ */
