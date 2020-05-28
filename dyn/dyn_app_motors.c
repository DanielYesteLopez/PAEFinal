//
// Created by danielyeste on 28/5/20.
//
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "dyn/dyn_instr.h"
#include "dyn/dyn_app_motors.h"
//0->Delante
//1->Atras
/*Funciones de lectura que llaman a dyn_read. Podríamos haber creado sólo una pero hemos
 * optado por escribir una función para cada motor y así hacer el código más extensible y entendible*/

int dyn_right_motor_read(uint8_t id,uint16_t *val){
    return dyn_read(id,DYN_REG_SPEED,val);//
}

int dyn_left_motor_read(uint8_t id,uint16_t *val){
    return dyn_read(id,DYN_REG_SPEED,val);
}

/*
 * Establishes endless turn to both motors so they will keep moving continually*/
/*Envíamos la instrucción 0 a los registros CCW y CW de manera que establecemos el modo endless turn explicado en la documentación*/
int dyn_left_motor_control_endlessTurn(uint8_t id,uint16_t instructionEndlessTurn){
    return dyn_write_byte(id,DYN_CCW_REGISTER,instructionEndlessTurn),dyn_write_byte(id,DYN_CW_REGISTER,instructionEndlessTurn);
}

int dyn_right_motor_control_endlessTurn(uint8_t id,uint16_t instructionEndlessTurn){
    return dyn_write_byte(id,DYN_CCW_REGISTER,instructionEndlessTurn),dyn_write_byte(id,DYN_CW_REGISTER,instructionEndlessTurn);
}
/*Establishes forward movement*/
/*Aunque no sea necesario ya que se podria manejar los dos motores con una sola función y simplemente cambiamos las ID,
 * hemos considerado que quizá sería útil diferenciarlos de alguna manera así que hemos creado dos funciones porque
 * quizás en un futuro nos es útil para extender el programa.*/
int dyn_right_motor_control(uint8_t id,uint8_t *val){
    return dyn_write(id,DYN_REG_SPEED,val,2);
}
int dyn_left_motor_control(uint8_t id,uint8_t *val){
    return dyn_write(id,DYN_REG_SPEED,val,2);
}

void bitStringControl(int status, uint8_t *bitString){
    /*Realizaremos los cambios necesarios en el string de bits
     * por tal de modificarlo. Para ello utilizaremos operaciones lógicas y
     * máscaras de bits*/
    uint8_t mask = 0;
    uint8_t lsbits = 0;
    if (status == 1){
        /*Forward*/
        /*Cogeremos con una máscara los dos bits de menos peso del primer paquete
         * por tal de no modificar la velocidad, y simplemente utilizaremos esos bits
         * que nunca modificarán el bit 11 para dejar este a 0*/
        mask = (1 << 2) - 1;
        lsbits = *bitString&mask;
        *bitString= lsbits;

    }else if (status == 2){
        /*Backward*/
        /*Cogeremos con una máscara los dos bits de menos peso del primer paquete
         * por tal de no modificar la velocidad y utilizaremos una OR para poner
         * el bit 11 a 1*/
        mask = (1 << 2) - 1;
        lsbits = *bitString&mask;
        *bitString= lsbits|0x04;
        /*IMPORTANTE:
         * Todas las modificaciones de velocidad se hacen desplazando sólo un bit a izquierda o derecha*/
    }else if (status == 3){
        /*Decrease speed*/
        /*Comprobaremos los dos bits de menos peso del primer paquete y, en caso de que sean 0,
         * manipularemos el segundo paquete y viceversa. La velocidad mínima será 1.*/
        mask = (1 << 2) - 1;
        lsbits = *bitString& mask;
        if(lsbits <= 0){
            if((*(bitString+1) >>0x01) <= 0){
                printf("Minimum speed raised \n");
            }else{
                *(bitString+1) = *(bitString+1) >>0x01;
            }
        }else{
            lsbits = lsbits>>0x01;
            if(*bitString > 3){
                *bitString = *bitString&lsbits;
                *bitString = *bitString|0x04;
            }else{
                *bitString = *bitString&lsbits;
            }
        }

    }else if (status == 4){
        /*Increase speed*/
        /*Comprobaremos los dos bits de menos peso del primer paquete y, en caso de que sea 0, pondremos
         * su valor a 1 por tal de aumentar la velocidad. Si no,modificaremos el segundo paquete.*/
        mask = (1 << 2) - 1;
        lsbits = *bitString& mask;
        if(lsbits>= 3){
            printf("Maximum speed raised \n");
        }else{
            if(*(bitString+1) >= 255){
                lsbits = lsbits<<0x01;
                lsbits = lsbits|0x01;
                *bitString = *bitString|lsbits;
            }else{
                *(bitString+1) = *(bitString+1) <<0x01;
                *(bitString+1) = *(bitString+1)|0x1;
            }
        }
    }
}

