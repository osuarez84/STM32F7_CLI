En este proyecto se ha tratado de hacer una consola de comandos para ser embebida en un 
microcontrolador. Para las pruebas se ha utilizado una placa STM32 NucleoF746ZG y un conector USB2COM así
como el transceiver para adecuar los niveles de tensión de el MCU.

Este proyecto está basado en el utilizado por Elecia White en su libro Making Embedded Systems y 
se ha reutilizado parte del código presentado en la página de github:

https://github.com/eleciawhite/reusable/

En el proyecto se ha implementado también el envío de mensajes a través de USART utilizando
la función printf(). Para ello se ha añadido al proyecto el fichero syscalls.c.

