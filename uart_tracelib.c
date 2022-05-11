/* Copyright (c) 2021 ALIF SEMICONDUCTOR

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ALIF SEMICONDUCTOR nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.
 *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Driver_USART.h"
#include "Driver_PINMUX_AND_PINPAD.h"

/* Select used UART.
 *  Supported UARTs:
 *      UART2 (Pin P3_17)
 *      UART4 (Pin P3_2)
 */
#define UART      4

/* For Release build disable printf and semihosting */
//#define DISABLE_PRINTF

#ifdef DISABLE_PRINTF
#define printf(fmt, ...) (0)
/* Also Disable Semihosting */
#if __ARMCC_VERSION >= 6000000
__asm(".global __use_no_semihosting");
#elif __ARMCC_VERSION >= 5000000
#pragma import(__use_no_semihosting)
#else
#error Unsupported compiler
#endif

void _sys_exit(int return_code) {
    while (1);
}
#endif

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(UART);

/* UART Driver instance */
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(UART);

volatile uint32_t uart_event;

static int hardware_init(void)
{
    int32_t ret;

#if UART == 2
    /* PINMUX UART2_B */

    /* Configure GPIO Pin : P3_16 as UART2_RX_B */
    ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_16, PINMUX_ALTERNATE_FUNCTION_2);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    /* Configure GPIO Pin : P3_17 as UART2_TX_B */
    ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_17, PINMUX_ALTERNATE_FUNCTION_2);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }
#elif UART == 4
    /* PINMUX UART4_B */

    /* Configure GPIO Pin : P3_1 as UART4_RX_B */
    ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_1, PINMUX_ALTERNATE_FUNCTION_1);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    /* Configure GPIO Pin : P3_2 as UART4_TX_B */
    ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_2, PINMUX_ALTERNATE_FUNCTION_1);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }
#else
    #error Unsupported UART!
#endif

    return 0;
}

void myUART_callback(uint32_t event)
{
    uart_event = event;
}

int tracelib_init()
{
    char  cmd    = 0;
    int32_t ret    = 0;
    uint32_t events = 0;
    ARM_DRIVER_VERSION version;

    printf("UART tracelib starting up...\n");

    version = USARTdrv->GetVersion();
    printf("\r\n UART version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize UART hardware pins using PinMux Driver. */
    ret = hardware_init();
    if(ret != 0)
    {
        printf("\r\n Error in UART hardware_init.\r\n");
        return ret;
    }

    /* Initialize UART driver */
    ret = USARTdrv->Initialize(myUART_callback);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Initialize.\r\n");
        return ret;
    }

    /* Power up UART peripheral */
    ret = USARTdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Power Up.\r\n");
        return ret;
    }

    /* Configure UART to 115200 Bits/sec */
    ret =  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                             ARM_USART_DATA_BITS_8       |
                             ARM_USART_PARITY_NONE       |
                             ARM_USART_STOP_BITS_1       |
                             ARM_USART_FLOW_CONTROL_NONE, 115200);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Control.\r\n");
        return ret;
    }

    /* Transmitter line */
    ret =  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Control TX.\r\n");
        return ret;
    }

    /* No RX
    ret =  USARTdrv->Control(ARM_USART_CONTROL_RX, 0); // RX disabled
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Control RX.\r\n");
        return ret;
    }
    */

    return ret;
}

int send_str(const char* str, uint32_t len)
{
    uart_event = 0;

    int32_t ret = USARTdrv->Send(str, len);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Send.\r\n");
        return ret;
    }

    while (!uart_event);
    return ret;
}

void tracef(const char * format, ...)
{
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  send_str(buffer, strlen(buffer));
  va_end(args);
}

#if 0 // power off
error_poweroff:

/* Received error Power off UART peripheral */
ret = USARTdrv->PowerControl(ARM_POWER_OFF);
if(ret != ARM_DRIVER_OK)
{
    printf("\r\n Error in UART Power OFF.\r\n");
}

error_uninitialize:

/* Received error Un-initialize UART driver */
ret = USARTdrv->Uninitialize();
if(ret != ARM_DRIVER_OK)
{
    printf("\r\n Error in UART Uninitialize.\r\n");
}

printf("\r\n XXX UART demo thread exiting XXX...\r\n");
}
#endif

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
