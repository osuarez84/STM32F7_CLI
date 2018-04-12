/*
 * console.h
 *
 *  Created on: 9 abr. 2018
 *      Author: omsulo
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_


#define PARAMETER_SEPARATER		(' ')
#define CONSOLE_COMMAND_MAX_LENGTH 10
#define STR_ENDLINE "\r\n"

// called from lower down areas of the code (consoleCommands)
typedef enum {
	COMMAND_SUCCESS = 0u,
	COMMAND_PARAMETER_ERROR = 0x10u,
	COMMAND_PARAMETER_END   = 0x11u,
	COMMAND_ERROR =0xFFu
} eCommandResult_T;


void ConsoleProcess(void); // call this in a loop


#endif /* CONSOLE_H_ */
