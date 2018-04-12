/*
 * console_commands.h
 *
 *  Created on: 9 abr. 2018
 *      Author: omsulo
 */

#ifndef CONSOLE_COMMANDS_H_
#define CONSOLE_COMMANDS_H_

#include <stdint.h>
#include <string.h>
#include "console.h"


#define CONSOLE_COMMAND_TABLE_END {NULL, NULL, ""}



typedef eCommandResult_T(*ConsoleCommand_T)(const uint8_t* buffer);

typedef struct sConsoleCommandStruct{
	const char* name;
	ConsoleCommand_T execute;
	const char* help;
}sConsoleCommandTable_T;

const sConsoleCommandTable_T* ConsoleCommandsGetTable(void);


#endif /* CONSOLE_COMMANDS_H_ */
