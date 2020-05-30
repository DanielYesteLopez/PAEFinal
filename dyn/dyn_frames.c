/*
 * hal_uart.c
 *
 *  Created on: 18 mar. 2020
 *      Author: droma
 *
 * Dynamixel framing layer
 */

#include "dyn_frames.h"

#ifndef __MSP432P401R__

#include "hal_dyn_uart/hal_dyn_uart_emu.h"
#include "fake_msp.h"

#define f_TxUAC2 TxUAC2_emu
#define f_Sentit_Dades_Tx Sentit_Dades_Tx_emu
#define f_Sentit_Dades_Rx Sentit_Dades_Rx_emu
#define f_rx_uart_byte rx_uart_byte_emu

#else
#include "hal_dyn_uart/hal_dyn_uart.h"
#include "msp.h"

#define f_TxUAC2 TxUAC2
#define f_Sentit_Dades_Tx Sentit_Dades_Tx
#define f_Sentit_Dades_Rx Sentit_Dades_Rx
#define f_Activa_Timer_TimeOut Activa_Timer_TimeOut
#define f_rx_uart_byte rx_uart_byte

#endif
//TxPacket()  3 par�metres: ID del Dynamixel, Mida dels par�metres, Instruction byte. torna la mida del "Return packet"
byte TxPacket(byte bID, byte bParameterLength, byte bInstruction,
              const byte * Parametros) {
    byte bCount, bCheckSum, bPacketLength;
    byte TxBuffer[32];
    f_Sentit_Dades_Tx();  //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Transmetre)
    TxBuffer[0] = 0xff;    //Primers 2 bytes que indiquen inici de trama FF, FF.
    TxBuffer[1] = 0xff;
    TxBuffer[2] = bID;         //ID del m�dul al que volem enviar el missatge
    TxBuffer[3] = bParameterLength + 2; //Length(Parameter,Instruction,Checksum)
    TxBuffer[4] = bInstruction;    //Instrucci� que enviem al M�dul

    //TODO: La instrucci� no ha de poder modificar les primeres 5 instruccions
    if ((Parametros[0]<5)&&(bInstruction==3)){//si se intenta escribir en una direccion <= 0x05,
        //emitir mensaje de error de direccion prohibida:
        //halLcdPrintLine(error,8,NORMAL_TEXT); El meu eclipse no l'agafa
        printf("Direcció prohibida");
        //y salir de la funcion
        return 0;
    }
    for (bCount = 0; bCount < bParameterLength; bCount++) //Comencem a generar la trama que hem d�enviar
    {
        TxBuffer[bCount + 5] = Parametros[bCount];
        /*Enviamos los paquetes a partir del 5 ya que antes hemos enviado lo ya expuesto en las posiciones
         * 0,1,2,3,4 las cuales forman la primera parte del paquete y no son parte de la instruccion que se le envia
         * al registro*/
    }
    bCheckSum = 0;
    bPacketLength = bParameterLength + 4 + 2;
    for (bCount = 2; bCount < bPacketLength - 1; bCount++) //C�lcul del checksum
    {
        bCheckSum += TxBuffer[bCount];
        /*Se calcula el checksum como se expone en la documentación a partir del parametro ID , ya
         * que los dos primeros, 0xFF,0xFF no se tienen en cuenta*/
    }
    TxBuffer[bCount] = ~bCheckSum;         //Escriu el Checksum (complement a 1)
    for (bCount = 0; bCount < bPacketLength; bCount++) //Aquest bucle �s el que envia la trama al M�dul Robot
    {
        f_TxUAC2(TxBuffer[bCount]);
        /*Se envía el paquete txbuffer(el paquete entero) a la simulación del modulo del robot*/
    }
    while ((UCA2STATW & UCBUSY)) {
    };   //Espera fins que s�ha transm�s el �ltim byte
    f_Sentit_Dades_Rx(); //Posem la l�nia de dades en Rx perqu� el m�dul Dynamixel envia resposta
    /*La llamada a la función superior parece inútil ya que ya la utilizas cuando se llama a la
     * función de RXPacket, habilitando la línea half duplex en Rx*/
    return (bPacketLength);
}

struct RxReturn RxPacket(void) {
    struct RxReturn respuesta;
    byte bCount;
    byte bCheckSum = 0;
    respuesta.time_out = false;
    respuesta.idx = 0;
    f_Sentit_Dades_Rx();   //Ponemos la linea half duplex en Rx
    //f_Activa_Timer_TimeOut();
    for (bCount = 0; bCount < 4; bCount++) {
        f_rx_uart_byte(&respuesta);
    } //fin del for
    if (!respuesta.time_out) {
        for (bCount = 0; bCount < respuesta.StatusPacket[3]; bCount++) {
            f_rx_uart_byte(&respuesta);
        } //fin del for
    }
    //TODO: Decode packet and verify checksum
    /*Calculamos el checksum y lo ponemos en el paquete correspondiente dentro del StatusPacket*/
    for (bCount = 2; bCount < respuesta.StatusPacket[3] - 1; bCount++) //C�lcul del checksum
    {
        bCheckSum += respuesta.StatusPacket[bCount];
    }
    /*Cálculo del checksum a partir de los primeros 16 bits como se ha hecho anteriormente, hasta el final del
     * paquete el cual miramos con la medida de este*/
    respuesta.StatusPacket[bCount] = ~bCheckSum;
    return respuesta;
}


/**
 * Perform a full read/write transaction
 *
 * This function will send an instruction frame to the dynamixel module
 * and the following status frame
 *
 * @param[in] bID Id of the dynamixel module
 * @param[in] bParameterLength Number of parameters to send
 * @param[in] bInstruction Instruction type of the frame
 * @param[in] Parametros Parameters of the TX frame
 * @return Returns a RxReturn struct with the information of the reply
 */
struct RxReturn RxTxPacket(byte bID, byte bParameterLength, byte bInstruction,
                           const byte * Parametros) {
    /*Envíamos los datos al robot con un TxPacket y luego obtenemos estos mediante un RxPacket que viene de vuelta.
     * Escritura completa en el módulo.*/
    struct RxReturn respuesta;
    TxPacket(bID, bParameterLength, bInstruction, Parametros);
    respuesta = RxPacket();
    return respuesta;
}
