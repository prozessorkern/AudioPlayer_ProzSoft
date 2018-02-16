
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OPERATING_DATA_H
#define __OPERATING_DATA_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

void operating_data_init(void);
void operating_data_process(void);
void operating_data_button_pressed(uint8_t button_num);

#endif
