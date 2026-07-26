/******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.h
 *
 * Description: Header file for the UART AVR driver
 *
 * Author: Mohamed Tarek (modified for clarity and completeness)
 *
 ******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include "std_types.h"

/*******************************************************************************
 *                         Types Declaration                                   *
 *******************************************************************************/

/* Number of data bits per frame */
typedef enum
{
	FIVE_BITS,
	SIX_BITS,
	SEVEN_BITS,
	EIGHT_BITS,
	NINE_BITS
} UART_BitDataType;

/* Parity configuration */
typedef enum
{
	DISABLED_PARITY,
	EVEN_PARITY,
	ODD_PARITY
} UART_ParityType;

/* Stop bit configuration */
typedef enum
{
	ONE_STOP_BIT,
	TWO_STOP_BITS
} UART_StopBitType;

/* Baud rate options (optional enumeration if desired) */
typedef enum
{
	BAUD_2400   = 2400,
	BAUD_4800   = 4800,
	BAUD_9600   = 9600,
	BAUD_14400  = 14400,
	BAUD_19200  = 19200,
	BAUD_38400  = 38400,
	BAUD_57600  = 57600,
	BAUD_115200 = 115200
} UART_BaudRateType;

/* UART configuration structure */
typedef struct
{
	UART_BitDataType bit_data;
	UART_ParityType parity;
	UART_StopBitType stop_bit;
	UART_BaudRateType baud_rate;
} UART_ConfigType;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Initialize the UART device:
 *  - Setup frame format (data bits, parity, stop bits)
 *  - Enable UART
 *  - Setup baud rate
 */
void UART_init(const UART_ConfigType *Config_Ptr);

/*
 * Description :
 * Send one byte to another UART device.
 */
void UART_sendByte(uint8 data);

/*
 * Description :
 * Receive one byte from another UART device.
 */
uint8 UART_receiveByte(void);

/*
 * Description :
 * Send a null-terminated string through UART.
 */
void UART_sendString(const uint8 *Str);

/*
 * Description :
 * Receive a string through UART until the '#' symbol.
 * The '#' symbol will not be stored in the string.
 */
void UART_receiveString(uint8 *Str);

uint8 UART_dataReady(void);

#endif /* UART_H_ */
