#ifndef INC_DATA_STORE_GLOBAL_H_
#define INC_DATA_STORE_GLOBAL_H_

#include "stm32f4xx_hal.h"
#include "dmx.h"

#define DEFAULT_DMX_SIZE	256;

#define GLOBAL_NAME_LENGTH			61
#define GLOBAL_DESCRIPTION_LENGTH	61
#define GLOBAL_DATA_VERSION       1

#define GLOBAL_FILE_NAME      "global.cfg"
#define GLOBAL_TEMP_FILE_NAME "g_temp.cfg"

typedef struct
{
	uint32_t	operating_time;
	uint32_t	buttons_pressed[4];
} operationData_t;

typedef struct
{
  uint8_t           version;
  uint8_t           adress;       /*!< adress for the remote audioplayer protocol */
	operationData_t   operation_data;
	uint8_t		        name[GLOBAL_NAME_LENGTH];
	uint8_t		        description[GLOBAL_DESCRIPTION_LENGTH];
	uint16_t	        dmx_size;
	uint8_t		        dmx_data[DMX_MAX_CHANNEL];
} dataStorageGlobal_t;

extern void data_store_global_load(void);
extern void data_store_global_save(void);
extern void data_store_global_get_dmx(uint8_t *data);
extern void data_store_set_operation_data(operationData_t *operation_data);
extern void data_store_get_operation_data(operationData_t *operation_data);
extern void data_store_global_set_name(uint8_t *data);
extern void data_store_global_get_name(uint8_t *data);
extern void data_store_global_set_description(uint8_t *data);
extern void data_store_global_get_description(uint8_t *data);
extern void data_store_set_dmx_size(uint16_t data);
extern uint16_t data_store_get_dmx_size(void);
extern void data_store_record_default_dmx(void);
extern uint8_t data_store_global_get_adress(void);
extern void data_store_global_set_adress(uint8_t adress);
#endif
