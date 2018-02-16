#include "config_port.h"
#include "pcm1755.h"
#include "data_store_global.h"
#include "data_store_track.h"
#include "version.h"
#include "string.h"
#include "dmx.h"
#include "power_amp.h"

static void config_set_name( uint8_t *data, uint8_t length );
static void config_set_description( uint8_t *data, uint8_t length );
static void config_get_name( uint8_t *data, uint8_t length );
static void config_get_description( uint8_t *data, uint8_t length );
static void config_get_software_version( uint8_t *data, uint8_t length );
static void config_get_software_date( uint8_t *data, uint8_t length );
static void config_get_betriebsdaten( uint8_t *data, uint8_t length );
static void config_set_volume( uint8_t *data, uint8_t length );
static void config_get_volume( uint8_t *data, uint8_t length );
static void config_get_load_title1( uint8_t *data, uint8_t length );
static void config_get_load_title2( uint8_t *data, uint8_t length );
static void config_get_load_title3( uint8_t *data, uint8_t length );
static void config_get_load_title4( uint8_t *data, uint8_t length );
static void config_exit( uint8_t *data, uint8_t length );
static void config_play( uint8_t *data, uint8_t length );
static void config_pause( uint8_t *data, uint8_t length );
static void config_record_dmx( uint8_t *data, uint8_t length );
static void config_set_dmx_size( uint8_t *data, uint8_t length );
static void config_get_dmx_size( uint8_t *data, uint8_t length );
static void config_stop( uint8_t *data, uint8_t length );
static void config_save( uint8_t *data, uint8_t length );
static void config_set_amp_mode( uint8_t *data, uint8_t length );
static void config_get_amp_mode( uint8_t *data, uint8_t length );

static void config_port_send(uint8_t *data, uint8_t length);
static void config_port_send_byte(uint8_t data);
static void config_port_send_termination(void);

static const configPortTable_t config_port_table[CONFIG_PORT_FUNCTION_NUM] =
{
	{'N', &config_set_name},
	{'D', &config_set_description},
	{'n', &config_get_name},
	{'d', &config_get_description},
	{'V', &config_get_software_version},
	{'T', &config_get_software_date},
	{'B', &config_get_betriebsdaten},
	{'L', &config_set_volume},
	{'l', &config_get_volume},
	{'1', &config_get_load_title1},
	{'2', &config_get_load_title2},
	{'3', &config_get_load_title3},
	{'4', &config_get_load_title4},
	{'0', &config_exit},
	{'P', &config_play},
	{'p', &config_pause},
	{'r', &config_record_dmx},
	{'C', &config_set_dmx_size},
	{'c', &config_get_dmx_size},
	{'S', &config_stop},
	{'s', &config_save},
	{'A', &config_set_amp_mode},
	{'a', &config_get_amp_mode}
};

static uint8_t send_buffer[CONFIG_PORT_SEND_BUFFER_SIZE];
static uint8_t send_buffer_read_counter = 0;
static uint8_t send_buffer_write_counter = 0;
static uint8_t send_busy = 0;

uint8_t	track_num = 0;

extern UART_HandleTypeDef huart1;

static void config_set_name( uint8_t *data, uint8_t length )
{
	if(track_num == 0)
	{
		//terminating 0
		data[length] = 0;

		data_store_global_set_name(data + 1);
		config_port_send_byte(CONFIG_PORT_ACK);
	}

	else if(track_num > 0 && track_num < 5)
	{
		//terminating 0
		data[length] = 0;

		data_store_track_set_name(data + 1);
		config_port_send_byte(CONFIG_PORT_ACK);
	}

	else
	{
		config_port_send_byte(CONFIG_PORT_ERROR);
	}
}

static void config_set_description( uint8_t *data, uint8_t length )
{
	if(track_num == 0)
	{
		//terminating 0
		data[length] = 0;

		data_store_global_set_description(data + 1);
		config_port_send_byte(CONFIG_PORT_ACK);
	}

	else if(track_num > 0 && track_num < 5)
	{
		//terminating 0
		data[length] = 0;

		data_store_track_set_description(data + 1);
		config_port_send_byte(CONFIG_PORT_ACK);
	}

	else
	{
		config_port_send_byte(CONFIG_PORT_ERROR);
	}
}

static void config_get_name( uint8_t *data, uint8_t length )
{
	if(track_num == 0)
	{
		data_store_global_get_name(data + 1);
		config_port_send(data, 1 + GLOBAL_NAME_LENGTH);
	}

	else if(track_num > 0 && track_num < 5)
	{
		data_store_track_get_name(data + 1);
		config_port_send(data, 1 + TRACK_NAME_LENGTH);
	}

	else
	{
		config_port_send_byte(CONFIG_PORT_ERROR);
	}
}

static void config_get_description( uint8_t *data, uint8_t length )
{
	if(track_num == 0)
	{
		data_store_global_get_description(data + 1);
		config_port_send(data, 1 + GLOBAL_NAME_LENGTH);
	}

	else if(track_num > 0 && track_num < 5)
	{
		data_store_track_get_description(data + 1);
		config_port_send(data, 1 + TRACK_NAME_LENGTH);
	}

	else
	{
		config_port_send_byte(CONFIG_PORT_ERROR);
	}
}

static void config_get_software_version( uint8_t *data, uint8_t length )
{
	softwareVersion_t software_version;

	software_version.software_major = SOFTWARE_VERSION_MAJOR;
	software_version.software_minor = SOFTWARE_VERSION_MINOR;

	memcpy(data + 1, &software_version, sizeof(softwareVersion_t));

	config_port_send(data, 1 + sizeof(softwareVersion_t));
}

static void config_get_software_date( uint8_t *data, uint8_t length )
{
	softwareDate_t software_date;

	software_date.software_day = SOFTWARE_DATE_DAY;
	software_date.software_month = SOFTWARE_DATE_MONTH;
	software_date.software_year = SOFTWARE_DATE_YEAR;

	memcpy(data + 1, &software_date, sizeof(softwareDate_t));

	config_port_send(data, 1 + sizeof(softwareDate_t));
}

static void config_set_volume( uint8_t *data, uint8_t length )
{
	dataStorageAudio_t audio_data;

	if(length == 3)
	{
		data_store_track_get_audio(&audio_data);

		audio_data.volume[0] = data[1];
		audio_data.volume[1] = data[2];

		data_store_track_set_audio(&audio_data);

		pcm_set_volume(SPEAKER_LEFT, data[1]);
		pcm_set_volume(SPEAKER_RIGHT, data[2]);

		config_port_send_byte(CONFIG_PORT_ACK);
	}

	else
	{
		config_port_send_byte(CONFIG_PORT_ERROR);
	}
}

static void config_get_volume( uint8_t *data, uint8_t length )
{
dataStorageAudio_t audio_data;

	data_store_track_get_audio(&audio_data);

	data[1] = audio_data.volume[0];
	data[2] = audio_data.volume[1];

	config_port_send(data, 3);
}

static void config_get_betriebsdaten( uint8_t *data, uint8_t length )
{

	data_store_get_operation_data((operationData_t *)(data + 1));

	config_port_send(data, 1 + sizeof(operationData_t));

}

static void config_get_load_title1( uint8_t *data, uint8_t length )
{
	track_num = 1;

	player_load_file(track_num);

	config_port_send_byte(CONFIG_PORT_ACK);
}

static void config_get_load_title2( uint8_t *data, uint8_t length )
{
	track_num = 2;

	player_load_file(track_num);

	config_port_send_byte(CONFIG_PORT_ACK);
}

static void config_get_load_title3( uint8_t *data, uint8_t length )
{
	track_num = 3;

	player_load_file(track_num);

	config_port_send_byte(CONFIG_PORT_ACK);
}

static void config_get_load_title4( uint8_t *data, uint8_t length )
{
	track_num = 4;

	player_load_file(track_num);

	config_port_send_byte(CONFIG_PORT_ACK);
}

static void config_exit( uint8_t *data, uint8_t length )
{
	track_num = 0;

	player_stop();

	config_port_send_byte(CONFIG_PORT_ACK);
}

static void config_play( uint8_t *data, uint8_t length )
{
	player_play();

	config_port_send_byte(CONFIG_PORT_ACK);
}

static void config_pause( uint8_t *data, uint8_t length )
{
	player_pause();

	config_port_send_byte(CONFIG_PORT_ACK);
}

static void config_record_dmx( uint8_t *data, uint8_t length )
{
	volatile uint32_t dmx_counter = 0;

	dmx_received_sets = 0;


	if(track_num == 0)
	{
		dmx_start_receive();

		while(dmx_received_sets < 3 && (dmx_counter < 10000000))
		{
			dmx_counter ++;
		}

		if(dmx_received_sets >= 3)
		{
			data_store_record_default_dmx();

			config_port_send_byte(CONFIG_PORT_ACK);
		}

		else
		{
			config_port_send_byte(CONFIG_PORT_ERROR);
		}
	}

	else
	{
		dmx_start_receive();
		player_start_dmx_record(track_num);
	}
}

static void config_set_dmx_size( uint8_t *data, uint8_t length )
{
	uint16_t size;

	if(length == 3)
	{

		memcpy(&size, data + 1, sizeof(uint16_t));

		if((size <= DMX_MAX_CHANNEL))
		{
			config_port_send_byte(CONFIG_PORT_ACK);

			data_store_set_dmx_size(size);
		}

		else
		{
			config_port_send_byte(CONFIG_PORT_ERROR);
		}


	}

	else
	{
		config_port_send_byte(CONFIG_PORT_ERROR);
	}
}

static void config_get_dmx_size( uint8_t *data, uint8_t length )
{
	uint16_t size;

	size = data_store_get_dmx_size();

	memcpy(data + 1, &size, sizeof(uint16_t));

	config_port_send(data, 1 + sizeof(uint16_t));

}

static void config_stop( uint8_t *data, uint8_t length )
{
	player_stop();

	player_load_file(track_num);

	config_port_send_byte(CONFIG_PORT_ACK);
}

static void config_save( uint8_t *data, uint8_t length )
{
	if(track_num == 0)
	{
		data_store_global_save();
	}
	else if(track_num > 0 && track_num < 5)
	{
		data_store_track_save();
	}

	config_port_send_byte(CONFIG_PORT_ACK);
}

static void config_set_amp_mode( uint8_t *data, uint8_t length )
{
	dataStorageAudio_t audio_data;

	if(track_num > 0 && track_num < 5)
	{
		if(length == 2)
		{
			data_store_track_get_audio(&audio_data);

			audio_data.amp_active = data[1];

			data_store_track_set_audio(&audio_data);

			if(data[1] == 0)
			{
				amp_stop();
			}
			else
			{
				amp_start();
			}

			config_port_send_byte(CONFIG_PORT_ACK);
		}
		else
		{
			config_port_send_byte(CONFIG_PORT_ERROR);
		}
	}
	else
	{
		config_port_send_byte(CONFIG_PORT_ERROR);
	}

}

static void config_get_amp_mode( uint8_t *data, uint8_t length )
{
	dataStorageAudio_t audio_data;

	if(track_num > 0 && track_num < 5)
	{
		data_store_track_get_audio(&audio_data);

		data[1] = audio_data.amp_active;

		config_port_send(data, 2);
	}
	else
	{
		config_port_send_byte(CONFIG_PORT_ERROR);
	}

;
}

static void config_port_send(uint8_t *data, uint8_t length)
{
	uint8_t counter = 0;


	for(counter = 0; counter < length; counter ++)
	{
		config_port_send_byte(data[counter]);

	}

}

static void config_port_send_byte(uint8_t data)
{
	send_buffer[send_buffer_write_counter] = data;

	if(send_buffer_write_counter < (CONFIG_PORT_SEND_BUFFER_SIZE - 1))
	{
		send_buffer_write_counter ++;
	}

	else
	{
		send_buffer_write_counter = 0;
	}

	if(send_busy == 0)
	{
		CONFIG_PORT_UART->DR = send_buffer[send_buffer_read_counter];
		if(send_buffer_read_counter < (CONFIG_PORT_SEND_BUFFER_SIZE - 1))
		{
			send_buffer_read_counter ++;
		}

		else
		{
			send_buffer_read_counter = 0;
		}

		send_busy = 1;

	}
}

static void config_port_send_termination(void)
{
	uint8_t cr_lf[2] = {CR, LF};

	config_port_send(cr_lf, 2);
}

void config_port_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = BUTTON_1_Pin|BUTTON_2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_TC);

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

}

void config_port_terminate(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	HAL_NVIC_DisableIRQ(USART1_IRQn);

	GPIO_InitStruct.Pin = BUTTON_1_Pin|BUTTON_2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

void config_port_receive_handler(uint8_t data)
{
	static volatile uint8_t receive_buffer[CONFIG_PORT_RECEIVE_BUFFER_SIZE];
	static uint8_t receive_buffer_counter = 0;
	static uint8_t counter = 0;

	receive_buffer[receive_buffer_counter] = data;

	if(receive_buffer_counter < CONFIG_PORT_RECEIVE_BUFFER_SIZE )
	{
		receive_buffer_counter ++;
	}

	else
	{
		//overflow
		receive_buffer_counter = 0;
	}


	if(receive_buffer_counter >= 3)
	{
		//Ende Erkennen
		if(receive_buffer[receive_buffer_counter - 1] == LF && receive_buffer[receive_buffer_counter - 2] == CR)
		{
			//Tabelleneintrag suchen
			for(counter = 0; counter < CONFIG_PORT_FUNCTION_NUM; counter ++)
			{
				//Tabelleneintrag gefunden
				if(config_port_table[counter].config_port_idx == receive_buffer[0])
				{
					//Funktion aufrufen
					(*config_port_table[counter].config_port_function)(receive_buffer, receive_buffer_counter - 2);

					//Termination senden
					config_port_send_termination();
				}
			}

			//neu starten
			receive_buffer_counter = 0;
		}
	}
}

void config_port_send_handler(void)
{
	if(send_buffer_read_counter != send_buffer_write_counter)
	{
		send_busy = 1;

		CONFIG_PORT_UART->DR = send_buffer[send_buffer_read_counter];
		if(send_buffer_read_counter < (CONFIG_PORT_SEND_BUFFER_SIZE - 1))
		{
			send_buffer_read_counter ++;
		}

		else
		{
			send_buffer_read_counter = 0;
		}
	}

	else
	{
		send_busy = 0;
	}
}
