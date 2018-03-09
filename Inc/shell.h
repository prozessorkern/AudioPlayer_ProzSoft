#ifndef SHELL_H
#define SHELL_H

#include "prozStdio.h"

// Funktionspointer für command port Funktionen
typedef void ( *cmdFunction_t )(shellBuffer_t *buffer, uint8_t data);

typedef struct {
  const uint8_t       *command;     /*!< command string */
  const cmdFunction_t cmdFunction;  /*!< function pointer */
  const uint8_t       *help;        /*!< help string */
} shellCommand_t;

extern void shellDoProcess();
extern void shellByteReveiced(shellBuffer_t *buffer, uint8_t data);

#endif
