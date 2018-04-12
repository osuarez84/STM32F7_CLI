/*
 * OS_uart_command.c
 *
 *  Created on: 10 abr. 2018
 *      Author: omsulo
 *
 *     #########################################################################
 *     NOTES:
 *     This USART driver can only receive/transmit using 8 bits and without
 *     parity. It has been created based on the HAL Cube to work with a
 *     command console and receive parameters from the PC until a carriage
 *     return is read ('\n').
 *     When this take place the RxCplt callback is summoned.
 *     #########################################################################
 */

#include "stm32f7xx_hal.h"
#include "OS_uart_command.h"


static void OS_UART_EndRxTransfer(UART_HandleTypeDef *huart);
static HAL_StatusTypeDef OS_UART_EndTransmit_IT(UART_HandleTypeDef *huart);
static HAL_StatusTypeDef OS_UART_Transmit_IT(UART_HandleTypeDef *huart);


/**
  * @brief Receive an amount of data in interrupt mode.
  * @param huart UART handle.
  * @param pData pointer to data buffer.
  * @param Size amount of data to be received.
  * @retval HAL status
  */
HAL_StatusTypeDef OS_HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData)
{
  /* Check that a Rx process is not already ongoing */
  if(huart->RxState == HAL_UART_STATE_READY)
  {
    if((pData == NULL ))
    {
      return HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(huart);

    huart->pRxBuffPtr = pData;
    //huart->RxXferSize = Size;
    huart->RxXferCount = 0;

    /* Computation of UART mask to apply to RDR register */
    UART_MASK_COMPUTATION(huart);

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;

    /* Process Unlocked */
    __HAL_UNLOCK(huart);

    /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    SET_BIT(huart->Instance->CR3, USART_CR3_EIE);

    /* Enable the UART Parity Error and Data Register not empty Interrupts */
    SET_BIT(huart->Instance->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}



void OS_HAL_USART_command_IRQHandler (UART_HandleTypeDef* huart) {

    uint32_t isrflags   = READ_REG(huart->Instance->ISR);
    uint32_t cr1its     = READ_REG(huart->Instance->CR1);
    uint32_t cr3its     = READ_REG(huart->Instance->CR3);
    uint32_t errorflags;

    /* If no error occurs */
    errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
    if (errorflags == RESET)
    {
        /* UART in mode Receiver ---------------------------------------------------*/
        if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
        {
            OS_UART_Receive_IT(huart);
            return;
        }
    }

    /* If some errors occur */
    if(   (errorflags != RESET)
        && (   ((cr3its & USART_CR3_EIE) != RESET)
            || ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)) )
    {

        /* UART parity error interrupt occurred -------------------------------------*/
        if(((isrflags & USART_ISR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
        {
            __HAL_UART_CLEAR_IT(huart, UART_CLEAR_PEF);

            huart->ErrorCode |= HAL_UART_ERROR_PE;
        }

        /* UART frame error interrupt occurred --------------------------------------*/
        if(((isrflags & USART_ISR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
        {
            __HAL_UART_CLEAR_IT(huart, UART_CLEAR_FEF);

            huart->ErrorCode |= HAL_UART_ERROR_FE;
        }

        /* UART noise error interrupt occurred --------------------------------------*/
        if(((isrflags & USART_ISR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
        {
            __HAL_UART_CLEAR_IT(huart, UART_CLEAR_NEF);

            huart->ErrorCode |= HAL_UART_ERROR_NE;
        }

        /* UART Over-Run interrupt occurred -----------------------------------------*/
        if(((isrflags & USART_ISR_ORE) != RESET) &&
        (((cr1its & USART_CR1_RXNEIE) != RESET) || ((cr3its & USART_CR3_EIE) != RESET)))
        {
            __HAL_UART_CLEAR_IT(huart, UART_CLEAR_OREF);

            huart->ErrorCode |= HAL_UART_ERROR_ORE;
        }

        /* Call UART Error Call back function if need be --------------------------*/
        if(huart->ErrorCode != HAL_UART_ERROR_NONE)
        {
            /* UART in mode Receiver ---------------------------------------------------*/
            if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
            {
                OS_UART_Receive_IT(huart);
            }

            /* If Overrun error occurs, or if any error occurs in DMA mode reception,
                consider error as blocking */
            if (((huart->ErrorCode & HAL_UART_ERROR_ORE) != RESET) ||
                (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR)))
            {
                /* Blocking error : transfer is aborted
                Set the UART state ready to be able to start again the process,
                Disable Rx Interrupts, and disable Rx DMA request, if ongoing */
                OS_UART_EndRxTransfer(huart);

                /* Call user error callback */
                HAL_UART_ErrorCallback(huart);

            }
            else
            {
                /* Non Blocking error : transfer could go on.
                Error is notified to user through user error callback */
                HAL_UART_ErrorCallback(huart);
                huart->ErrorCode = HAL_UART_ERROR_NONE;
            }
        }
        return;

    } /* End if some error occurs */

    /* UART in mode Transmitter ------------------------------------------------*/
    if(((isrflags & USART_ISR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
    {
        OS_UART_Transmit_IT(huart);
        return;
    }

    /* UART in mode Transmitter (transmission end) -----------------------------*/
    if(((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
    {
        OS_UART_EndTransmit_IT(huart);
        return;
    }

}

HAL_StatusTypeDef OS_UART_Receive_IT (UART_HandleTypeDef* huart) {

    uint16_t* tmp;
    uint16_t uhMask = huart->Mask;
    uint8_t ch = 0;

    /* Check that a Rx process is ongoing */
    if(huart->RxState == HAL_UART_STATE_BUSY_RX)
    {

    	/* ONLY CAN RECEIVE IN 8 BIT MODE WITHOUT PARITY */
        ch = (uint8_t)(huart->Instance->RDR & (uint8_t)uhMask);
        *huart->pRxBuffPtr++ = (uint8_t)(huart->Instance->RDR & (uint8_t)uhMask);


        if(ch == '\n')
        {
            /* Disable the UART Parity Error Interrupt and RXNE interrupt*/
            CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));

            /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
            CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

            /* Rx process is completed, restore huart->RxState to Ready */
            huart->RxState = HAL_UART_STATE_READY;

            HAL_UART_RxCpltCallback(huart);

            return HAL_OK;
        }

        return HAL_OK;
    }
    else
    {
        /* Clear RXNE interrupt flag */
        __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);

        return HAL_BUSY;
    }
}


/**
  * @brief  End ongoing Rx transfer on UART peripheral (following error detection or Reception completion).
  * @param  huart UART handle.
  * @retval None
  */
static void OS_UART_EndRxTransfer(UART_HandleTypeDef *huart)
{
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

  /* At end of Rx process, restore huart->RxState to Ready */
  huart->RxState = HAL_UART_STATE_READY;
}


/**
  * @brief  Wrap up transmission in non-blocking mode.
  * @param  huart pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
static HAL_StatusTypeDef OS_UART_EndTransmit_IT(UART_HandleTypeDef *huart)
{
  /* Disable the UART Transmit Complete Interrupt */
  CLEAR_BIT(huart->Instance->CR1, USART_CR1_TCIE);

  /* Tx process is ended, restore huart->gState to Ready */
  huart->gState = HAL_UART_STATE_READY;

  HAL_UART_TxCpltCallback(huart);

  return HAL_OK;
}


/**
  * @brief Send an amount of data in interrupt mode
  *         Function called under interruption only, once
  *         interruptions have been enabled by HAL_UART_Transmit_IT()
  * @param  huart UART handle
  * @retval HAL status
  */
static HAL_StatusTypeDef OS_UART_Transmit_IT(UART_HandleTypeDef *huart)
{
  uint16_t* tmp;

  /* Check that a Tx process is ongoing */
  if (huart->gState == HAL_UART_STATE_BUSY_TX)
  {

    if(huart->TxXferCount == 0U)
    {
    	/* Disable the UART Transmit Data Register Empty Interrupt */
		CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);

		/* Enable the UART Transmit Complete Interrupt */
		SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);

		return HAL_OK;
    }
    else
    {

    	/* CAN ONLY TRANSMIT IN 8 BIT MODE WITHOUT PARITY */
    	huart->Instance->TDR = (uint8_t)(*huart->pTxBuffPtr++ & (uint8_t)0xFFU);


    	huart->TxXferCount--;

    	return HAL_OK;
    }
  }
  else
  {
    return HAL_BUSY;
  }
}
