/*
 * dmx.c
 *
 *  Created on: 23.02.2016
 *      Author: Stefan
 */
#include "dmx.h"
#include "gpio.h"
#include "usart.h"

volatile uint16_t dmx_received_sets = 0;
volatile uint8_t *dmx_data;
static uint8_t dmx_buffer[2][DMX_MAX_CHANNEL + 1];
static int16_t dmx_counter;
static int16_t dmx_check_counter;
static int16_t dmx_channel_num;
static uint8_t dmx_buffer_num;

uint16_t dmx_size = DMX_MAX_CHANNEL;

dmxMode_t dmx_mode;
uint8_t mark = 0;
extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;

void dmx_init(void)
{
	dmx_data = &(dmx_buffer[0][0]);
	MX_USART6_UART_Init();
	dmx_mode = DMX_MODE_TRANSMIT;
	dmx_data[0] = DMX_BYTE0;
}

void dmx_start_transmit(void)
{
	//Driver enable
	HAL_GPIO_WritePin(DMX_DE_GPIO_Port, DMX_DE_Pin, GPIO_PIN_SET);

	__HAL_UART_DISABLE_IT(&huart6, UART_IT_ERR);

	dmx_mode = DMX_MODE_TRANSMIT;

	if(mark == 0)
	{
		HAL_GPIO_WritePin(DMX_DE_GPIO_Port, DMX_TX_Pin, GPIO_PIN_SET);
		HAL_UART_Transmit_DMA(&huart6, dmx_data, 2);
		mark = 1;
	}
	else
	{
		mark = 0;

		GPIO_InitTypeDef GPIO_InitStruct;

		GPIO_InitStruct.Pin = DMX_TX_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
		HAL_GPIO_Init(DMX_TX_GPIO_Port, &GPIO_InitStruct);

		__HAL_UART_DISABLE_IT(&huart6, UART_IT_ERR);


		dmx_data[0] = DMX_BYTE0;
		HAL_UART_Transmit_DMA(&huart6, dmx_data, dmx_size + 1);

	}
}

void dmx_transmit_finish_handler(void)
{

	if(dmx_mode == DMX_MODE_TRANSMIT)
	{
		GPIO_InitTypeDef GPIO_InitStruct;

		GPIO_InitStruct.Pin = DMX_TX_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
		HAL_GPIO_Init(DMX_TX_GPIO_Port, &GPIO_InitStruct);

		if(mark == 1)
		{
			dmx_start_transmit();
		}

		else
		{
			//Pause Signal
			HAL_GPIO_WritePin(DMX_DE_GPIO_Port, DMX_TX_Pin, GPIO_PIN_RESET);
		}
	}
}

void dmx_start_receive(void)
{
	uint16_t counter1, counter2;
	dmx_mode = DMX_MODE_RECEIVE;
	dmx_buffer_num = 0;

	I2S_record_dmx();

	for(counter1 = 0; counter1 < 2; counter1 ++)
	{
		for(counter2 = 0; counter2 < (DMX_MAX_CHANNEL+1); counter2 ++)
		{
			dmx_buffer[counter1][counter2] = 0;
		}
	}

	HAL_GPIO_WritePin(DMX_DE_GPIO_Port, DMX_DE_Pin, GPIO_PIN_RESET);

	//HAL_UART_DMAStop(&huart6);
	HAL_DMA_Start(&hdma_usart6_rx, (uint32_t)(&(USART6->DR)), (uint32_t)(&(dmx_buffer[1][0])), (DMX_MAX_CHANNEL + 1));
	__HAL_UART_ENABLE_IT(&huart6, UART_IT_ERR);
	huart6.Instance->CR3 |= USART_CR3_DMAR;
}

void dmx_receive_reset_handler(void)
{
	hdma_usart6_rx.Instance->CR &= ~(1);
	huart6.Instance->CR3 &= ~(USART_CR3_DMAR);

	__HAL_UNLOCK(&hdma_usart6_rx);


	if(dmx_mode == DMX_MODE_RECEIVE)
	{
		dmx_counter = (DMX_MAX_CHANNEL + 1) - hdma_usart6_rx.Instance->NDTR;

		//Plausibilitätsprüfung (stimmt die Anzahl der Kanäle)
		if(dmx_counter == dmx_channel_num)
		{
			dmx_check_counter = dmx_counter;

			if(dmx_buffer[dmx_buffer_num][0] == DMX_BYTE0)
			{
				if(dmx_received_sets < 10000)
				{
					dmx_received_sets ++;
				}

				//DMX Buffer übernehmen
				dmx_data = &(dmx_buffer[dmx_buffer_num][0]);

				dmx_buffer_num ^= 1;

			}
		}

		//für nächsten Empfang Kanal Anzahl speichern (Plausibilitätsprüfung)
		else if(dmx_check_counter != dmx_counter)
		{
			dmx_check_counter = dmx_counter;
		}

		//nächster Empfang mit neuer Kanalanzahl wird erkannt
		else
		{
			dmx_channel_num = dmx_counter;
		}

		__HAL_DMA_CLEAR_FLAG(&hdma_usart6_rx, DMA_FLAG_TCIF2_6 | DMA_FLAG_HTIF2_6 | DMA_FLAG_TEIF2_6 | DMA_FLAG_DMEIF2_6 | DMA_FLAG_FEIF2_6 );

		HAL_DMA_Start(&hdma_usart6_rx, (uint32_t)(&(USART6->DR)), (uint32_t)(&(dmx_buffer[dmx_buffer_num][0])), (DMX_MAX_CHANNEL + 1));
		huart6.Instance->CR3 |= USART_CR3_DMAR;

	}
}
