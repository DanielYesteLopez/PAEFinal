#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <posicion.h>
#include <stdio.h>
#include <time.h>
#include <zconf.h>
#include <math.h>

#include "main.h"
#include "dyn/dyn_app_common.h"
#include "dyn_test/dyn_emu.h"
#include "dyn_test/b_queue.h"
#include "joystick_emu/joystick.h"
#include "dyn/dyn_app_sensor.h"
#include "dyn/dyn_app_motors.h"
#include "habitacion_001.h"

uint8_t estado = Ninguno, estado_anterior = Ninguno, finalizar = 0;
uint32_t indice;

/**
 * main.c
 */
void endlessTurn() {
    printf("Setting Endless turn \n");
    dyn_left_motor_control_endlessTurn(1, 0);
    dyn_right_motor_control_endlessTurn(2, 0);
}

int calculoDistanciaGir(int sensorFrontal, int sensorLateral) {
    return (sensorFrontal * sensorLateral) /(sqrt(pow(sensorFrontal, 2) + pow(sensorLateral, 2))) ;
}

int main(void) {
    pthread_t tid, jid;
    uint8_t tmp_left, tmp_front, tmp_right;

    //Init queue for TX/RX data
    init_queue(&q_tx);
    init_queue(&q_rx);

    //Start thread for dynamixel module emulation
    // Passing the room information to the dyn_emu thread
    pthread_create(&tid, NULL, dyn_emu, (void *) datos_habitacion);
    pthread_create(&jid, NULL, joystick_emu, (void *) &jid);
    /*Motor 1->Izquierda
     * Motor2 -2 Derecha*/
    /*Primero estableceremos el modo endless turn para los dos motores,
     * accediendo a los registros que corresponden a CW y CCW y enviándoles la instrucción 0.*/
    endlessTurn();
    /*bitStringMovement es la cadena de strings que iremos manipulando por tal de
     * enviar los  dos paquetes de 8 bits que se necesitan para controlar tanto la dirección
     * como la velocidad en el modo endless turn ya establecido.*/
    uint8_t bitStringMovement[2] = {0x4, 0XFF};
    /*Por defecto, el robot se moverá hacia adelante, así que mandaremos los respectivos
     * paquetes a ambos motores con el string de bits correspondiendte*/
    bitStringControl(1, bitStringMovement);
    dyn_right_motor_control(2, bitStringMovement);
    dyn_left_motor_control(1, bitStringMovement);
    /*Empieza el programa entrega final*/

    printf("\n************************\n");
    printf("Test passed successfully\n");

    printf("\nDimensiones habitacion %d ancho x %d largo mm2\n", ANCHO, LARGO);
    printf("En memoria: %I64u B = %I64u MiB\n", sizeof(datos_habitacion), sizeof(datos_habitacion) >> 20);

    printf("Pulsar 'q' para terminar, qualquier tecla para seguir\n");
    fflush(stdout);//	return 0;
    /*Booleano para referirnos a si hemos encontrado la primera pared*/
    bool firstWall = false;
    /*Última pared que hemos detectado
     * 0->Arriba
     * 1->Derecha
     * 2->Izquierda
     * 3->Abajo*/
    int lastWall = 0;
    int calculateExactTurn = 0;
    int c = 0;
    /*                   ->Pared Delante-> Giro derecha
     * Pared Izquierda
     *                   ->Pared Derecha + Pared delante ->Hacia adetras ***
     *
     *                   ->Pared Derecha -> Giro Izquierda
     * Pared Delante     ->(Giramos derecha) Podemos escoger según la última pared
     *                   ->Pared Izquierda ->Giro Derecha
     *                   ->  Pared Delante-> Giro izquierda
     *Pared Derecha
     *                  ->Pared Derecha + Pared delante ->Hacia adetras ***
     *                      Si !findWall{
     *                          sigue el curso
     *                      }
     * No detectamos pared Si findWall {
     *                          lastWall
     *                      }
     * */
    /*
     * i->UP
     * j->Left
     * k->Center
     * l->Right
     * m->Down*/

    while (estado != Quit) {
        if (simulator_finished) {
            estado = Quit;
        } else {
            Get_estado(&estado, &estado_anterior);
            dyn_left_distance(3, &tmp_left);


            dyn_right_distance(3, &tmp_right);

            dyn_front_distance(3, &tmp_front);
            if (!firstWall) {
                /*Decrease speed*/
                bitStringControl(3, bitStringMovement);
                bitStringControl(3, bitStringMovement);
                dyn_left_motor_control(1, bitStringMovement);
                dyn_right_motor_control(2, bitStringMovement);
                /*Giro izquierda*/
                bitStringControl(2, bitStringMovement);
                dyn_left_motor_control(1, bitStringMovement);
                while (tmp_front > 230) {
                    dyn_front_distance(3, &tmp_front);
                }
                firstWall = true;
                /*Frontal*/
                bitStringControl(1, bitStringMovement);
                dyn_left_motor_control(1, bitStringMovement);
                /*Increase speed*/
                bitStringControl(4, bitStringMovement);
                bitStringControl(4, bitStringMovement);
                dyn_right_motor_control(2, bitStringMovement);
                dyn_left_motor_control(1, bitStringMovement);
            }
            /*Encontramos pared delante*/
            if ((tmp_front <= 10) && (tmp_left > 10) && (tmp_right > 10)) {
                /*Decrease speed*/
                bitStringControl(3, bitStringMovement);
                bitStringControl(3, bitStringMovement);
                dyn_right_motor_control(2, bitStringMovement);
                dyn_left_motor_control(1, bitStringMovement);
                /*Turn right*/
                bitStringControl(2, bitStringMovement);
                dyn_right_motor_control(2, bitStringMovement);
                while (tmp_front <= 20) {
                    dyn_front_distance(3, &tmp_front);
                    dyn_left_distance(3, &tmp_left);
                }
                if (tmp_front > 100) {
                    /*Straight*/
                    bitStringControl(1, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    /*Increase speed*/
                    bitStringControl(4, bitStringMovement);
                    bitStringControl(4, bitStringMovement);
                    dyn_left_motor_control(1, bitStringMovement);

                } else {
                    calculateExactTurn = calculoDistanciaGir(tmp_front, tmp_left);
                    while (tmp_left < calculateExactTurn) {
                        dyn_left_distance(3, &tmp_left);
                    }
                    /*Straight*/
                    bitStringControl(1, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    /*Increase speed*/
                    bitStringControl(4, bitStringMovement);
                    bitStringControl(4, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    dyn_left_motor_control(1, bitStringMovement);
                    lastWall = 2;
                }

            }
                /*Encontramos objeto izquierda*/

            else if (tmp_left <= 10) {
                    /*Decrease speed*/
                    bitStringControl(3, bitStringMovement);
                    bitStringControl(3, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    while (tmp_left < 10) {
                        dyn_left_distance(3, &tmp_left);
                    }
                    /*Increase speed*/
                    bitStringControl(4, bitStringMovement);
                    bitStringControl(4, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    lastWall = 2;

            }
                /*Encontramos pared derecha*/
            else if (tmp_right <= 10) {
                /*Decrease speed*/
                bitStringControl(3, bitStringMovement);
                bitStringControl(3, bitStringMovement);
                dyn_left_motor_control(1, bitStringMovement);
                while (tmp_right <= 10) {
                    dyn_right_distance(3, &tmp_right);
                }
                /*Increase speed*/
                bitStringControl(4, bitStringMovement);
                bitStringControl(4, bitStringMovement);
                dyn_left_motor_control(1, bitStringMovement);
                lastWall = 1;
            } else if ((tmp_front <= 10) && (tmp_left <= 10) && (tmp_right <= 10)) {
                bitStringControl(2, bitStringMovement);
                dyn_right_motor_control(2, bitStringMovement);
                dyn_left_motor_control(1, bitStringMovement);
                while (tmp_right < 15 || tmp_left < 15) {
                    dyn_left_distance(3, &tmp_left);
                    dyn_right_distance(3, &tmp_right);
                }
                /*Turn left to find the wall*/
                bitStringControl(2, bitStringMovement);
                dyn_left_motor_control(1, bitStringMovement);
            }
            if (tmp_front > 15 && tmp_left > 15 && tmp_right > 15) {
                if (lastWall == 0) {

                } else if (lastWall == 1) {
                    /*Decrease speed*/
                    bitStringControl(3, bitStringMovement);
                    bitStringControl(3, bitStringMovement);
                    dyn_left_motor_control(1, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    /*Turn right*/
                    bitStringControl(2, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    while (tmp_front > 20) {
                        dyn_front_distance(3, &tmp_front);
                    }
                    /*Straight*/
                    bitStringControl(1, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    /*Increase speed*/
                    bitStringControl(4, bitStringMovement);
                    bitStringControl(4, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    dyn_left_motor_control(1, bitStringMovement);
                    lastWall = 1;

                } else if (lastWall == 2) {
                    /*Decrease speed*/
                    bitStringControl(3, bitStringMovement);
                    bitStringControl(3, bitStringMovement);
                    dyn_left_motor_control(1, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    /*Turn left*/
                    bitStringControl(2, bitStringMovement);
                    dyn_left_motor_control(1, bitStringMovement);
                    while (tmp_front > 20) {
                        dyn_front_distance(3, &tmp_front);
                    }
                    /*Straight*/
                    bitStringControl(1, bitStringMovement);
                    dyn_left_motor_control(1, bitStringMovement);
                    /*Increase speed*/
                    bitStringControl(4, bitStringMovement);
                    bitStringControl(4, bitStringMovement);
                    dyn_right_motor_control(2, bitStringMovement);
                    dyn_left_motor_control(1, bitStringMovement);
                    lastWall = 2;
                }
            }

        }

    }
    //Signal the emulation thread to stop
    pthread_kill(tid, SIGTERM);
    pthread_kill(jid, SIGTERM);
    printf("Programa terminado\n");
    fflush(stdout);
}
