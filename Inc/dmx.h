
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __dmx_H
#define __dmx_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"


 typedef enum dmxMode_t
 {
	 DMX_MODE_RECEIVE,
	 DMX_MODE_TRANSMIT
 } dmxMode_t;

#define DMX_MAX_CHANNEL		512
#define DMX_BYTE0			0x00

void dmx_init(void);
void dmx_start_transmit(void);
void dmx_transmit_finish_handler(void);
void dmx_start_receive(void);
void dmx_receive_reset_handler(void);

extern volatile uint8_t *dmx_data;
extern volatile uint16_t dmx_received_sets;


#ifdef __cplusplus
}
#endif
#endif /*__ dmx_H */
