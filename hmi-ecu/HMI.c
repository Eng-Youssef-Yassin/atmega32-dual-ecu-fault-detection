/*
 * HMI.c
 *
 * Created on: Nov 4, 2025
 * Author: Youssef Yassin
 */

/* ================== Includes ================== */
#include <avr/io.h>
#include "common_macros.h"
#include "gpio.h"
#include "keypad.h"
#include "lcd.h"
#include "std_types.h"
#include "timer.h"
#include "uart.h"
#include <util/delay.h>

/* ================== Protocol Codes ================== */
#define START_OPERATION_CODE 0x10
#define SEND_VALUE_CODE      0x20
#define SEND_ERRORS_CODE     0x30
#define STOP_SENSING_CODE    0x40

#define READY_TO_SEND        0x55
#define ACK_BYTE             0xAA

/* ================== Global Variables ================== */

// Timer-related global flags
volatile uint8 g_secCount = 0;   // Counts up to 10
volatile uint8 g_10secCount = 0; // Flag set every 10 seconds

// State variable to track if the system is running
uint8 g_is_system_active = 0; // 0 = Stopped, 1 = Started

// Global variables to store received sensor data
uint8 g_temp = 0;
uint8 g_win1 = 0;
uint8 g_win2 = 0;
uint8 g_dist = 0;
uint8 g_fault1 = 0;
uint8 g_fault2 = 0;

// Timer1 Configuration structure (used by helper functions)
Timer_ConfigType g_timer1_config = {
		.timer_ID = TIMER1_ID,
		.timer_Mode = TIMER_MODE_CTC,
		.timer_Clock = F_CPU_1024,
		.timer_InitialValue = 0,
		.timer_CompareMatchValue = 7812 // Value for 1-second tick with 8MHz F_CPU and 1024 prescaler
};

/* ================== Function Prototypes ================== */

/* --- Initialization --- */
void initialize_system(void);

/* --- Timer Callbacks and Helpers --- */
void Timer1_Handler(void);
void start_operation_timer(void);
void stop_operation_timer(void);

/* --- UART Communication Helpers --- */
void send_uart_command(uint8 command_code);

/* --- Main Menu Logic Handlers --- */
uint8 handle_menu_choice(uint8 pressed_key);
uint8 handle_start_operation(void);
uint8 handle_display_values(void);
uint8 handle_retrieve_faults(void);
uint8 handle_stop_system(void);

/* --- LCD Display Functions --- */
void lcd_display_main_menu(void);
void lcd_startMonitor_display(void);
void lcd_sensorValues_display(uint8 temp, uint8 dist, uint8 s1, uint8 s2);
void lcd_diplayAgain1(void);
void lcd_diplayAgain2(void);
void lcd_display_system_stopped(void);
void lcd_display_logged_faults(uint8 val1, uint8 val2);


/* ================== Main Function ================== */
int main(void)
{
	uint8 pressed_key = 0; // Stores the key to be processed

	initialize_system();

	for (;;) // Infinite loop
	{
		// If the last action was completed, reset key to 0
		// This block waits for a new choice from the main menu
		if (pressed_key == 0)
		{
			lcd_display_main_menu();
			while (!(pressed_key = KEYPAD_getPressedKey())); // Wait for user
		}

		// Check if the choice is valid based on system state
		if ( (g_is_system_active == 0) && (pressed_key != 1) && (pressed_key != 4) )
		{
			// If system is stopped, only 'Start' (1) is allowed
			// (and 'Stop' (4), which does nothing)
			// Any other key is ignored.
			pressed_key = 0; // Go back to main menu
		}
		else
		{
			// Process the user's choice and get the next state
			// The handler function will return 0 to go to main menu,
			// or 2/3 to "display again".
			pressed_key = handle_menu_choice(pressed_key);
		}
	}

	return 0; // Should never be reached
}

/* ================== Initialization Function ================== */
void initialize_system(void)
{
	// Enable Global Interrupt I-Bit
	SREG |= (1 << 7);

	// Setup Timer1 callback
	Timer_setCallBack(Timer1_Handler, TIMER1_ID);

	// Initialize peripherals
	LCD_init();

	// UART configuration
	UART_ConfigType uartConfig = {
			EIGHT_BITS,
			DISABLED_PARITY,
			ONE_STOP_BIT,
			BAUD_9600
	};
	UART_init(&uartConfig);
}


/* ================== Timer Functions ================== */

void Timer1_Handler(void)
{
	g_secCount++;
	if (g_secCount >= 10)
	{
		g_10secCount = 1; // Set the 10-second flag
		g_secCount = 0;   // Reset the 1-second counter
	}
}

/**
 * @brief Resets flags and starts Timer1.
 */
void start_operation_timer(void)
{
	g_secCount = 0;
	g_10secCount = 0;
	Timer_init(&g_timer1_config);
}

/**
 * @brief Stops Timer1.
 */
void stop_operation_timer(void)
{
	Timer_deInit(TIMER1_ID);
}


/* ================== UART Helper ================== */

/**
 * @brief Performs the standard 3-way handshake to send a command.
 * 1. Send "Ready"
 * 2. Wait for "Ack"
 * 3. Send the actual command
 */
void send_uart_command(uint8 command_code)
{
	UART_sendByte(READY_TO_SEND);
	while (UART_receiveByte() != ACK_BYTE);
	UART_sendByte(command_code);
}


/* ================== Main Application Logic ================== */

/**
 * @brief Calls the correct function based on the user's key press.
 * @param pressed_key The key (1-4) pressed by the user.
 * @return The next state (0 = main menu, 2 = display again, 3 = display again)
 */
uint8 handle_menu_choice(uint8 pressed_key)
{
	uint8 next_key_state = 0;

	switch (pressed_key)
	{
	case 1:
		next_key_state = handle_start_operation();
		break;
	case 2:
		next_key_state = handle_display_values();
		break;
	case 3:
		next_key_state = handle_retrieve_faults();
		break;
	case 4:
		next_key_state = handle_stop_system();
		break;
	}
	return next_key_state;
}

/**
 * @brief Handles Menu Option 1: Start Operation
 */
uint8 handle_start_operation(void)
{
	start_operation_timer();
	send_uart_command(START_OPERATION_CODE);
	lcd_startMonitor_display();
	g_is_system_active = 1; // Mark system as active

	// Wait for 10 seconds (timer flag)
	while (g_10secCount == 0);

	stop_operation_timer();
	return 0; // Go back to main menu
}

/**
 * @brief Handles Menu Option 2: Display Sensor Values
 */
uint8 handle_display_values(void)
{
	start_operation_timer();
	send_uart_command(SEND_VALUE_CODE);

	// Loop for 10 seconds, polling for new data
	while (g_10secCount == 0)
	{
		// Wait for Control MCU to be ready
		while (UART_receiveByte() != READY_TO_SEND);
		UART_sendByte(ACK_BYTE); // Send acknowledgment

		// Receive 4 bytes of data
		g_dist = UART_receiveByte();
		g_temp = UART_receiveByte();
		g_win1 = UART_receiveByte();
		g_win2 = UART_receiveByte();

		// Update the LCD
		lcd_sensorValues_display(g_temp, g_dist, g_win1, g_win2);
		_delay_ms(500); // Display for a short time

		// Send request again for the next loop
		send_uart_command(SEND_VALUE_CODE);
	}

	// Get one final reading after the 10s is up
	while (UART_receiveByte() != READY_TO_SEND);
	UART_sendByte(ACK_BYTE);
	g_dist = UART_receiveByte();
	g_temp = UART_receiveByte();
	g_win1 = UART_receiveByte();
	g_win2 = UART_receiveByte();

	stop_operation_timer();

	// Ask user if they want to display again
	lcd_diplayAgain1();
	uint8 key = 0;
	while (!(key = KEYPAD_getPressedKey()));

	if (key == 2)
		return 2; // Return 2 to re-run this function
	else
		return 0; // Return 0 for main menu
}

/**
 * @brief Handles Menu Option 3: Retrieve Logged Faults
 */
uint8 handle_retrieve_faults(void)
{
	start_operation_timer();
	send_uart_command(SEND_ERRORS_CODE);

	// Loop for 10 seconds, polling for new data
	while (g_10secCount == 0)
	{
		// Handshake
		while (UART_receiveByte() != READY_TO_SEND);
		UART_sendByte(ACK_BYTE);

		// Receive 2 bytes: fault1, fault2
		g_fault1 = UART_receiveByte();
		g_fault2 = UART_receiveByte();

		lcd_display_logged_faults(g_fault1, g_fault2);
		_delay_ms(500);

		// Send request again for the next loop
		send_uart_command(SEND_ERRORS_CODE);
	}

	// Get one final reading after the 10s is up
	while (UART_receiveByte() != READY_TO_SEND);
	UART_sendByte(ACK_BYTE);
	g_fault1 = UART_receiveByte();
	g_fault2 = UART_receiveByte();

	stop_operation_timer();

	// Ask user if they want to display again
	lcd_diplayAgain2();
	uint8 key = 0;
	while (!(key = KEYPAD_getPressedKey()));

	if (key == 3)
		return 3; // Return 3 to re-run this function
	else
		return 0; // Return 0 for main menu
}

/**
 * @brief Handles Menu Option 4: Stop System
 */
uint8 handle_stop_system(void)
{
	lcd_display_system_stopped();
	send_uart_command(STOP_SENSING_CODE);
	g_is_system_active = 0; // Mark system as stopped

	// Start timer to display the "stopped" message for 10 seconds
	start_operation_timer();
	while (g_10secCount == 0);
	stop_operation_timer();

	return 0; // Go back to main menu
}


/* ================== LCD Display Functions ================== */
/* (These functions are unchanged from your original code) */

void lcd_display_main_menu(void)
{
	LCD_clearScreen();
	LCD_moveCursor(0, 0);
	LCD_displayString("1)startOperation");
	LCD_moveCursor(1, 0);
	LCD_displayString("2)display values");
	LCD_moveCursor(2, 0);
	LCD_displayString("3)RetrieveFaults");
	LCD_moveCursor(3, 0);
	LCD_displayString("4)stopMonitoring");
}

void lcd_startMonitor_display(void)
{
	LCD_clearScreen();
	LCD_moveCursor(0, 0);
	LCD_displayString("operationStarted");
	LCD_moveCursor(1, 0);
	LCD_displayString("MonitorActived!");
}

void lcd_sensorValues_display(uint8 temp, uint8 dist, uint8 s1, uint8 s2)
{
	LCD_clearScreen();
	LCD_moveCursor(0, 0);
	LCD_displayString("Temp:");
	LCD_intgerToString(temp);
	LCD_displayString("C");
	LCD_moveCursor(1, 0);
	LCD_displayString("Dist:");
	LCD_intgerToString(dist);
	LCD_displayString("cm");
	LCD_moveCursor(2, 0);
	LCD_displayString("win1:");
	LCD_displayString((s1 == 0) ? "closed" : "open");
	LCD_moveCursor(3, 0);
	LCD_displayString("win2:");
	LCD_displayString((s2 == 0) ? "closed" : "open");
}

void lcd_diplayAgain1(void)
{
	LCD_clearScreen();
	LCD_moveCursor(0, 0);
	LCD_displayString("Display again?");
	LCD_moveCursor(1, 0);
	LCD_displayString("press2=yes");
	LCD_moveCursor(2, 0);
	LCD_displayString("other=main menu");
}

void lcd_diplayAgain2(void)
{
	LCD_clearScreen();
	LCD_moveCursor(0, 0);
	LCD_displayString("Display again?");
	LCD_moveCursor(1, 0);
	LCD_displayString("press3=yes");
	LCD_moveCursor(2, 0);
	LCD_displayString("other=main menu");
}

void lcd_display_system_stopped(void)
{
	LCD_clearScreen();
	LCD_moveCursor(0, 0);
	LCD_displayString("system monitor");
	LCD_moveCursor(1, 0);
	LCD_displayString("stopped");
	LCD_moveCursor(2, 0);
	LCD_displayString("returning to ");
	LCD_moveCursor(3, 0);
	LCD_displayString("main menu");
}

void lcd_display_logged_faults(uint8 val1, uint8 val2)
{
	LCD_clearScreen();
	LCD_moveCursor(0, 0);
	LCD_displayString("logged faults");
	if (val1 != 0)
	{
		LCD_moveCursor(1, 0);
		LCD_displayString("p002:overheat");
	}
	if (val2 != 0)
	{
		LCD_moveCursor(2, 0);
		LCD_displayString("p001:close");
	}
	if ((val1 == 0) && (val2 == 0))
	{
		LCD_moveCursor(1, 0);
		LCD_displayString("no errors");
	}
	LCD_moveCursor(3, 0);
	LCD_displayString("--end of list--");
}
