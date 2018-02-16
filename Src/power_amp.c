#include "power_amp.h"
#include "gpio.h"

static void amp_mute(void);
static void amp_unmute(void);
static void amp_stby(void);
static void amp_wakeupt(void);

uint8_t amp_state = 0;
int16_t amp_counter = 0;

void amp_start(void)
{
	amp_state = 1;
	amp_counter = 0;

	amp_wakeupt();
}

void amp_stop(void)
{
	amp_state = 0;
	amp_counter = 0;

	amp_mute();
}

void amp_process(void)
{

	if(amp_state == 0)
	{
		if(amp_counter < AMP_MUTE_DELAY)
		{
			amp_counter ++;

		}

		else if(amp_counter == AMP_MUTE_DELAY)
		{
			amp_stby();
			amp_counter ++;
		}
	}

	else
	{
		if(amp_counter < AMP_MUTE_DELAY)
		{
			amp_counter ++;

		}

		else if(amp_counter == AMP_MUTE_DELAY)
		{
			amp_unmute();
			amp_counter ++;
		}
	}

}

void amp_mute(void)
{
	HAL_GPIO_WritePin(AMP_MUTE_GPIO_Port, AMP_MUTE_Pin, GPIO_PIN_RESET);
}

void amp_unmute(void)
{
	HAL_GPIO_WritePin(AMP_MUTE_GPIO_Port, AMP_MUTE_Pin, GPIO_PIN_SET);
}

void amp_stby(void)
{
	HAL_GPIO_WritePin(AMP_STBY_GPIO_Port, AMP_STBY_Pin, GPIO_PIN_RESET);
}

void amp_wakeupt(void)
{
	HAL_GPIO_WritePin(AMP_STBY_GPIO_Port, AMP_STBY_Pin, GPIO_PIN_SET);
}
