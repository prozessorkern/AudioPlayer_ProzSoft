
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __button_H
#define __button_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "player.h"

#define BUTTON_NUM	4
#define BUTTON_PRESSED_TIME 100
#define BUTTON_STOP_TIME 500
#define MODE_CHANGE_DELAY 500

typedef enum buttonMode_t
{
	BUTTON_MODE_NORMAL,
	BUTTON_MODE_UART
} buttonMode_t;

typedef struct
{
	uint8_t button_pressed;
	uint16_t pressed_time;
}buttonPressed_t;

typedef struct
{
	GPIO_TypeDef *port;
	uint16_t pin;
	uint16_t pressed_counter;
}buttonDescriptor_t;

buttonPressed_t button_get(void);
void button_clear(void);
buttonMode_t button_get_mode(void);
void button_process(void);


#ifdef __cplusplus
}
#endif

#endif
