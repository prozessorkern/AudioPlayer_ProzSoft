#include "led.h"
#include "gpio.h"

#include "pcm1755.h"

//LED Muster
uint32_t led_pattern[5] = {
	0b00000000000000000000000000000000,
	0b11111111111111111111111111111111,
	0b11111111000000001111111100000000,
	0b11110000111100001111000011110000,
	0b10101011001100110010101000000000
};

led_t led_pattern_memory = LED_OFF;


void led_set(led_t pattern)
{
	led_pattern_memory = pattern;
}

void led_process(void)
{
	static uint8_t led_index = 0;
	static uint16_t prescaler = 0;

	if(prescaler < 9)
	{
		prescaler ++;
	}

	else
	{
		prescaler = 0;

		if(led_pattern[led_pattern_memory] & (0x00000001 << led_index) )
		{
			HAL_GPIO_WritePin(LED_PIN_GPIO_Port, LED_PIN_Pin, GPIO_PIN_SET);
		}

		else
		{
			HAL_GPIO_WritePin(LED_PIN_GPIO_Port, LED_PIN_Pin, GPIO_PIN_RESET);
		}

		if(led_index < 31)
		{
			led_index ++;
		}

		else
		{
			led_index = 0;

		}

	}

}
