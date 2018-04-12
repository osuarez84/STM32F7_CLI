En este proyecto se ha tratado de hacer una consola de comandos para ser embebida en un 
microcontrolador. Para las pruebas se ha utilizado una placa STM32 NucleoF746ZG y un conector USB2COM as�
como el transceiver para adecuar los niveles de tensi�n de el MCU.

Este proyecto est� basado en el utilizado por Elecia White en su libro Making Embedded Systems y 
se ha reutilizado parte del c�digo presentado en la p�gina de github:

https://github.com/eleciawhite/reusable/

En el proyecto se ha implementado tambi�n el env�o de mensajes a trav�s de USART utilizando
la funci�n printf(). Para ello se ha a�adido al proyecto el fichero syscalls.c.

