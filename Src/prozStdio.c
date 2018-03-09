#include <stdio.h>
#include <stdarg.h>
#include "prozStdio.h"
#include "usart.h"

shellBuffer_t volatile localShellBuffer;
shellBuffer_t volatile remoteShellBuffer;
ringBuffer_t volatile localRingBuffer;
ringBuffer_t volatile remoteRingBuffer;

static void sendData(shellBuffer_t *buffer, uint8_t *data, uint8_t length);

int prozPrintf(shellBuffer_t *buffer, const char *template, ...)
{
  va_list args;
  int ret;
  char tempBuffer[PRINT_BUFFER_SIZE];

  va_start (args, template);

  ret = vsnprintf(tempBuffer, PRINT_BUFFER_SIZE, template, args);

  if((ret > 0) && (ret <= PRINT_BUFFER_SIZE))
    {
      sendData(buffer, tempBuffer, ret);
    }

  va_end (args);

  return ret;
}

static void sendData(shellBuffer_t *buffer, uint8_t *data, uint8_t length)
{
  uint8_t counter = 0;


  for(counter = 0; counter < length; counter ++)
  {
      sendByte(buffer, data[counter]);
  }

}

void sendByte(shellBuffer_t *buffer, uint8_t data)
{
  uint8_t writePtr = buffer->sendBuffer.writePtr;

  buffer->sendBuffer.data[writePtr] = data;

  if(writePtr < (PRINT_BUFFER_SIZE - 1))
  {
    writePtr ++;
  }
  else
  {
    writePtr = 0;
  }

  /*! - wait for space in the buffer */
  while((writePtr == buffer->sendBuffer.readPtr) &&
      (localShellBuffer.sendBuffer.readPtr != localShellBuffer.sendBuffer.writePtr))
  {
    __asm__("nop");
  }

  buffer->sendBuffer.writePtr = writePtr;

  if((buffer == &localShellBuffer) && (buffer->sendBuffer.busy == 0))
  {
    localSendHandler();
  }

}

void localSendHandler(void)
{
  if(localShellBuffer.sendBuffer.readPtr != localShellBuffer.sendBuffer.writePtr)
  {
      localShellBuffer.sendBuffer.busy = 1;

    CONFIG_PORT_UART->DR = localShellBuffer.sendBuffer.data[localShellBuffer.sendBuffer.readPtr];
    if(localShellBuffer.sendBuffer.readPtr < (PRINT_BUFFER_SIZE - 1))
    {
        localShellBuffer.sendBuffer.readPtr ++;
    }

    else
    {
        localShellBuffer.sendBuffer.readPtr = 0;
    }
  }

  else
  {
      localShellBuffer.sendBuffer.busy = 0;
  }
}

void receiveByte(ringBuffer_t *ringBuffer, uint8_t data)
{
  ringBuffer->data[ringBuffer->writePtr] = data;

  if(ringBuffer->writePtr < PRINT_BUFFER_SIZE)
  {
    ringBuffer->writePtr ++;
  }
  else
  {
    ringBuffer->writePtr = 0;
  }
}
