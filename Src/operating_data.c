#include "operating_data.h"
#include "data_store_global.h"
#include "i2s.h"
#include "tim.h"

operationData_t operation_data;

void operating_data_init(void)
{
	data_store_get_operation_data(&operation_data);
	MX_TIM4_Init();
}

//alle 1 s aufrufen
void operating_data_process(void)
{
	static uint32_t counter = 0;

	counter ++;

	//10 Minuten
	if(counter >= 600)
	{
		if(I2S_get_state() == I2S_STOPPED)
		{
			operation_data.operating_time += (counter / 60);

			counter = counter % 60;

			data_store_set_operation_data(&operation_data);
			data_store_global_save();
		}
	}

}

void operating_data_button_pressed(uint8_t button_num)
{
	if(button_num > 0 && button_num < 5)
	{
		operation_data.buttons_pressed[button_num - 1] ++;
	}
}
