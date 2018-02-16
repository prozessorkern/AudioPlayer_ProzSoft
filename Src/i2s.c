/**
  ******************************************************************************
  * File Name          : I2S.c
  * Description        : This file provides code for the configuration
  *                      of the I2S instances.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "i2s.h"

#include "gpio.h"
#include "dma.h"
#include "player.h"

/* USER CODE BEGIN 0 */
#include "fatfs.h"
#include"pcm1755.h"

FRESULT res;
FIL filePointer;
UINT out;

wavFileDescriptor_t wav_descriptor;
static i2sPlayState_t play_state = I2S_STOP;

uint8_t header[44];
uint16_t in_buffer[I2S_BUFFER_SIZE/2];
uint16_t buffer[I2S_BUFFER_SIZE];

uint8_t dmx_record_active = 0;

static volatile uint8_t eof = 0;


/* USER CODE END 0 */

I2S_HandleTypeDef hi2s2;
DMA_HandleTypeDef hdma_spi2_tx;

/* I2S2 init function */
void MX_I2S2_Init(void)
{

  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILLIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B_EXTENDED;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_44K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  HAL_I2S_Init(&hi2s2);

}

void HAL_I2S_MspInit(I2S_HandleTypeDef* hi2s)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hi2s->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */
    /* Peripheral clock enable */
    __SPI2_CLK_ENABLE();
  
    /**I2S2 GPIO Configuration    
    PC3     ------> I2S2_SD
    PB10     ------> I2S2_CK
    PB12     ------> I2S2_WS
    PC6     ------> I2S2_MCK 
    */
    GPIO_InitStruct.Pin = I2S_DATA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(I2S_DATA_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = I2S_CLK_Pin|I2S_LRCLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = I2S_MCLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(I2S_MCLK_GPIO_Port, &GPIO_InitStruct);

    /* Peripheral DMA init*/
  
    hdma_spi2_tx.Instance = DMA1_Stream4;
    hdma_spi2_tx.Init.Channel = DMA_CHANNEL_0;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_spi2_tx.Init.Mode = DMA_CIRCULAR;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_spi2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_spi2_tx);

    DMA1_Stream4->CR |= 0b0010100000000000;

    __HAL_LINKDMA(hi2s,hdmatx,hdma_spi2_tx);

  /* USER CODE BEGIN SPI2_MspInit 1 */

  /* USER CODE END SPI2_MspInit 1 */
  }
}

void HAL_I2S_MspDeInit(I2S_HandleTypeDef* hi2s)
{

  if(hi2s->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __SPI2_CLK_DISABLE();
  
    /**I2S2 GPIO Configuration    
    PC3     ------> I2S2_SD
    PB10     ------> I2S2_CK
    PB12     ------> I2S2_WS
    PC6     ------> I2S2_MCK 
    */
    HAL_GPIO_DeInit(GPIOC, I2S_DATA_Pin|I2S_MCLK_Pin);

    HAL_GPIO_DeInit(GPIOB, I2S_CLK_Pin|I2S_LRCLK_Pin);

    /* Peripheral DMA DeInit*/
    HAL_DMA_DeInit(hi2s->hdmatx);
  }
  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */
void I2S_init_conversion(void)
{
	HAL_I2S_Transmit_DMA(&hi2s2, buffer, I2S_BUFFER_SIZE);

	hi2s2.hdmatx->XferHalfCpltCallback = &HAL_I2S_RxHalfCpltCallback;
}

void I2S_load_file(const uint8_t* string)
{
	uint8_t counter = 0;
	do
	{
		res = f_close(&filePointer);
		counter ++;
	}while((res != FR_OK) && (counter < 5));

	counter = 0;
	do
	{
		res = f_open(&filePointer, (const TCHAR*)string, FA_READ);
		counter ++;
	}while((res != FR_OK) && (counter < 5));

	f_read(&filePointer, &header, 44, &out);

	wav_descriptor.sampe_rate = header[24] + (header[25]<<8) + (header[26]<<16) + (header[27]<<24);
	wav_descriptor.bit_depth = header[34] + (header[35]<<8);
/*
	f_read(&filePointer, &header, 44, &out);
	f_read(&filePointer, &header, 44, &out);
	f_read(&filePointer, &header, 44, &out);
	f_read(&filePointer, &header, 44, &out);
	f_read(&filePointer, &header, 44, &out);
	f_read(&filePointer, &header, 44, &out);
	f_read(&filePointer, &header, 44, &out);
	f_read(&filePointer, &header, 44, &out);
	f_read(&filePointer, &header, 44, &out);*/
}

void I2S_play(void)
{
	play_state = I2S_PLAY;
}

void I2S_stop(void)
{
	uint8_t counter = 0;

	play_state = I2S_STOP;
	dmx_record_active = 0;

	do
	{
		res = f_close(&filePointer);
		counter ++;
	}while((res != FR_OK) && (counter < 5));
}

void I2S_pause(void)
{
	play_state = I2S_PAUSE;
}

void I2S_record_dmx(void)
{
	dmx_record_active = 1;
}

i2sPlayState_t I2S_get_state(void)
{
	return play_state;
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s){

	int i;

	if(play_state == I2S_PLAY || play_state == I2S_PLAYING)
	{
		play_state = I2S_PLAYING;


		res = f_read(&filePointer, &(buffer[I2S_BUFFER_SIZE/2]), I2S_BUFFER_SIZE, &out);
		/*res = f_read(&filePointer, &(in_buffer), I2S_BUFFER_SIZE/2, &out);

		for(i = 0; i < (I2S_BUFFER_SIZE/4); i ++)
		{
			buffer[(I2S_BUFFER_SIZE/2)+(i*2)] = in_buffer[i];
		}*/

		if(out != I2S_BUFFER_SIZE)
		{
			for(i = ((out/2) + (I2S_BUFFER_SIZE/2)) ; i < I2S_BUFFER_SIZE; i ++)
			{
				buffer[i] = 0;
			}

			play_state = I2S_STOP;

			eof = 1;
		}
		else
		{
			player_frame_sent_callback();
		}

	}

	else if(play_state == I2S_PAUSE || play_state == I2S_STOP)
	{
		for(i = 0 ; i < I2S_BUFFER_SIZE; i ++)
		{
			buffer[i] = 0;
		}

		if(play_state == I2S_PAUSE)
		{
			play_state = I2S_PAUSED;
		}
		else
		{
			play_state = I2S_STOPPED;

			if(eof != 0)
			{
				eof = 0;
				player_eof_callback();
			}
		}
	}

	if(dmx_record_active == 0)
	{
		dmx_start_transmit();
	}
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s){

	int i;
	if(play_state == I2S_PLAYING)
	{
		res = f_read(&filePointer, &buffer, I2S_BUFFER_SIZE, &out);

		/*res = f_read(&filePointer, &(in_buffer), I2S_BUFFER_SIZE/2, &out);

		for(i = 0; i < (I2S_BUFFER_SIZE/4); i ++)
		{
			buffer[(i*2)] = in_buffer[i];
		}*/

		if(out != I2S_BUFFER_SIZE)
		{
			for(i = (out/2) ; i < (I2S_BUFFER_SIZE/2); i ++)
			{
				buffer[i] = 0;
			}

			play_state = I2S_STOP;

			eof = 1;
		}

	}

	else if(play_state == I2S_PAUSE || play_state == I2S_STOP)
	{
		for(i = 0 ; i < I2S_BUFFER_SIZE; i ++)
		{
			buffer[i] = 0;
		}

		if(play_state == I2S_PAUSE)
		{
			play_state = I2S_PAUSED;
		}
		else
		{
			play_state = I2S_STOPPED;

			if(eof != 0)
			{
				eof = 0;
				player_eof_callback();
			}
		}
	}

}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
