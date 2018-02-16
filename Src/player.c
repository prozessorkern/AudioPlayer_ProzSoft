#include "player.h"
#include "fatfs.h"
#include "dmx.h"
#include "led.h"
#include "i2s.h"
#include "pcm1755.h"
#include "button.h"
#include "data_store_global.h"
#include "data_store_track.h"
#include "power_amp.h"
#include "wwdg.h"

FRESULT res;
FATFS fatfs_handler;

static uint8_t record_dmx = 0;

//file names:
const TCHAR * file_names[5] =
{
	"MEOWS",
	"1.WAV",
	"2.WAV",
	"3.WAV",
	"4.WAV"
};

void player_init(void)
{

	res = f_mount(&fatfs_handler, "0:", 1);

	if(res == FR_OK)
	{

		data_store_global_load();

		operating_data_init();

		led_set(LED_ON);

		dmx_init();

		//default DMX Werte laden
		data_store_global_get_dmx(dmx_data + 1);

		I2S_init_conversion();

		pcm_init();

		amp_stop();

	}

	else
	{
		NVIC_SystemReset();

	}

}

void player_load_file(uint8_t file_num)
{

	I2S_load_file(file_names[file_num]);

	data_store_track_load(file_num);

	record_dmx = 0;
}

void player_play(void)
{
	dataStorageAudio_t audio;

	data_store_track_get_audio(&audio);

	pcm_set_volume(SPEAKER_LEFT, audio.volume[0]);
	pcm_set_volume(SPEAKER_RIGHT, audio.volume[1]);

	pcm_unmute(SPEAKER_BOTH);

	if(audio.amp_active == 0)
	{
		amp_stop();
	}

	else
	{
		amp_start();
	}

	I2S_play();
}

void player_pause(void)
{
	I2S_pause();
}

void player_stop(void)
{
	I2S_stop();
	amp_stop();
	pcm_mute(SPEAKER_BOTH);

	record_dmx = 0;

	data_store_track_close();

	//default DMX Werte laden
	data_store_global_get_dmx(dmx_data + 1);
}

void player_start_dmx_record(uint8_t file_num)
{
	data_store_track_load(file_num);
	I2S_load_file(file_names[file_num]);
	data_store_clear_dmx();
	I2S_record_dmx();

	record_dmx = 1;

	player_play();
}

void player_eof_callback(void)
{
	player_stop();
}

void player_frame_sent_callback(void)
{
	if(record_dmx == 0)
	{
		data_store_track_get_dmx_frame(dmx_data + 1);
	}

	else
	{
		data_store_record_dmx_frame(dmx_data + 1);
	}
}

void player_process(void)
{
	buttonPressed_t button = {0,0};
	static uint8_t actual_playing = 0;

	static sd_fail = 0;

	if(((SD_DETECT_GPIO_Port->IDR & SD_DETECT_Pin) == 0) && sd_fail)
	{
		NVIC_SystemReset();
	}
	else if((SD_DETECT_GPIO_Port->IDR & SD_DETECT_Pin))
	{
		sd_fail = 1;
	}

	if(button_get_mode() == BUTTON_MODE_NORMAL)
	{

		button = button_get();

		if(button.pressed_time != 0 && button.button_pressed != 0)
		{
			if(button.button_pressed == actual_playing)
			{
				//Stop
				player_stop();
				actual_playing = 0;
			}

			else if(button.button_pressed != actual_playing)
			{
				player_stop();
				player_load_file(button.button_pressed);
				player_play();
				actual_playing = button.button_pressed;
			}
			button_clear();
		}

		if(I2S_get_state() == I2S_PLAY || I2S_get_state() == I2S_PLAYING)
		{
			led_set(LED_FLASH);
		}
		else
		{
			led_set(LED_ON);
			actual_playing = 0;
		}

	}

	else
	{
		led_set(LED_FLASH_FAST);

		if(actual_playing != 0)
		{
			actual_playing = 0;
			player_stop();
		}
	}

}
