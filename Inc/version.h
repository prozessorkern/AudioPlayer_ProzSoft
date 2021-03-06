
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VERSION_H
#define __VERSION_H

typedef struct
{
	uint16_t software_major;
	uint16_t software_minor;
}softwareVersion_t;

typedef struct
{
	uint8_t software_day;
	uint8_t software_month;
	uint16_t software_year;
}softwareDate_t;

#define SOFTWARE_VERSION_MAJOR	1
#define SOFTWARE_VERSION_MINOR	6

#define SOFTWARE_DATE_DAY	    11
#define SOFTWARE_DATE_MONTH	  03
#define	SOFTWARE_DATE_YEAR	  2018

#endif
