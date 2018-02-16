#include "data_store_track.h"
#include "data_store_global.h"
#include "fatfs.h"

//file names:
const TCHAR * data_storage_file_names[5] = {
	"MEOWS",
	"1.cfg",
	"2.cfg",
	"3.cfg",
	"4.cfg"
};

static void data_store_parse_dmx(dataStorageDmxHeader_t dmx_header, uint8_t *data);


dataStorageTrack_t data_storage_track;

FRESULT res;
FIL track_file_pointer;
UINT out;

uint8_t file_num = 0;

uint32_t dmx_frame_num = 0;
uint16_t data_store_dmx_size = 512;
uint8_t dmx_frame_buffer[DMX_MAX_CHANNEL];
dataStorageDmxChan_t dmx_single_channels[DMX_SINGLE_CHANNEL_MAX_NUM];
volatile uint32_t dmx_frame_counter = 0;

uint8_t dmx_fail = 0;

void data_store_track_load(uint8_t track)
{
	file_num = track;
	uint8_t counter = 0;

	do
	{
		res = f_close(&track_file_pointer);

		counter ++;

	}while(res != FR_OK && counter < 5);

	counter = 0;

	do
	{
		res = f_open(&track_file_pointer, data_storage_file_names[file_num], FA_READ | FA_WRITE);

		counter ++;

	}while(res != FR_OK && counter < 5);

	if(res == FR_OK)
	{
		f_read(&track_file_pointer, &data_storage_track, sizeof(dataStorageTrack_t), &out);
	}

	if(out != sizeof(dataStorageTrack_t) || res != FR_OK)
	{
		data_storage_track.audio.amp_active = DEFAULT_AMP;
		data_storage_track.description[0] = 0;
		data_storage_track.name[0] = 0;
		data_storage_track.audio.volume[0] = DEFAULT_VOLUME;
		data_storage_track.audio.volume[1] = DEFAULT_VOLUME;
	}

	data_store_dmx_size = data_store_get_dmx_size();
	dmx_frame_num = 0;
	dmx_frame_counter = 0;
	dmx_fail = 0;

}

void data_store_track_save(void)
{
	uint8_t counter = 0;

	do
	{
		res = f_close(&track_file_pointer);

		res = f_open(&track_file_pointer, data_storage_file_names[file_num], FA_READ | FA_WRITE);

		if(res != FR_OK)
		{
			res = f_open(&track_file_pointer, data_storage_file_names[file_num], FA_READ | FA_WRITE | FA_CREATE_NEW);
		}

		counter ++;

	}while(res != FR_OK && counter < 5);

	if(res != FR_OK)
	{
		res = f_open(&track_file_pointer, data_storage_file_names[file_num], FA_READ | FA_WRITE | FA_CREATE_NEW);
	}

	if(res != FR_OK)
	{
		return;
	}

	else
	{
		res = f_write(&track_file_pointer, &data_storage_track, sizeof(dataStorageTrack_t), &out);
	}

	f_sync(&track_file_pointer);
}

void data_store_track_close(void)
{
	res = f_close(&track_file_pointer);
}

void data_store_track_set_audio(dataStorageAudio_t *data)
{
	memcpy(&(data_storage_track.audio), data, sizeof(dataStorageAudio_t));
}

void data_store_track_get_audio(dataStorageAudio_t *data)
{
	memcpy(data, &(data_storage_track.audio), sizeof(dataStorageAudio_t));
}

void data_store_track_set_name(uint8_t *data)
{
	uint8_t *string_pointer;
	uint8_t counter = 0;

	string_pointer = &(data_storage_track.name);

	while(*data && counter < (GLOBAL_NAME_LENGTH - 1))
	{
		*string_pointer = *data;

		data ++;
		string_pointer ++;
		counter ++;
	}

	//terminating 0
	*string_pointer = 0;
}

void data_store_track_get_name(uint8_t *data)
{
	uint8_t *string_pointer;
	uint8_t counter = 0;

	string_pointer = &(data_storage_track.name);

	while(*string_pointer && (counter < (GLOBAL_NAME_LENGTH - 1)))
	{
		*data = *string_pointer;

		data ++;
		string_pointer ++;
		counter ++;
	}

	//terminating 0
	*data = 0;
}

void data_store_track_set_description(uint8_t *data)
{
	uint8_t *string_pointer;
	uint8_t counter = 0;

	string_pointer = (uint8_t *)(&(data_storage_track.description));

	while(*data && (counter < (GLOBAL_DESCRIPTION_LENGTH - 1)))
	{
		*string_pointer = *data;

		data ++;
		string_pointer ++;
		counter ++;
	}

	//terminating 0
	*string_pointer = 0;
}

void data_store_track_get_description(uint8_t *data)
{
	uint8_t *string_pointer;
	uint8_t counter = 0;

	string_pointer = &(data_storage_track.description);

	while(*string_pointer && (counter < (GLOBAL_DESCRIPTION_LENGTH - 1)))
	{
		*data = *string_pointer;

		data ++;
		string_pointer ++;
		counter ++;
	}

	//terminating 0
	*data = 0;
}

void data_store_clear_dmx(void)
{
	if(file_num > 0 && file_num < 5)
	{
		res = f_close(&track_file_pointer);
		res = f_unlink(data_storage_file_names[file_num]);

		data_store_track_save();

		dmx_frame_num = 0;

		dmx_frame_counter = 0;

		data_store_dmx_size = data_store_get_dmx_size();
	}
}

void data_store_track_get_dmx_frame(uint8_t *data)
{
	static dataStorageDmxHeader_t dmx_get_header;

	if(dmx_frame_counter > 0)
	{
		dmx_frame_counter --;
	}
	else
	{
		if(dmx_frame_num == 0)
		{
			res = f_read(&track_file_pointer, &dmx_get_header, sizeof(dataStorageDmxHeader_t), &out);

			if(out != sizeof(dataStorageDmxHeader_t))
			{
				return;
			}

			data_store_parse_dmx(dmx_get_header, data);

			res = f_read(&track_file_pointer, &dmx_get_header, sizeof(dataStorageDmxHeader_t), &out);

		}

		else
		{
			data_store_parse_dmx(dmx_get_header, data);

			res = f_read(&track_file_pointer, &dmx_get_header, sizeof(dataStorageDmxHeader_t), &out);
		}

	}

	dmx_frame_num ++;
}

void data_store_record_dmx_frame(uint8_t *data)
{
	static volatile uint32_t dmx_old_frame_counter = 0;
	static uint32_t dmx_full_Image_counter = 0;

	uint16_t counter = 0;
	uint16_t different_bytes_counter = 0;
	dataStorageDmxHeader_t dmx_header;
	dataStorageDmxAreaHeader_t dmx_area_header;
	dataStorageDmxGlobalHeader_t dmx_global_header;

	uint8_t different_values = 0;
	uint8_t	different_values_buffer = 0;

	dmx_area_header.start_adress = 0;
	different_values_buffer = data[0];

	for(counter = 0; counter < data_store_dmx_size; counter ++)
	{
		if(dmx_frame_buffer[counter] != data[counter])
		{
			if(different_bytes_counter < DMX_SINGLE_CHANNEL_MAX_NUM)
			{
				dmx_single_channels[different_bytes_counter].adress = counter;
				dmx_single_channels[different_bytes_counter].value = data[counter];
			}

			if(dmx_area_header.start_adress == 0)
			{
				dmx_area_header.start_adress = counter + 1;
			}

			else
			{
				dmx_area_header.channel_number = (counter + 2) - dmx_area_header.start_adress;
			}

			dmx_frame_buffer[counter] = data[counter];

			different_bytes_counter ++;
		}

		if(data[counter] != different_values_buffer)
		{
			different_values = 1;
		}
	}

	//Beim Start und regelmäßig ein Raw Image speichern
	if(dmx_frame_num == 0 || dmx_full_Image_counter == 0)
	{
		if(dmx_frame_num == 0)
		{
			dmx_old_frame_counter = 0;
		}

		if(dmx_old_frame_counter != 0)
		{
			dmx_header.type = DMX_TYPE_EMPTY;
			dmx_header.length = dmx_old_frame_counter;

			res = f_write(&track_file_pointer, &dmx_header, sizeof(dataStorageDmxHeader_t), &out);

		}
/*
		if(different_values == 0)
		{
			dmx_global_header.type = DMX_TYPE_GLOBAL;
			dmx_global_header.length = 1;
			dmx_global_header.value = data[0];
			res = f_write(&track_file_pointer, &dmx_global_header, sizeof(dataStorageDmxGlobalHeader_t), &out);

		}

		else
		{*/
			dmx_header.type = DMX_TYPE_RAW;
			dmx_header.length = data_store_dmx_size;

			res = f_write(&track_file_pointer, &dmx_header, sizeof(dataStorageDmxHeader_t), &out);
			res = f_write(&track_file_pointer, dmx_frame_buffer, dmx_header.length, &out);
//		}

		dmx_full_Image_counter = DMX_RAW_IMAGE_SPACE;
		dmx_old_frame_counter = 0;
	}

	else
	{
		if(different_bytes_counter == 0)
		{
			dmx_old_frame_counter ++;
		}

		else
		{
			if(dmx_old_frame_counter != 0)
			{
				dmx_header.type = DMX_TYPE_EMPTY;
				dmx_header.length = dmx_old_frame_counter;

				res = f_write(&track_file_pointer, &dmx_header, sizeof(dataStorageDmxHeader_t), &out);
				dmx_old_frame_counter = 0;
			}
/*
			if(different_values == 0)
			{
				dmx_global_header.type = DMX_TYPE_GLOBAL;
				dmx_global_header.length = sizeof(dataStorageDmxGlobalHeader_t) - sizeof(dataStorageDmxHeader_t);
				dmx_global_header.value = data[0];
				res = f_write(&track_file_pointer, &dmx_global_header, sizeof(dataStorageDmxGlobalHeader_t), &out);
			}

			else */if((dmx_area_header.channel_number + 4) < (different_bytes_counter * 3))
			{
				dmx_area_header.type = DMX_TYPE_AREA;
				dmx_area_header.length = dmx_area_header.channel_number + sizeof(dataStorageDmxAreaHeader_t) - sizeof(dataStorageDmxHeader_t);

				res = f_write(&track_file_pointer, &dmx_area_header, sizeof(dataStorageDmxAreaHeader_t), &out);
				res = f_write(&track_file_pointer, &(data[dmx_area_header.start_adress]) - 1, dmx_area_header.channel_number, &out);
			}

			else if(different_bytes_counter <= DMX_SINGLE_CHANNEL_MAX_NUM)
			{
				dmx_header.type = DMX_TYPE_SINGLE;
				dmx_header.length = different_bytes_counter * sizeof(dataStorageDmxChan_t);

				res = f_write(&track_file_pointer, &dmx_header, sizeof(dataStorageDmxHeader_t), &out);
				res = f_write(&track_file_pointer, &dmx_single_channels, dmx_header.length, &out);
			}

			else
			{
				dmx_area_header.type = DMX_TYPE_AREA;
				dmx_area_header.length = sizeof(dataStorageDmxAreaHeader_t) - sizeof(dataStorageDmxHeader_t) + dmx_area_header.channel_number;

				res = f_write(&track_file_pointer, &dmx_area_header, sizeof(dataStorageDmxAreaHeader_t), &out);
				res = f_write(&track_file_pointer, &(data[dmx_area_header.start_adress]), dmx_area_header.channel_number, &out);
			}


		}
	}

	if(dmx_full_Image_counter > 0)
	{
		dmx_full_Image_counter --;
	}
	dmx_frame_num ++;
}

static void data_store_parse_dmx(dataStorageDmxHeader_t dmx_header, uint8_t *data)
{
	dataStorageDmxAreaHeader_t dmx_area_header;
	dataStorageDmxGlobalHeader_t dmx_global_header;

	uint16_t counter = 0;

	if(dmx_header.type == DMX_TYPE_GLOBAL)
	{

		res = f_read(&track_file_pointer, &dmx_global_header + sizeof(dataStorageDmxHeader_t), sizeof(dataStorageDmxGlobalHeader_t) - sizeof(dataStorageDmxHeader_t), &out);

		for(counter = 0; counter < data_store_dmx_size; counter ++)
		{
			data[counter] = dmx_global_header.value;
		}

	}

	else if(dmx_header.type == DMX_TYPE_RAW)
	{
		res = f_read(&track_file_pointer, data, dmx_header.length, &out);
	}

	else if(dmx_header.type == DMX_TYPE_SINGLE)
	{
		res = f_read(&track_file_pointer, &dmx_single_channels, dmx_header.length, &out);
		for(counter = 0; counter < (dmx_header.length / sizeof(dataStorageDmxChan_t)); counter ++)
		{
			data[dmx_single_channels[counter].adress] = dmx_single_channels[counter].value;
		}
	}

	else if(dmx_header.type == DMX_TYPE_AREA)
	{
		res = f_read(&track_file_pointer, &(dmx_area_header.start_adress), sizeof(dataStorageDmxAreaHeader_t) - sizeof(dataStorageDmxHeader_t), &out);

		res = f_read(&track_file_pointer, data + dmx_area_header.start_adress , dmx_area_header.channel_number, &out);
	}

	else if(dmx_header.type == DMX_TYPE_EMPTY)
	{
		dmx_frame_counter = dmx_header.length - 1;
	}

	else
	{
		return;
	}
}
