#ifndef REMOTE_H
#define REMOTE_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define REMOTE_PREAMBLE     0xAAu
#define REMOTE_BUFFER_SIZE  255u

#define REMOTE_CMD_SHELL    0x55u

typedef enum
{
  WAIT,         /*!< wait for the next preamble */
  RECEIVE_HDR,  /*!< receive the telegram */
  RECEIVE,      /*!< receive the telegram */
  CALC_HDR,     /*!< send the answer */
  SEND_HDR,     /*!< send the answer */
  SEND_BODY,     /*!< send the answer */
}remoteStates_t;

typedef struct
{
  uint8_t preamble;
  uint8_t adr;
  uint8_t cmd;
  uint8_t length;
}remoteHeader_t;

extern void RemoteInit(void);
extern void RemoteReceiveByte(uint8_t byte);
extern void RemoteDoProcess(void);
extern void RemoteSendHandler(void);
extern void RemoteMasterTransmit(uint8_t tempAdr);

#endif