/*
 * dyn_sensor.c
 *
 *  Created on: 18 mar. 2020
 *      Author: droma
 *
 * TODO: High-level functions like "distance_wall_front", etc
 * TODO: Generate another file for motors, with functions like "move_forward", etc
 */
#include "dyn_app_sensor.h"
#include "dyn_instr.h"
#include "dyn_frames.h"
#include "dyn_app_common.h"
//Llegim sensor d'infraroig, distancies
int dyn_left_distance(uint8_t id, uint8_t *val){
    return dyn_read_byte(id,DYN_REG_LEFTDISTANCE,val);
}

int dyn_front_distance(uint8_t id, uint8_t *val){
    return dyn_read_byte(id,DYN_REG_FRONTDISTANCE,val);
}

int dyn_right_distance(uint8_t id, uint8_t *val){
    return dyn_read_byte(id, DYN_REG_RIGHTDISTANCE, val);
}

/*struct data_values sensor_distance(uint8_t tmp1, uint8_t tmp2, uint8_t tmp3){  //Mètode per llegir valors del sensor. Està en forma de struct ja que tenim 3 valors de distàncies dieferents. Per esquerre, centre i dreta.
    struct data_values read;

    printf("Obtenim distància sensor esquerre\n");
    dyn_left_distance(IDSENSOR_INFRA, &tmp1);  //Llegim el valor del sensor
    read.left_data = tmp1;  //Guardem el valor que indica el sensor en la variable esquerre

    printf("Obtenim distància sensor centre\n");
    dyn_front_distance(IDSENSOR_INFRA, &tmp2);  //Llegim el valor del sensor
    read.front_data = tmp2;  //Guardem valor

    printf("Obtenim distància sensor dret\n");
    dyn_right_distance(IDSENSOR_INFRA, &tmp3);  //Llegim
    read.right_data = tmp3; //Guardem valor

    return read;  //Retornem l'struct amb les distancies corresponents

}*/





