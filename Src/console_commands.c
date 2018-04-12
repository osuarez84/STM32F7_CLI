/*
 * console_commands.c
 *
 *  Created on: 9 abr. 2018
 *      Author: omsulo
 */
#include "console_commands.h"
#include "console.h"
#include "main.h"

static eCommandResult_T ConsoleCommandHelp (const uint8_t* buffer);
static eCommandResult_T ConsoleCommandLED (const uint8_t* buffer);
static eCommandResult_T ConsoleCommandVer (const uint8_t* buffer);


/* the final LF_CHAR is used to compute the length of the command */
static const sConsoleCommandTable_T mConsoleCommandTable[] = {

		{"help\n", &ConsoleCommandHelp, "Lists the commands available"},
		{"led\n", &ConsoleCommandLED, "Start to blink the green led"},
		{"ver\n", &ConsoleCommandVer, "Get the version string"},

		CONSOLE_COMMAND_TABLE_END // Must be the last


};


static eCommandResult_T ConsoleCommandHelp (const uint8_t* buffer) {

	uint32_t i;
	uint32_t tableLenght;
	eCommandResult_T result = COMMAND_SUCCESS;

	tableLenght = sizeof(mConsoleCommandTable) / sizeof(mConsoleCommandTable[0]);

	for (i = 0; i < tableLenght - 1; i++) {
		msg_info(mConsoleCommandTable[i].name);
		msg_info(" : ");
		msg_info(mConsoleCommandTable[i].help);
		msg_info(STR_ENDLINE);
	}

	return result;

}



static eCommandResult_T ConsoleCommandLED (const uint8_t* buffer) {
	eCommandResult_T result = COMMAND_SUCCESS;

	msg_info("The LED has been turned on!");
	msg_info(STR_ENDLINE);

	return result;
}


static eCommandResult_T ConsoleCommandVer (const uint8_t* buffer) {
	eCommandResult_T result = COMMAND_SUCCESS;


	msg_info("ver 1.0.0 test CEMITEC");
	msg_info(STR_ENDLINE);
	return result;
}



const sConsoleCommandTable_T* ConsoleCommandsGetTable(void)
{
	return (mConsoleCommandTable);
}


