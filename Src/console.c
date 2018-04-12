/*
 * console.c
 *
 *  Created on: 9 abr. 2018
 *      Author: omsulo
 */

#include <stdint.h>
#include "console.h"
#include "main.h"
#include "console_commands.h"

#define NULL_CHAR            '\0'
#define CR_CHAR              '\r'
#define LF_CHAR              '\n'
#define NOT_FOUND		-1


// Local functions
static uint32_t ConsoleCommandMatch (const char* name, const uint8_t* buffer);
static void ConsoleResetBuffer (uint8_t* receiveBuffer) ;

static eCommandResult_T ConsoleUtilHexCharToInt(char charVal, uint8_t* pInt);
static eCommandResult_T ConsoleUtilsIntToHexChar(uint8_t intVal, char* pChar);


/* Console Command Match */
/* Look to see if the data in the buffer matches the command name given that
 * the strings are different lengths and we have parameter separators */
static uint32_t ConsoleCommandMatch (const char* name, const uint8_t* buffer) {


	uint32_t i = 0;
	uint32_t result = 0;	// match
	uint32_t command_length = 0;


	/* first compute the length of the command (finde the LF_CHAR) */
	while(name[i] != LF_CHAR) {
		i++;
	}

	command_length = i;

	/* reset the counter */
	i = 0;

	/* check if the chars are equal */
	if (buffer[i] == name[i]) {
		result = 1;
		i++;
	}

	while( (result == 1) &&
			(i < CONSOLE_COMMAND_MAX_LENGTH) &&
			(buffer[i] != LF_CHAR) &&
			(buffer[i] != CR_CHAR) &&
			(buffer[i] != NULL_CHAR) ) {

		if (buffer[i] != name[i]){
			result = 0;
		}

		i++;
	}


	/* OK, the chars checked are equal, but is the command
	 * received of the same length as the one checked from the
	 * command table?? or is the command received truncated? */
	if (result == 1 ) {
		/* if the command is truncated then is not the proper command! */
		if (command_length != i) {
			result = 0;
		}
	}


	return result;
}


/* ConsoleResetBuffer */
static void ConsoleResetBuffer (uint8_t* receiveBuffer) {

	for ( uint8_t i = 0; i < (sizeof(receiveBuffer) / sizeof(receiveBuffer[0]))	; i++) {
		receiveBuffer[i] = 0;
	}
}


// ConsoleUtilHexCharToInt
// Converts a single hex character (0-9,A-F) to an integer (0-15)
static eCommandResult_T ConsoleUtilHexCharToInt(char charVal, uint8_t* pInt)
{
    eCommandResult_T result = COMMAND_SUCCESS;

    if ( ( '0' <= charVal ) && ( charVal <= '9' ) )
    {
        *pInt = charVal - '0';
    }
    else if ( ( 'A' <= charVal ) && ( charVal <= 'F' ) )
    {
        *pInt = 10u + charVal - 'A';
    }
    else if( ( 'a' <= charVal ) && ( charVal <= 'f' ) )
    {
        *pInt = 10u + charVal - 'a';
    }
	else if ( ( LF_CHAR != charVal ) || ( CR_CHAR != charVal )
			|| ( PARAMETER_SEPARATER == charVal ) )
	{
		result = COMMAND_PARAMETER_END;

	}
    else
    {
        result = COMMAND_PARAMETER_ERROR;
    }

    return result;
}



// ConsoleUtilsIntToHexChar
// Converts an integer nibble (0-15) to a hex character (0-9,A-F)
static eCommandResult_T ConsoleUtilsIntToHexChar(uint8_t intVal, char* pChar)
{
    eCommandResult_T result = COMMAND_SUCCESS;

    if ( intVal <= 9u )
    {
        *pChar = intVal + '0';
    }
    else if ( ( 10u <= intVal ) && ( intVal <= 15u ) )
    {
        *pChar = intVal - 10u + 'A';
    }
    else
    {
        result = COMMAND_PARAMETER_ERROR;
    }

    return result;
}





/* ************************************************************************** */
/* Console command API */
/* ************************************************************************** */


void ConsoleProcess (void) {

	const sConsoleCommandTable_T* commandTable;
	uint32_t cmdIndex;
	int32_t found;
	eCommandResult_T result;

	commandTable = ConsoleCommandsGetTable();
	cmdIndex = 0;
	found = NOT_FOUND;

	/* search in the table while not reach the end of it */
	while( (commandTable[cmdIndex].name != NULL) && (found == NOT_FOUND) ) {


		/* if find the command is the line cmdIndex... */
		if(ConsoleCommandMatch(commandTable[cmdIndex].name, UART6_RxBuffer)) {

			/* found! execute the function  */
			result = commandTable[cmdIndex].execute(UART6_RxBuffer);

			/* if the execution goes wrong! */
			if(result != COMMAND_SUCCESS) {
				msg_info("Error!");
				msg_info("Help:");
				msg_info(commandTable[cmdIndex].help);
				msg_info(STR_ENDLINE);
			}

			/* set the flag of command found! to break the loop */
			found = cmdIndex;

		}
		/* if not search in the following line... */
		else {
			cmdIndex++;
		}

	}

	/* if not find the command in the table */
	if( found == NOT_FOUND ) {

		msg_info("Command not found.");
		msg_info(STR_ENDLINE);
	}

	/* clean the buffer */
	ConsoleResetBuffer(UART6_RxBuffer);
}

