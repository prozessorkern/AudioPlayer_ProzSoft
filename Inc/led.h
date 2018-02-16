
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __led_H
#define __led_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

typedef enum led_t
{
	LED_OFF = 0,
	LED_ON = 1,
	LED_FLASH = 2,
	LED_FLASH_FAST = 3,
	LED_SOS = 4
} led_t;

void led_set(led_t pattern);
void led_process(void);

#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */
