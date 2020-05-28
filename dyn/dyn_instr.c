/*
 * hal_dyn.c
 *
 *  Created on: 18 mar. 2020
 *      Author: droma
 *
 * This file implements the different dynamixel instructions
 */

#include "dyn/dyn_instr.h"
#include "dyn/dyn_frames.h"
#include <math.h>

/**
 * Single byte write instruction
 *
 * This function sends a write instruction for a single address position
 * to a given dynamixel module.
 *
 * @param[in] module_id Id of the dynamixel module
 * @param[in] reg_addr Address where the write is performed
 * @param[in] reg_write_val Value written to the previous address
 * @return Error code to be treated at higher levels.
 */
int dyn_write_byte(uint8_t module_id, DYN_REG_t reg_addr, uint8_t reg_write_val) {
    uint8_t parameters[2];
    struct RxReturn reply;

    parameters[0] = reg_addr;
    parameters[1] = reg_write_val;
    reply = RxTxPacket(module_id, 2, DYN_INSTR__WRITE, parameters);
    return (reply.tx_err << 1) | reply.time_out;
}

/**
 * Single byte read instruction
 *
 * This function sends a read instruction for a single address position
 * to a given dynamixel module.
 *
 * @param[in] module_id Id of the dynamixel module
 * @param[in] reg_addr Address where the read is performed
 * @param[out] reg_read_val Pointer where the read value is stored
 * @return Error code to be treated at higher levels.
 */
int dyn_read_byte(uint8_t module_id, DYN_REG_t reg_addr, uint8_t *reg_read_val) {
    uint8_t parameters[2];
    struct RxReturn reply;

    parameters[0] = reg_addr;
    parameters[1] = 1;
    reply = RxTxPacket(module_id, 2, DYN_INSTR__READ, parameters);
    *reg_read_val = reply.StatusPacket[5];

    return (reply.tx_err << 1) | reply.time_out;
}

/**
 * Multi-byte write instruction
 *
 * This function sends a write instruction starting at a given address position
 * with a given length for a dynamixel module.
 *
 * @param[in] module_id Id of the dynamixel module
 * @param[in] reg_addr Address where the write is performed
 * @param[in] val Pointer to the byte array to be written
 * @param[in] len Number of position to be written
 * @return Error code to be treated at higher levels.
 */
int dyn_write(uint8_t module_id, DYN_REG_t reg_addr, uint8_t *val, uint8_t len) {
    //TODO: Implement multiposition write
    uint8_t parameters[2],longParameters[3];
    struct RxReturn reply;
    /*Misma lógica que hemos utilizado en dyn_write_byte*/
    if(len == 1){
        parameters[0] = reg_addr;
        parameters[1] = *val;
        reply = RxTxPacket(module_id, 2, DYN_INSTR__WRITE, parameters);
        return (reply.tx_err < 1) | reply.time_out;
        /*Si los parámetros que pasamos son mayores que 1, entonces pasaremos
         * dos paquetes de instrucciones a los módulos
         * del robot. No se contempla más de dos ya que no se ha detectado
         * ningún registro que abarque más de 16 bits*/
    }else{
        longParameters[0] = reg_addr;
        longParameters[1] = *val;
        longParameters[2] = *(val+1);//Segundo paquete
        reply = RxTxPacket(module_id, 3, DYN_INSTR__WRITE, longParameters);//Indicamos número de parámetros como 3
        return (reply.tx_err < 1) | reply.time_out;	}
}
/**
 * Single byte read instruction
 *
 * This function sends a read instruction for a single address position
 * to a given dynamixel module.
 *
 * @param[in] module_id Id of the dynamixel module
 * @param[in] reg_addr Address where the read is performed
 * @param[out] reg_read_val Pointer where the read value is stored
 * @return Error code to be treated at higher levels.
 **/
int dyn_read(uint8_t module_id, DYN_REG_t reg_addr, uint16_t* reg_read_val) {
    //uint8_t module_id, DYN_REG_t reg_addr, uint8_t *val, uint8_t len
    /*Leemos la velocidad de los motores. Para ello, indicamos que se deben
     * leer los 16 bits y calculamos la velocidad a partir de esto, sumando
     * los dos últimos bits de con los del segundo paquete para devolver la velocidad.*/
    uint8_t parameters[3];
    struct RxReturn reply;
    uint8_t mask = (1 << 2) - 1;
    parameters[0] = reg_addr;
    parameters[1] = 0x2;
    reply = RxTxPacket(module_id, 2, DYN_INSTR__READ, parameters);
    *reg_read_val = reply.StatusPacket[5];
    uint8_t lsbits = *reg_read_val& mask;
    uint8_t nbitsInLsbits =(int)(log2(lsbits)+1);//Calculamos cuantos bits a 1 hay en los 2 primeros lsbits
    *reg_read_val = ((reply.StatusPacket[6])<<nbitsInLsbits|lsbits);

    return (reply.tx_err < 1) | reply.time_out;
}


