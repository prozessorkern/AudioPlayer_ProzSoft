#include "button.h"
#include "gpio.h"
#include "config_port.h"
#include "operating_data.h"

buttonDescriptor_t buttons[] =
{
	{0,0,0},
	{BUTTON_1_GPIO_Port, BUTTON_1_Pin, 0},
	{BUTTON_2_GPIO_Port, BUTTON_2_Pin, 0},
	{BUTTON_3_GPIO_Port, BUTTON_3_Pin, 0},
	{BUTTON_4_GPIO_Port, BUTTON_4_Pin, 0}
};

uint8_t pressed_button = 0;
buttonMode_t mode = BUTTON_MODE_NORMAL;

buttonPressed_t button_get(void)
{
	buttonPressed_t button;

	button.button_pressed = pressed_button;

	if(pressed_button == 0)
	{
		button.pressed_time = 0;
	}
	else
	{
		button.pressed_time = buttons[pressed_button].pressed_counter;
	}

	return button;
}

void button_clear(void)
{
	pressed_button = 0;
}

buttonMode_t button_get_mode(void)
{
	return mode;
}

void button_process(void)
{
	uint8_t counter = 0;
	uint8_t pressed_button_num = 0;
	uint8_t new_pressed_button = 0;

	for(counter = 1; counter <= BUTTON_NUM; counter++)
	{

		if((buttons[counter].port->IDR & buttons[counter].pin) == 0)
		{
			if(buttons[counter].pressed_counter < 50000)
			{
				buttons[counter].pressed_counter += 10;
			}

			pressed_button_num ++;
			new_pressed_button = counter;
		}

		else
		{
			buttons[counter].pressed_counter = 0;
		}

	}

	if(buttons[3].pressed_counter > MODE_CHANGE_DELAY && buttons[4].pressed_counter > MODE_CHANGE_DELAY)
	{
		if(mode != BUTTON_MODE_UART)
		{
			mode = BUTTON_MODE_UART;

			config_port_init();

			pressed_button = 0;

		}

	}

	else
	{
		if(mode != BUTTON_MODE_NORMAL)
		{
			mode = BUTTON_MODE_NORMAL;

			config_port_terminate();
		}

		if(pressed_button_num > 1)
		{
			pressed_button = 0;
		}

		else if(pressed_button_num == 1 && buttons[new_pressed_button].pressed_counter == BUTTON_PRESSED_TIME)
		{
			pressed_button = new_pressed_button;

			operating_data_button_pressed(new_pressed_button);

		}

		else
		{
			pressed_button = 0;
		}
	}

}
