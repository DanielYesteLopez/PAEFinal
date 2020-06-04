#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <posicion.h>
#include <stdio.h>
#include <time.h>

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
int main(void) {
    pthread_t tid, jid;
    uint8_t tmp, tmp_left, tmp_front, tmp_right;

    //Init queue for TX/RX data
    init_queue(&q_tx);
    init_queue(&q_rx);

    //Start thread for dynamixel module emulation
    // Passing the room information to the dyn_emu thread
    pthread_create(&tid, NULL, dyn_emu, (void *) datos_habitacion);
    pthread_create(&jid, NULL, joystick_emu, (void *) &jid);
    void endlessTurn(){
        printf("Setting Endless turn \n");
        dyn_left_motor_control_endlessTurn(1,0);
        dyn_right_motor_control_endlessTurn(2,0);
    }
    /*Motor 1->Izquierda
     * Motor2 -2 Derecha*/
    /*void testSensors(){
        printf("TEST SENSOR: Obtenim distancies sensor");
        uint8_t tmp_left = 15, tmp_front = 10, tmp_right = 15; //Distancies prefixades
        struct data_values distancies;
        printf("\n");
        distancies = sensor_distance(tmp_left, tmp_front, tmp_right);  //Cridem al mÃ¨tode que calcula la distancia que llegeix el sensor, i guardem els resultats a "distancies".

        //Mostrem com predefinim un valor pel sensor, perÃ² despres de cridar als mÃ¨todes aquests valors canvien a 0 ja que al no tenir robot la distancia es 0. PerÃ² aixÃ² demostra que la comunicaciÃ³ amb el mÃ²duls Ã©s la correcta.
        printf("Distancia dreta prefixada: ");
        printf("%d", tmp_right);  //Veiem que primer tenim valor 10
        printf("\n");
        printf("Distancia esquerre llegida pel sensor: ");
        printf("%d", distancies.right_data); //PerÃ² despres de llegir el sensor, el valor canvia a 0, ja que en aquet moment no tenim el robot, perÃ² si estigues donaria la distÃ ncia corresponenet
        printf("\n");
        tmp_right = distancies.right_data;  //Passem el valor real calculat pel sensor al tmp i fem l'assert
        assert(tmp_right == 0);  //Com no tenim el robot per mesurar, el RxPacket ens retorna distancia 0.

        //Fem el mateix que pel sensor dret, amb front
        printf("Distancia front prefixada: ");
        printf("%d", tmp_front);   //Mostrem la distÃ ncia prefixada
        printf("\n");
        printf("Distancia front llegida pel sensor: ");
        printf("%d", distancies.front_data);
        printf("\n");
        tmp_front = distancies.front_data;
        assert(tmp_front == 0);
        //Igual que amb els altres sensors, mostrem el valor predefinit i com canvia despres de mirar el sensor
        printf("Distancia esquerre prefixada: ");
        printf("%d", tmp_left);
        printf("\n");
        printf("Distancia esquerre llegida pel sensor: ");
        printf("%d", distancies.left_data);
        printf("\n");
        tmp_left = distancies.left_data;
        assert(tmp_left == 0);
    }
    testSensors(); //Testejem sensors*/
    /*Primero estableceremos el modo endless turn para los dos motores,
     * accediendo a los registros que corresponden a CW y CCW y enviándoles la instrucción 0.*/
    endlessTurn();
    /*bitStringMovement es la cadena de strings que iremos manipulando por tal de
     * enviar los  dos paquetes de 8 bits que se necesitan para controlar tanto la dirección
     * como la velocidad en el modo endless turn ya establecido.*/
    uint8_t bitStringMovement[2]={0x4,0XFF};
    /*Por defecto, el robot se moverá hacia adelante, así que mandaremos los respectivos
     * paquetes a ambos motores con el string de bits correspondiendte*/
    bitStringControl(1,bitStringMovement);
    dyn_right_motor_control(2,bitStringMovement);
    dyn_left_motor_control(1,bitStringMovement);
    /*Empieza el programa entrega final*/
    //Testing some high level function
    printf("\nSetting LED to 0 \n");
    dyn_led_control(1, 0);
    printf("\nGetting LED value \n");
    dyn_led_read(1, &tmp);
    assert(tmp == 0);
    printf("\nSetting LED to 1 \n");
    dyn_led_control(1, 1);
    printf("\nGetting LED value \n");
    dyn_led_read(1, &tmp);
    assert(tmp == 1);

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
            break;
        }
        //Get_estado(&estado, &estado_anterior);
        dyn_left_distance(3,&tmp_left);
        /*printf("Lectura sensor esquerre ");
        printf("%d", tmp_left);*/

        dyn_right_distance(3,&tmp_right);
        /*printf("Lectura sensor dret ");
        printf("%d", tmp_right);*/

        dyn_front_distance(3,&tmp_front);
        /*printf("Lectura sensor centre ");
        printf("%d", tmp_front);*/
        if(!firstWall){
            if((tmp_left<=10) || (tmp_front<=10) || (tmp_right<=10)){
                firstWall = true;
            }
        }
        /*Pared frontal con pared a la izquierda.
         * Giro Derecha*/
        if(tmp_front <= 10 && tmp_right>50){
            /*Decrease speed*/
            bitStringControl(3,bitStringMovement);
            bitStringControl(3,bitStringMovement);
            dyn_right_motor_control(2,bitStringMovement);
            dyn_left_motor_control(1,bitStringMovement);
            /*Giro derecha*/
            bitStringControl(2,bitStringMovement);
            dyn_right_motor_control(2,bitStringMovement);
            while(tmp_front <= 20 && tmp_left >= 6){
                dyn_front_distance(3,&tmp_front);
                dyn_left_distance(3,&tmp_left);
            }
            /*Recto*/
            bitStringControl(1,bitStringMovement);
            dyn_right_motor_control(2,bitStringMovement);
            /*Increase speed*/
            bitStringControl(4,bitStringMovement);
            bitStringControl(4,bitStringMovement);
            dyn_right_motor_control(2,bitStringMovement);
            dyn_left_motor_control(1,bitStringMovement);
            lastWall = 2;

        }
        if(tmp_left<= 6){
            /*Decrease speed*/
            bitStringControl(3,bitStringMovement);
            bitStringControl(3,bitStringMovement);
            dyn_right_motor_control(2,bitStringMovement);
            dyn_left_motor_control(1,bitStringMovement);
            /*Giro derecha*/
            bitStringControl(2,bitStringMovement);
            dyn_right_motor_control(2,bitStringMovement);
            while(tmp_left<= 8 && tmp_front >= 8){
                dyn_left_distance(3,&tmp_left);
                dyn_front_distance(3,&tmp_front);
            }
            /*Frontal*/
            bitStringControl(1,bitStringMovement);
            dyn_right_motor_control(2,bitStringMovement);
            /*Increase speed*/
            bitStringControl(4,bitStringMovement);
            bitStringControl(4,bitStringMovement);
            dyn_right_motor_control(2,bitStringMovement);
            dyn_left_motor_control(1,bitStringMovement);
            lastWall = 2;

        }
        if(tmp_front>15 && tmp_left>15 && tmp_right>15 && firstWall){
            if(lastWall == 0){

            }else if(lastWall == 1){

            }else if(lastWall == 2){
                /*Decrease speed*/
                bitStringControl(3,bitStringMovement);
                bitStringControl(3,bitStringMovement);
                dyn_left_motor_control(1,bitStringMovement);
                dyn_right_motor_control(2,bitStringMovement);
                /*Turn left*/
                bitStringControl(2,bitStringMovement);
                dyn_left_motor_control(1,bitStringMovement);
                while(tmp_front>30) {
                    dyn_front_distance(3,&tmp_front);
                }
                /*Straight*/
                bitStringControl(1,bitStringMovement);
                dyn_left_motor_control(1,bitStringMovement);
                /*Increase speed*/
                bitStringControl(4,bitStringMovement);
                dyn_right_motor_control(2,bitStringMovement);
                bitStringControl(4,bitStringMovement);
                dyn_left_motor_control(1,bitStringMovement);
                lastWall = 2;
            }
        }


        /*if (estado != estado_anterior) {
            Set_estado_anterior(estado);
            printf("estado = %d\n", estado);
            switch (estado) {
                case Sw1:
                    printf("Boton Sw1 ('a') apretado\n");
                    dyn_led_control(1, 1); //Probaremos de encender el led del motor 2
                    printf("\n");
                    break;
                case Sw2:
                    printf("Boton Sw2 ('s') apretado\n");
                    dyn_led_control(1, 0); //Probaremos de apagar el led del motor 2
                    printf("\n");
                    break;
                case Up:

                    break;
                case Down:

                    break;
                case Left:
                    //Comprobaremos si detectamos las esquinas de la pared izquierda:
                    printf("Esquina inferior izquierda:\n");
                    printf("(1, 1): %d (fuera pared)\n", obstaculo(1, 1, datos_habitacion));
                    printf("(0, 1): %d (pared izq.)\n", obstaculo(0, 1, datos_habitacion));
                    printf("(1, 0): %d (pared del.)\n", obstaculo(1, 0, datos_habitacion));
                    printf("(0, 0): %d (esquina)\n", obstaculo(0, 0, datos_habitacion));
                    printf("Esquina superior izquierda:\n");
                    printf("(1, 4094): %d (fuera pared)\n", obstaculo(1, 4094, datos_habitacion));
                    printf("(0, 4094): %d (pared izq.)\n", obstaculo(0, 4094, datos_habitacion));
                    printf("(1, 4095): %d (pared fondo.)\n", obstaculo(1, 4095, datos_habitacion));
                    printf("(0, 4095): %d (esquina)\n", obstaculo(0, 4095, datos_habitacion));
                    break;
                case Right:
                    //Comprobaremos si detectamos las esquinas de la pared derecha:
                    printf("Esquina inferior derecha:\n");
                    printf("(4094, 1): %d (fuera pared)\n", obstaculo(4094, 1, datos_habitacion));
                    printf("(4094, 0): %d (pared del.)\n", obstaculo(4094, 0, datos_habitacion));
                    printf("(4095, 1): %d (pared der.)\n", obstaculo(4095, 1, datos_habitacion));
                    printf("(4095, 0): %d (esquina)\n", obstaculo(4095, 0, datos_habitacion));
                    printf("Esquina superior derecha:\n");
                    printf("(4094, 4094): %d (fuera pared)\n", obstaculo(4094, 4094, datos_habitacion));
                    printf("(4094, 4095): %d (pared fondo)\n", obstaculo(4094, 4095, datos_habitacion));
                    printf("(4095, 4094): %d (pared der.)\n", obstaculo(4095, 4094, datos_habitacion));
                    printf("(4095, 4095): %d (esquina)\n", obstaculo(4095, 4095, datos_habitacion));
                    break;
                case Center:

                    break;
                case Quit:
                    printf("Adios!\n");
                    break;
                    //etc, etc...
            }
            fflush(stdout);
        }*/

    }
    //Signal the emulation thread to stop
    pthread_kill(tid, SIGTERM);
    pthread_kill(jid, SIGTERM);
    printf("Programa terminado\n");
    fflush(stdout);
}
