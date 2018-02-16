#ifndef INC_DATA_STORE_TRACK_H_
#define INC_DATA_STORE_TRACK_H_

#include "stm32f4xx_hal.h"

#define DEFAULT_VOLUME		220
#define DEFAULT_AMP			1

#define TRACK_NAME_LENGTH			61
#define TRACK_DESCRIPTION_LENGTH	61

#define DMX_SINGLE_CHANNEL_MAX_NUM	10
#define DMX_RAW_IMAGE_SPACE			1000

typedef struct
{
	uint8_t volume[2];
	uint8_t amp_active;
} dataStorageAudio_t;

typedef struct
{
	dataStorageAudio_t	audio;
	uint8_t				name[TRACK_NAME_LENGTH];
	uint8_t				description[TRACK_DESCRIPTION_LENGTH];
} dataStorageTrack_t;

typedef enum dataStorageDmxTypes_t
{
	DMX_TYPE_RAW = 'R',
	DMX_TYPE_AREA = 'P',
	DMX_TYPE_GLOBAL = 'G',
	DMX_TYPE_SINGLE = 'S',
	DMX_TYPE_EMPTY = 'E'
}dataStorageDmxTypes_t;

typedef struct
{
	uint16_t adress;
	uint8_t value;
} dataStorageDmxChan_t;

typedef struct
{
	uint16_t start_adress;
	uint16_t channel_number;
} dataStorageDmxArea_t;

typedef struct
{
	dataStorageDmxTypes_t	type;
	uint16_t				length;
} dataStorageDmxHeader_t;

typedef struct
{
	dataStorageDmxTypes_t	type;
	uint16_t				length;
	uint16_t 				start_adress;
	uint16_t 				channel_number;
} dataStorageDmxAreaHeader_t;

typedef struct
{
	dataStorageDmxTypes_t	type;
	uint16_t				length;
	uint8_t 				value;
} dataStorageDmxGlobalHeader_t;


void data_store_track_load(uint8_t track);
void data_store_track_save(void);
void data_store_track_close(void);
void data_store_track_set_audio(dataStorageAudio_t *data);
void data_store_track_get_audio(dataStorageAudio_t *data);
void data_store_track_set_name(uint8_t *data);
void data_store_track_get_name(uint8_t *data);
void data_store_track_set_description(uint8_t *data);
void data_store_track_get_description(uint8_t *data);
void data_store_clear_dmx(void);
void data_store_track_get_dmx_frame(uint8_t *data);
void data_store_record_dmx_frame(uint8_t *data);

#endif
