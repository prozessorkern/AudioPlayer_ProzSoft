#include "remote.h"
#include "usart.h"
#include "data_store_global.h"
#include "prozStdio.h"
#include "stm32f4xx_hal_gpio.h"

static uint8_t adr;
static uint8_t remoteAdr;
static sendBuffer[REMOTE_BUFFER_SIZE];
static volatile remoteStates_t state = WAIT;
static volatile uint8_t master = 0u;
static volatile uint32_t lastActionTick;

void RemoteInit(void)
{

  adr = data_store_global_get_adress();

  if(adr != 0)
  {
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  }
}

void RemoteReceiveByte(uint8_t byte)
{
  static uint8_t buffer[REMOTE_BUFFER_SIZE];
  static idx = 0u;
  remoteHeader_t *header;

  header = &buffer;

  lastActionTick = HAL_GetTick();

  switch(state)
  {
  case WAIT:
    if(REMOTE_PREAMBLE == byte){

      buffer[0] = byte;
      idx = 1u;
      state = RECEIVE_HDR;

    }
    break;
  case RECEIVE_HDR:
    buffer[idx] = byte;
    idx ++;

    if(idx >= sizeof(remoteHeader_t))
    {
      if( (header->cmd == REMOTE_CMD_SHELL) &&
          ((header->adr == adr) || ((0 == header->adr) && (0 != master))) &&
          (header->length < (REMOTE_BUFFER_SIZE - sizeof(remoteHeader_t))))
      {
        if((header->length > 0u))
        {
          state = RECEIVE;
        }
        else
        {
          state = CALC_HDR;
        }
      }
      else
      {
        state = WAIT;
      }
    }

    break;
  case RECEIVE:
    buffer[idx] = byte;
    idx ++;

    if(0u == master)
    {
      receiveByte(&remoteRingBuffer, byte);
    }
    else
    {
      sendByte(&localShellBuffer, byte);
    }

    if(idx >= (sizeof(remoteHeader_t) + header->length))
    {
      if(0u == master)
      {
        state = CALC_HDR;
      }
      else
      {
        master = 0;
        state = WAIT;
      }

    }

    break;
  case CALC_HDR:
    __asm__("nop");
    break;
  case SEND_HDR:
    __asm__("nop");
    break;
  case SEND_BODY:
    __asm__("nop");
    break;
  default:
    state = WAIT;
    break;
  }
}

void RemoteDoProcess(void)
{
  static uint8_t length;
  static remoteHeader_t header;
  static uint8_t idx;
  uint8_t *headerPtr;

  headerPtr = &header;

  if((lastActionTick + 5000) < HAL_GetTick())
  {
    lastActionTick = UINT32_MAX - 5001;
    remoteShellBuffer.sendBuffer.busy = 0u;
    remoteShellBuffer.sendBuffer.writePtr = 0u;
    remoteShellBuffer.sendBuffer.readPtr = 0u;

    state = WAIT;
  }

  switch(state)
  {
  case WAIT:
  case RECEIVE_HDR:
  case RECEIVE:
    HAL_GPIO_WritePin(GPIOA, USART2_DE_Pin, GPIO_PIN_RESET);

    break;
  case CALC_HDR:

    if(remoteShellBuffer.sendBuffer.readPtr <= remoteShellBuffer.sendBuffer.writePtr )
    {
      length = remoteShellBuffer.sendBuffer.writePtr - remoteShellBuffer.sendBuffer.readPtr;
    }
    else
    {
      length = PRINT_BUFFER_SIZE - (remoteShellBuffer.sendBuffer.readPtr - remoteShellBuffer.sendBuffer.writePtr);
    }

    if(length > (REMOTE_BUFFER_SIZE - sizeof(remoteHeader_t)))
    {
      length = (REMOTE_BUFFER_SIZE - sizeof(remoteHeader_t));
    }

    header.preamble = REMOTE_PREAMBLE;
    header.cmd      = REMOTE_CMD_SHELL;
    header.adr      = 0u;
    header.length   = length;

    if(0 != master)
    {
      header.adr = remoteAdr;
    }
    /*! - enable transmitter  */
    HAL_GPIO_WritePin(GPIOA, USART2_DE_Pin, GPIO_PIN_SET);

    idx = 0u;
    state = SEND_HDR;

    break;
  case SEND_HDR:
    remoteShellBuffer.sendBuffer.busy = 1u;

    USART2->DR = headerPtr[idx];

    idx ++;

    if(idx >= sizeof(remoteHeader_t))
    {
      idx = 0u;
      state = SEND_BODY;
    }

    break;
  case SEND_BODY:

    if(idx >= length)
    {
      remoteShellBuffer.sendBuffer.busy = 0u;
      HAL_GPIO_WritePin(GPIOA, USART2_DE_Pin, GPIO_PIN_RESET);
      state = WAIT;
    }
    else
    {
      USART2->DR = remoteShellBuffer.sendBuffer.data[remoteShellBuffer.sendBuffer.readPtr];
      if(remoteShellBuffer.sendBuffer.readPtr < (PRINT_BUFFER_SIZE - 1))
      {
        remoteShellBuffer.sendBuffer.readPtr ++;
      }

      else
      {
        remoteShellBuffer.sendBuffer.readPtr = 0;
      }
    }

    idx ++;

    break;
  default:
    state = WAIT;
    break;
  }
}

void RemoteSendHandler(void)
{
  if(localShellBuffer.sendBuffer.readPtr != localShellBuffer.sendBuffer.writePtr)
  {
      localShellBuffer.sendBuffer.busy = 1;

    REMOTE_UART->DR = localShellBuffer.sendBuffer.data[localShellBuffer.sendBuffer.readPtr];
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

void RemoteMasterTransmit(uint8_t tempAdr)
{
  lastActionTick = HAL_GetTick();
  remoteAdr = tempAdr;
  master = 1u;
  state = CALC_HDR;
  localShellBuffer.sendBuffer.busy = 0u;
}
