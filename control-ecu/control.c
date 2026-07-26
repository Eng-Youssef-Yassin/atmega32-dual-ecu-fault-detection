/*
 * Created on: Nov 4, 2025
 * Author: Youssef Yassin

 */

/* ================== Includes ================== */
#include <avr/io.h>
#include <util/delay.h>
#include "adc.h"
#include "common_macros.h"
#include "external_eeprom.h"
#include "gpio.h"
#include "lm35_sensor.h"
#include "motor.h"
#include "motor2.h"
#include "pwm.h"
#include "std_types.h"
#include "twi.h"
#include "uart.h"
#include "ultrasonic.h"

/* ================== Defines and Constants ================== */

/* --- UART Command Protocol --- */
#define START_OPERATION_CODE 0x10
#define SEND_VALUE_CODE      0x20
#define SEND_ERRORS_CODE     0x30
#define STOP_SENSING_CODE    0x40

/* --- UART Handshake Bytes --- */
#define READY_TO_SEND        0x55
#define ACK_BYTE             0xAA

/* --- EEPROM Error Storage Addresses --- */
#define EEPROM_HEAT_ERROR_ADDR 0x70
#define EEPROM_DIST_ERROR_ADDR 0x80

/* --- Sensor Thresholds --- */
#define OVERHEAT_THRESHOLD     90
#define MIN_DISTANCE_THRESHOLD 10

/* --- Hardware Pin Definitions --- */
#define BUTTON_PORT            PORTB_ID
#define BUTTON_WIN1_CLOSE_PIN  PIN0
#define BUTTON_WIN1_OPEN_PIN   PIN1
#define BUTTON_WIN2_CLOSE_PIN  PIN2
#define BUTTON_WIN2_OPEN_PIN   PIN4

/* ================== Global Variables ================== */

// Window states: 0 = Closed, 1 = Open
volatile uint8 g_window1_state = 0;
volatile uint8 g_window2_state = 0;

// Sensor readings
uint8 g_temperature = 0;
uint8 g_distance = 0;

// Error flags
uint8 g_dist_error_flag = 0;
uint8 g_overheat_error_flag = 0;

/* ================== Function Prototypes ================== */

// System setup
void initialize_system(void);

// Window motor controls
void window1_open(void);
void window1_close(void);
void window2_open(void);
void window2_close(void);
void window_control(void); // Checks buttons

// Sensor and error logic
void read_all_sensors(void);
void update_error_flags(void);
void check_and_store_errors(void);

// UART communication and protocol handlers
void wait_for_start_command(void);
uint8 receive_command(void);
void handle_command_send_values(void);
void handle_command_send_errors(void);
void process_main_loop(void);

/* ================== Main Function ================== */

int main()
{
	// Set up all hardware
	initialize_system();

	while (1)
	{
		// 1. Wait until the other device signals to start
		wait_for_start_command();

		// 2. Run the main operational loop
		process_main_loop();

		// 3. If the loop is broken (by STOP command), we return here
		//    and wait for the next START command.
	}
	return 0; // Should never be reached
}

/* ================== System Initialization ================== */

void initialize_system(void)
{
	// Enable global interrupts
	SREG |= (1 << 7);

	// Initialize motors
	DcMotor_Init();
	DcMotor_Init2();

	// Initialize sensors
	LM35_init();
	Ultrasonic_init();

	// Configure UART
	UART_ConfigType uartConfig = {
			EIGHT_BITS,
			DISABLED_PARITY,
			ONE_STOP_BIT,
			BAUD_9600
	};
	UART_init(&uartConfig);

	// Setup button pins as inputs
	GPIO_setupPinDirection(BUTTON_PORT, BUTTON_WIN1_CLOSE_PIN, PIN_INPUT);
	GPIO_setupPinDirection(BUTTON_PORT, BUTTON_WIN1_OPEN_PIN, PIN_INPUT);
	GPIO_setupPinDirection(BUTTON_PORT, BUTTON_WIN2_CLOSE_PIN, PIN_INPUT);
	GPIO_setupPinDirection(BUTTON_PORT, BUTTON_WIN2_OPEN_PIN, PIN_INPUT);
}

/* ================== Main Operational Loop ================== */

void process_main_loop(void)
{
	uint8 received_command = 0;

	for (;;) // This is an infinite loop, same as while(1)
	{
		// Always check buttons for immediate window control
		window_control();

		// Wait for the other device to be ready, then get a command
		received_command = receive_command();

		// Read sensors and check for new errors
		read_all_sensors();
		check_and_store_errors();

		// Handle the command we just received
		switch (received_command)
		{
			case SEND_VALUE_CODE:
				handle_command_send_values();
				break;

			case SEND_ERRORS_CODE:
				handle_command_send_errors();
				break;

			case STOP_SENSING_CODE:
				// Exit this function and go back to main
				return;
		}
	}
}

/* ================== UART Communication Functions ================== */

/**
 * @brief Halts the program until a START command is received over UART.
 */
void wait_for_start_command(void)
{
	// Keep looping until we get the specific start command
	while (1)
	{
		// Wait for the "ready" signal from the other device
		while (UART_receiveByte() != READY_TO_SEND);
		UART_sendByte(ACK_BYTE); // Send acknowledgment

		// Check if the next byte is the START command
		if (UART_receiveByte() == START_OPERATION_CODE)
		{
			break; // Exit the loop and continue
		}
		// If not, the loop repeats, waiting for the next "ready" signal
	}
}

/**
 * @brief Performs the handshake to receive a single command byte.
 * @return The command byte received from UART.
 */
uint8 receive_command(void)
{
	// Wait for the other device to signal it's ready to send
	while (UART_receiveByte() != READY_TO_SEND);

	// Acknowledge that we are ready to receive
	UART_sendByte(ACK_BYTE);

	// Return the command byte
	return UART_receiveByte();
}

/**
 * @brief Handles the SEND_VALUE_CODE command.
 * Performs handshake and sends current sensor/window status.
 */
void handle_command_send_values(void)
{
	// Signal that we are ready to send data
	UART_sendByte(READY_TO_SEND);
	// Wait for the other device to acknowledge
	while (UART_receiveByte() != ACK_BYTE);

	// Send the data
	UART_sendByte(g_distance);
	UART_sendByte(g_temperature);
	UART_sendByte(g_window1_state);
	UART_sendByte(g_window2_state);
}

/**
 * @brief Handles the SEND_ERRORS_CODE command.
 * Reads error flags from EEPROM and sends them.
 */
void handle_command_send_errors(void)
{
	uint8 heat_error = 0;
	uint8 dist_error = 0;

	// Read stored error flags from non-volatile memory
	EEPROM_readByte(EEPROM_HEAT_ERROR_ADDR, &heat_error);
	_delay_ms(10); // Give EEPROM time to process
	EEPROM_readByte(EEPROM_DIST_ERROR_ADDR, &dist_error);
	_delay_ms(10); // Give EEPROM time to process

	// Signal that we are ready to send data
	UART_sendByte(READY_TO_SEND);
	// Wait for the other device to acknowledge
	while (UART_receiveByte() != ACK_BYTE);

	// Send the error flags
	UART_sendByte(heat_error);
	UART_sendByte(dist_error);
}

/* ================== Sensor and Error Functions ================== */

/**
 * @brief Reads all connected sensors and updates global variables.
 */
void read_all_sensors(void)
{
	g_temperature = LM35_getTemperature();
	g_distance = Ultrasonic_readDistance();
}

/**
 * @brief Checks sensor values against thresholds and updates flag variables.
 */
void update_error_flags(void)
{
	// Use a ternary operator (a simple if/else) to set flags
	g_overheat_error_flag = (g_temperature > OVERHEAT_THRESHOLD) ? 1 : 0;
	g_dist_error_flag = (g_distance < MIN_DISTANCE_THRESHOLD) ? 1 : 0;
}

/**
 * @brief Updates error flags and writes them to EEPROM if an error is detected.
 */
void check_and_store_errors(void)
{
	// First, update the flags based on current sensor readings
	update_error_flags();

	// If an error is present, save it to persistent memory
	if (g_overheat_error_flag == 1)
	{
		EEPROM_writeByte(EEPROM_HEAT_ERROR_ADDR, g_overheat_error_flag);
		_delay_ms(10); // Give EEPROM time to write
	}

	if (g_dist_error_flag == 1)
	{
		EEPROM_writeByte(EEPROM_DIST_ERROR_ADDR, g_dist_error_flag);
		_delay_ms(10); // Give EEPROM time to write
	}
}

/* ================== Window Control Functions ================== */

/**
 * @brief Opens window 1.
 */
void window1_open(void)
{
	DcMotor_Rotate(CW);
	_delay_ms(1000); // Run motor for 1 second
	DcMotor_Rotate(STOP);
	g_window1_state = 1; // Update state to "Open"
}

/**
 * @brief Closes window 1.
 */
void window1_close(void)
{
	DcMotor_Rotate(ACW);
	_delay_ms(1000); // Run motor for 1 second
	DcMotor_Rotate(STOP);
	g_window1_state = 0; // Update state to "Closed"
}

/**
 * @brief Opens window 2.
 */
void window2_open(void)
{
	DcMotor_Rotate2(CW2);
	_delay_ms(1000); // Run motor for 1 second
	DcMotor_Rotate2(STOP2);
	g_window2_state = 1; // Update state to "Open"
}

/**
 * @brief Closes window 2.
 */
void window2_close(void)
{
	DcMotor_Rotate2(ACW2);
	_delay_ms(1000); // Run motor for 1 second
	DcMotor_Rotate2(STOP2);
	g_window2_state = 0; // Update state to "Closed"
}

/**
 * @brief Checks all window control buttons and triggers actions.
 */
void window_control(void)
{
	// Check if the "Open Window 1" button is pressed
	if (BIT_IS_SET(PINB, BUTTON_WIN1_OPEN_PIN))
	{
		window1_open();
	}

	// Check if the "Close Window 1" button is pressed
	if (BIT_IS_SET(PINB, BUTTON_WIN1_CLOSE_PIN))
	{
		window1_close();
	}

	// Check if the "Open Window 2" button is pressed
	if (BIT_IS_SET(PINB, BUTTON_WIN2_OPEN_PIN))
	{
		window2_open();
	}

	// Check if the "Close Window 2" button is pressed
	if (BIT_IS_SET(PINB, BUTTON_WIN2_CLOSE_PIN))
	{
		window2_close();
	}
}
