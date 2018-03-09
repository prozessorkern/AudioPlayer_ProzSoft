#include "data_store_global.h"
#include "data_store_track.h"
#include "fatfs.h"
#include "string.h"

dataStorageGlobal_t data_store_global;

FRESULT res;
FIL global_file_pointer;
UINT out;

void data_store_global_load(void)
{
  uint16_t counter = 0;
  do
  {
    res = f_open(&global_file_pointer, GLOBAL_FILE_NAME, FA_READ);

    counter ++;

  }while(res != FR_OK && counter < 10);

  if(res == FR_OK)
  {
  f_read(&global_file_pointer, &data_store_global, sizeof(dataStorageGlobal_t), &out);
  }

  if( (out != sizeof(dataStorageGlobal_t)) ||
      (res != FR_OK) ||
      (data_store_global.version != GLOBAL_DATA_VERSION))
    {
      data_store_global.version = GLOBAL_DATA_VERSION;
      data_store_global.adress = 0;
      data_store_global.description[0] = 0;
      data_store_global.name[0] = 0;
      data_store_global.operation_data.operating_time = 0;
      data_store_global.operation_data.buttons_pressed[0] = 0;
      data_store_global.operation_data.buttons_pressed[1] = 0;
      data_store_global.operation_data.buttons_pressed[2] = 0;
      data_store_global.operation_data.buttons_pressed[3] = 0;
      data_store_global.dmx_size = DEFAULT_DMX_SIZE;

    for(counter = 0; counter < DMX_MAX_CHANNEL; counter ++)
    {
      data_store_global.dmx_data[counter] = 0;
    }
  }
  res = f_close(&global_file_pointer);
}

void data_store_global_save(void)
{
	res = f_close(&global_file_pointer);

	res = f_unlink("temp.del");

	res = f_rename(GLOBAL_FILE_NAME, "temp.del");

	res = f_open(&global_file_pointer, GLOBAL_FILE_NAME, FA_READ | FA_WRITE | FA_CREATE_NEW);

	if(res == FR_OK)
	{
		res = f_write(&global_file_pointer, &data_store_global, sizeof(dataStorageGlobal_t), &out);
	}

	res = f_close(&global_file_pointer);

}

void data_store_global_get_dmx(uint8_t *data)
{
	uint16_t counter = 0;

	memcpy(data, data_store_global.dmx_data, DMX_MAX_CHANNEL);

}

void data_store_set_operation_data(operationData_t *operation_data)
{
	memcpy(&(data_store_global.operation_data), operation_data, sizeof(operationData_t));
}

void data_store_get_operation_data(operationData_t *operation_data)
{
	memcpy(operation_data, &(data_store_global.operation_data), sizeof(operationData_t));
}

void data_store_global_set_name(uint8_t *data)
{
	uint8_t *string_pointer;
	uint8_t counter = 0;

	string_pointer = &(data_store_global.name);

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

void data_store_global_get_name(uint8_t *data)
{
	uint8_t *string_pointer;
	uint8_t counter = 0;

	string_pointer = &(data_store_global.name);

	while(*string_pointer && counter < (GLOBAL_NAME_LENGTH - 1))
	{
		*data = *string_pointer;

		data ++;
		string_pointer ++;
		counter ++;
	}

	//terminating 0
	*data = 0;
}

void data_store_global_set_description(uint8_t *data)
{
	uint8_t *string_pointer;
	uint8_t counter = 0;

	string_pointer = (uint8_t *)(&(data_store_global.description));

	while(*data && counter < (GLOBAL_DESCRIPTION_LENGTH - 1))
	{
		*string_pointer = *data;

		data ++;
		string_pointer ++;
		counter ++;
	}

	//terminating 0
	*string_pointer = 0;
}

void data_store_global_get_description(uint8_t *data)
{
	uint8_t *string_pointer;
	uint8_t counter = 0;

	string_pointer = &(data_store_global.description);

	while(*string_pointer && counter < (GLOBAL_DESCRIPTION_LENGTH - 1))
	{
		*data = *string_pointer;

		data ++;
		string_pointer ++;
		counter ++;
	}

	//terminating 0
	*data = 0;
}

void data_store_set_dmx_size(uint16_t data)
{
	data_store_global.dmx_size = data;
}

uint16_t data_store_get_dmx_size(void)
{
	return data_store_global.dmx_size;
}

void data_store_record_default_dmx(void)
{
	memcpy(data_store_global.dmx_data, dmx_data + 1, data_store_global.dmx_size);
}

uint8_t data_store_global_get_adress(void)
{
  return data_store_global.adress;
}

void data_store_global_set_adress(uint8_t adress)
{
  data_store_global.adress = adress;
}
