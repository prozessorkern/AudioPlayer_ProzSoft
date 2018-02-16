/*
 * pcm1755.h
 *
 *  Created on: 31.12.2015
 *      Author: OneeChan
 */

#ifndef INC_PLAYER_H_
#define INC_PLAYER_H_

#include "stm32f4xx_hal.h"

void player_init(void);
void player_load_file(uint8_t file_num);
void player_play(void);
void player_pause(void);
void player_stop(void);
void player_start_dmx_record(uint8_t file_num);
void player_eof_callback(void);
void player_frame_sent_callback(void);
void player_process(void);

#endif /* INC_PCM1755_H_ */
