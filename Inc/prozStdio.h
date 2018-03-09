#ifndef PROZ_STDIO_H
#define PROZ_STDIO_H

#include <stdint.h>

#define PRINT_BUFFER_SIZE       128
#define CONFIG_PORT_UART        USART1
#define REMOTE_UART             USART2

typedef struct
{
  char data[PRINT_BUFFER_SIZE];
  uint8_t readPtr;
  uint8_t writePtr;
  uint8_t busy;
}ringBuffer_t;

typedef struct
{
  ringBuffer_t receiveBuffer;
  ringBuffer_t sendBuffer;
}shellBuffer_t;

extern shellBuffer_t volatile localShellBuffer;
extern shellBuffer_t volatile remoteShellBuffer;
extern ringBuffer_t volatile localRingBuffer;
extern ringBuffer_t volatile remoteRingBuffer;

extern int prozPrintf(shellBuffer_t *buffer, const char *template, ...);
extern void localSendHandler(void);
extern void sendByte(shellBuffer_t *buffer, uint8_t data);
extern void receiveByte(ringBuffer_t *ringBuffer, uint8_t data);

#endif
