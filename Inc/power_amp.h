#ifndef POWER_AMP_H_
#define POWER_AMP_H_

#include "stm32f4xx_hal.h"

//in 10ms Schritten
#define AMP_MUTE_DELAY 10

void amp_start(void);
void amp_stop(void);
void_amp_process(void);

#endif /* INC_PCM1755_H_ */
