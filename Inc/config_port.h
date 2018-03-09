/*
 * pcm1755.h
 *
 *  Created on: 31.12.2015
 *      Author: OneeChan
 */

#ifndef INC_CONFIG_PORT_H_
#define INC_CONFIG_PORT_H_

#include "stm32f4xx_hal.h"

#define CONFIG_PORT_FUNCTION_NUM			23
#define CONFIG_PORT_RECEIVE_BUFFER_SIZE		64
#define CONFIG_PORT_SEND_BUFFER_SIZE		64

#define CR	0x0D
#define LF	0x0A

#define CONFIG_PORT_ACK 	0xFF
#define CONFIG_PORT_ERROR	0x00

// Funktionspointer für command port Funktionen
typedef void ( *configPortFunction_t )( uint8_t *data, uint8_t length );

//  Tabelleneintrag
typedef struct {
   const uint8_t       				config_port_idx;	    //Befehls Index
   const configPortFunction_t    	config_port_function;    //Funktionspointer
} configPortTable_t;

void config_port_init(void);
void config_port_terminate(void);
void config_port_receive_handler(uint8_t data);
void config_port_send_handler(void);

#endif /* INC_PCM1755_H_ */
