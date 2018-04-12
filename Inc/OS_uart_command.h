/*
 * OS_uart_command.h
 *
 *  Created on: 10 abr. 2018
 *      Author: omsulo
 */

#ifndef OS_UART_COMMAND_H_
#define OS_UART_COMMAND_H_

HAL_StatusTypeDef OS_UART_Receive_IT (UART_HandleTypeDef* huart);
HAL_StatusTypeDef OS_HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData);
void OS_HAL_USART_command_IRQHandler (UART_HandleTypeDef* huart);

#endif /* OS_UART_COMMAND_H_ */
