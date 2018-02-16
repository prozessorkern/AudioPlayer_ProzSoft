/*
 * pcm1755.h
 *
 *  Created on: 31.12.2015
 *      Author: OneeChan
 */

#ifndef INC_PCM1755_H_
#define INC_PCM1755_H_

#include "stm32f4xx_hal.h"

typedef enum speakerSelect_t
{
    SPEAKER_LEFT   = 0x00,
    SPEAKER_RIGHT   = 0x01,
    SPEAKER_BOTH   = 0xFF
} speakerSelect_t;

typedef enum pcmRegisterIndex_t
{
	ATTENUATOR_LEFT = 16,
	ATTENUATOR_RIGHT = 17,
	MUTE_OVERSAMPLING_RESET = 18,
	OPERATION_CONTROL = 19,
	FORMAT_FILTER_CONTROL = 20,
	ZERO_PHASE_CONTROL = 22
} pcmRegisterIndex_t;

typedef enum pcmStates_t
{
	PCM_WAIT,
	PCM_SEND_ADRESS,
	PCM_STROBE_ADRESS,
	PCM_SEND_DATA,
	PCM_STROBE_DATA

} pcmStates_t;


void pcm_send_command(pcmRegisterIndex_t index, uint8_t data);
void pcm_init(void);
void pcm_mute(speakerSelect_t speaker);
void pcm_unmute(speakerSelect_t speaker);
void pcm_set_volume(speakerSelect_t speaker, uint8_t volume);
void pcm_send_process(void);


/*Defines*/

#define PCM_COMMAND_MEMORY_SIZE	10
#define PCM_COMMAND_REGISTER	0
#define PCM_COMMAND_DATA		1

//REGISTER 18
#define PCM_MUTE_LEFT 			0b00000001
#define PCM_MUTE_RIGHT 			0b00000010
#define PCM_OVERSAMPLING 		0b01000000
#define PCM_RESET 				0b10000000
//REGISTER 19
#define PCM_DAC_CONTROL_LEFT 	0b00000001
#define PCM_DAC_CONTROL_RIGHT 	0b00000010
#define PCM_DEEMPHASIS 			0b00010000
#define PCM_DMF_32				0b01000000
#define PCM_DMF_48				0b00100000
#define PCM_DMF_44				0b00000000
//REGISTER 20
#define PCM_FORMAT_24RIGHT		0b00000000
#define PCM_FORMAT_20RIGHT		0b00000001
#define PCM_FORMAT_18RIGHT		0b00000010
#define PCM_FORMAT_16RIGHT		0b00000011
#define PCM_FORMAT_I2S			0b00000100
#define PCM_FORMAT_LEFT			0b00000101
#define PCM_FILTER_SLOW			0b00100000
#define PCM_FILTER_SHARP		0b00000000
//REGISTER 22
#define PCM_PHASE_NORMAL		0b00000000
#define PCM_PHASE_INV			0b00000001
#define PCM_ZERO_LOW_ACTIVE		0b00000010
#define PCM_ZERO_HIGH_ACTIVE	0b00000000
#define PCM_ZERO_INDEPENDENT	0b00000000
#define PCM_ZERO_COMMON			0b00000100

#endif /* INC_PCM1755_H_ */
