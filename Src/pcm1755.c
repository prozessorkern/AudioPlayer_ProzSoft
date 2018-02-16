/*
 * pcm1755.c
 *
 *  Created on: 29.12.2015
 *      Author: OneeChan
 */

#include "pcm1755.h"
#include "gpio.h"

//Ringpuffer fuer Befehle
uint8_t command_memory[2][PCM_COMMAND_MEMORY_SIZE];
uint8_t command_memory_read_index = 0;
uint8_t command_memory_write_index = 0;

//Register Speicher um Aenderungen maskieren zu koennen
uint8_t register_memory[7];

//Befehl in Befehlspuffer schreiben
void pcm_send_command(uint8_t index, uint8_t data)
{
	command_memory[PCM_COMMAND_REGISTER][command_memory_write_index] = index;
	command_memory[PCM_COMMAND_DATA][command_memory_write_index] = data;

	register_memory[index - 16] = data;

	if(command_memory_write_index < (PCM_COMMAND_MEMORY_SIZE - 1))
	{
		command_memory_write_index ++;
	}

	else
	{
		command_memory_write_index = 0;
	}
}

//Standard Befehle ausfuehren
void pcm_init(void)
{
	pcm_send_command(ATTENUATOR_LEFT, 210);
	pcm_send_command(ATTENUATOR_RIGHT, 210);
	pcm_send_command(MUTE_OVERSAMPLING_RESET, PCM_OVERSAMPLING | PCM_MUTE_LEFT | PCM_MUTE_RIGHT);
	pcm_send_command(OPERATION_CONTROL, 0x00);
	pcm_send_command(FORMAT_FILTER_CONTROL, PCM_FORMAT_I2S | PCM_FILTER_SHARP);
	pcm_send_command(ZERO_PHASE_CONTROL, PCM_PHASE_NORMAL | PCM_ZERO_HIGH_ACTIVE | PCM_ZERO_INDEPENDENT);
}

//Mute
void pcm_mute(speakerSelect_t speaker)
{
	uint8_t temp = register_memory[MUTE_OVERSAMPLING_RESET - 16] &= ~(PCM_MUTE_LEFT | PCM_MUTE_RIGHT);

	if(speaker == SPEAKER_LEFT)
	{
		temp |= PCM_MUTE_LEFT;

	}

	else if(speaker == SPEAKER_RIGHT)
	{
		temp |= PCM_MUTE_RIGHT;
	}

	else
	{
		temp |= PCM_MUTE_LEFT | PCM_MUTE_RIGHT;
	}

	pcm_send_command(MUTE_OVERSAMPLING_RESET, temp );
}

void pcm_unmute(speakerSelect_t speaker)
{
	uint8_t temp = register_memory[MUTE_OVERSAMPLING_RESET - 16];

	if(speaker == SPEAKER_LEFT)
	{
		temp &= ~PCM_MUTE_LEFT;

	}

	else if(speaker == SPEAKER_RIGHT)
	{
		temp &= ~PCM_MUTE_RIGHT;
	}

	else
	{
		temp &= ~(PCM_MUTE_LEFT | PCM_MUTE_RIGHT);
	}

	pcm_send_command(MUTE_OVERSAMPLING_RESET, temp );
}

//Einstellen der Lautstaerke < 128 = MUTE, Attenuation -0,5 dB * (256 - volume)
void pcm_set_volume(speakerSelect_t speaker, uint8_t volume)
{
	if(speaker == SPEAKER_LEFT)
	{
		pcm_send_command(ATTENUATOR_LEFT, volume );

	}

	else if(speaker == SPEAKER_RIGHT)
	{
		pcm_send_command(ATTENUATOR_RIGHT, volume );
	}

	else
	{
		pcm_send_command(ATTENUATOR_LEFT, volume );
		pcm_send_command(ATTENUATOR_RIGHT, volume );
	}
}

//Muss Zyklisch aufgerufen werden, enthaelt State Machine zum handeln der Schnittstelle
void pcm_send_process(void)
{
	static pcmStates_t state = PCM_WAIT;
	static uint8_t bit_counter = 0;


	switch(state)
	{
	//Warten auf neue Befehle
	case PCM_WAIT:

		if(command_memory_read_index != command_memory_write_index)
		{
			state = PCM_SEND_ADRESS;
			bit_counter = 7;
			HAL_GPIO_WritePin(PCM_MLATCH_GPIO_Port, PCM_MLATCH_Pin, GPIO_PIN_RESET);
		}

		break;

	//Adresse Bitweise ausgeben
	case PCM_SEND_ADRESS:

		HAL_GPIO_WritePin(PCM_MCLK_GPIO_Port, PCM_MCLK_Pin, GPIO_PIN_RESET);

		if((command_memory[PCM_COMMAND_REGISTER][command_memory_read_index] >> bit_counter)& 0x01)
		{
			HAL_GPIO_WritePin(PCM_MDATA_GPIO_Port, PCM_MDATA_Pin, GPIO_PIN_SET);
		}

		else
		{
			HAL_GPIO_WritePin(PCM_MDATA_GPIO_Port, PCM_MDATA_Pin, GPIO_PIN_RESET);
		}

		state = PCM_STROBE_ADRESS;

		break;

	//Clock Strobe erzeugen
	case PCM_STROBE_ADRESS:

		HAL_GPIO_WritePin(PCM_MCLK_GPIO_Port, PCM_MCLK_Pin, GPIO_PIN_SET);

		if(bit_counter > 0)
		{
			bit_counter --;
			state = PCM_SEND_ADRESS;
		}

		else
		{
			bit_counter = 7;
			state = PCM_SEND_DATA;
		}

		break;

	//Daten Bitweise ausgeben
	case PCM_SEND_DATA:

		HAL_GPIO_WritePin(PCM_MCLK_GPIO_Port, PCM_MCLK_Pin, GPIO_PIN_RESET);

		if((command_memory[PCM_COMMAND_DATA][command_memory_read_index] >> bit_counter)& 0x01)
		{
			HAL_GPIO_WritePin(PCM_MDATA_GPIO_Port, PCM_MDATA_Pin, GPIO_PIN_SET);
		}

		else
		{
			HAL_GPIO_WritePin(PCM_MDATA_GPIO_Port, PCM_MDATA_Pin, GPIO_PIN_RESET);
		}

		state = PCM_STROBE_DATA;

		break;

	//Clock Strobe erzeugen
	case PCM_STROBE_DATA:

		//Clock Leitung aktivieren
		HAL_GPIO_WritePin(PCM_MCLK_GPIO_Port, PCM_MCLK_Pin, GPIO_PIN_SET);

		//bits weiterzaehlen
		if(bit_counter > 0)
		{
			bit_counter --;
			state = PCM_SEND_DATA;
		}

		//Uebertragung beendet, Pufferzeuger erhoehen
		else
		{
			bit_counter = 7;
			state = PCM_WAIT;
			HAL_GPIO_WritePin(PCM_MLATCH_GPIO_Port, PCM_MLATCH_Pin, GPIO_PIN_SET);

			if(command_memory_read_index < (PCM_COMMAND_MEMORY_SIZE - 1))
			{
				command_memory_read_index ++;
			}

			else
			{
				command_memory_read_index = 0;
			}
		}

		break;
	default:
		break;
	}

}
