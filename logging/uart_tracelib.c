/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

// Uncomment this to disable traces to UART
//#define DISABLE_UART_TRACE

#include "uart_tracelib.h"

#if !defined(DISABLE_UART_TRACE)
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
//#include "Driver_PINMUX_AND_PINPAD.h"
#include "pinconf.h"


#if defined(M55_HP)
    #define UART    4
#elif defined(M55_HE)
    #define UART    2
#elif defined(A32)
    #define UART    5   // Was 6 in RevA
#else
    #error "Undefined M55 CPU!"
#endif

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(UART);

/* UART Driver instance */
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(UART);

volatile uint32_t uart_event;
static bool initialized = false;
const char * tr_prefix = NULL;
static bool has_cb = false;
uint16_t prefix_len;
#define MAX_TRACE_LEN 256

static int hardware_init(void)
{
#if UART == 2
    // PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_0, PINMUX_ALTERNATE_FUNCTION_1); // P1_0:RX  (mux mode 1)
    // PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_1, PINMUX_ALTERNATE_FUNCTION_1); // P1_1:TX  (mux mode 1)
    pinconf_set(PORT_1, PIN_0, PINMUX_ALTERNATE_FUNCTION_1, 0);
    pinconf_set(PORT_1, PIN_1, PINMUX_ALTERNATE_FUNCTION_1, 1);
#elif UART == 4
    // PINMUX_Config(PORT_NUMBER_12, PIN_NUMBER_1, PINMUX_ALTERNATE_FUNCTION_2); // P12_1: RX  (mux mode 2)
    // PINMUX_Config(PORT_NUMBER_12, PIN_NUMBER_2, PINMUX_ALTERNATE_FUNCTION_2); // P12_2: TX  (mux mode 2)
    pinconf_set(PORT_12, PIN_1, PINMUX_ALTERNATE_FUNCTION_2, 0);
    pinconf_set(PORT_12, PIN_2, PINMUX_ALTERNATE_FUNCTION_2, 1);
#elif UART == 5
    // PINMUX_Config(PORT_NUMBER_5, PIN_NUMBER_2, PINMUX_ALTERNATE_FUNCTION_2); // P5_2: RX  (mux mode 2)
    // PINMUX_Config(PORT_NUMBER_5, PIN_NUMBER_3, PINMUX_ALTERNATE_FUNCTION_2); // P5_3: TX  (mux mode 2)
    pinconf_set(PORT_5, PIN_2, PINMUX_ALTERNATE_FUNCTION_2, 0); // P5_2: RX  (mux mode 2)
    pinconf_set(PORT_5, PIN_3, PINMUX_ALTERNATE_FUNCTION_2, 1); // P5_3: TX  (mux mode 2)
#else
    #error Unsupported UART!
#endif

    return 0;
}

int tracelib_init(const char * prefix, ARM_USART_SignalEvent_t cb_event)
{
    int32_t ret    = 0;

    tr_prefix = prefix;
    if (tr_prefix) {
        prefix_len = strlen(tr_prefix);
    } else {
        prefix_len = 0;
    }

    /* Initialize UART hardware pins using PinMux Driver. */
    ret = hardware_init();
    if(ret != 0)
    {
        return ret;
    }

    /* Initialize UART driver */
    if (cb_event) {
        has_cb = true;
    }
    ret = USARTdrv->Initialize(cb_event);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Power up UART peripheral */
    ret = USARTdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
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
        return ret;
    }

    /* Transmitter line */
    ret =  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Receiver line */
    ret =  USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    initialized = true;
    return ret;
}

int receive_str(char* str, uint32_t len)
{
    int ret = 0;
    if (initialized)
    {
        ret = USARTdrv->Receive(str, len);
        if(ret != ARM_DRIVER_OK)
        {
            return ret;
        }
        if (has_cb == false) {
            while (USARTdrv->GetRxCount() != len);
        }
    }
    return ret;
}

int send_str(const char* str, uint32_t len)
{
    int ret = 0;

    if (initialized)
    {
        uart_event = 0;
        ret = USARTdrv->Send(str, len);
        if(ret != ARM_DRIVER_OK)
        {
            return ret;
        }

        while (USARTdrv->GetTxCount() != len);
    }
    return ret;
}

void tracef(const char * format, ...)
{
    if (initialized)
    {
        static char buffer[MAX_TRACE_LEN];

        va_list args;
        va_start(args, format);
        if (prefix_len) {
            memcpy(buffer, tr_prefix, prefix_len);
        }
        vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);
        send_str(buffer, strlen(buffer));
        va_end(args);
    }
}

#else

int tracelib_init(const char * prefix, ARM_USART_SignalEvent_t cb_event)
{
    (void)prefix;
    (void)cb_event;
    return 0;
}

int receive_str(char* str, uint32_t len)
{
    (void)str;
    (void)len;
    return 0;
}

int send_str(const char* str, uint32_t len)
{
    (void)str;
    (void)len;
    return 0;
}

void tracef(const char * format, ...)
{
    (void)format;
}

#endif // DISABLE_UART_TRACE

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
