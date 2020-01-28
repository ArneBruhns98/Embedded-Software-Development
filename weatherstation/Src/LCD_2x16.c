/**
  ******************************************************************************
  * @file           : LCD_2x16.c
  * @brief          : Implements Functions to handle the LCD screen
  ******************************************************************************
  *
  * @author			: Felix Lohse & Arne Bruhns
  *
  *
  ******************************************************************************
  */

#include "LCD_2x16.h"

/* Instructions for the display --------------------------------------------------*/
#define LCD_8BIT_MODE 		0x30
#define LCD_4BIT_MODE 		0x20
#define LCD_TWOLINES_5_8  	0x28
#define DISPLAY_OFF	  		0x04
#define CLEAR_DISPLAY 		0x01
#define DISPLAY_ON	  		0x0C
#define CURSOR_ON		    0x0E
#define CURSOR_ON_BLINKING  0x0F
#define CURSOR_OFF_BLINKING	0x0D
#define CURSOR_OFF			0x0C
#define HOME				0x02
#define SET_CURSOR_SEC_LINE 0xC0
#define SET_CURSOR_HOURS	0x85
#define SET_CURSOR_MINS		0x88
#define SET_CURSOR_SECS		0x8B


/* String literals that represent the formats for creating the output on rows --*/
static const char* const time_only_first_row = "    %02d:%02d:%02d    ";
static const char* const time_second_row = "        %02d:%02d:%02d";
static const char* const empty_row = "                 ";
static const char* const temp_row = "    %s%d.%d""\xDF""C     ";
static const char* const humidity_row = "       %d%%       "; // double % for escaping

/* Private prototypes ----------------------------------------------------------*/
void send_byte_to_lcd(uint8_t byte);
void send_instruction(uint8_t byte);
void send_data(uint8_t byte);
void setRSInstruction();
void setRSData();
void send_enable_pulse();

/* -----------------------------------------------------------------------------*/

/* Arrays to iterate while while sending data/instruction --------------------- */
uint16_t LCD_PINS[4] = {DB4_Pin, DB5_Pin, DB6_Pin, DB7_Pin};
GPIO_TypeDef* LCD_PORTS[4] = {DB4_GPIO_Port, DB5_GPIO_Port, DB6_GPIO_Port, DB7_GPIO_Port};

/**
  * @brief Sets register select pin low in preparation for sending
  * 	   instructions to the display
  * @retval None
  */
void setRSInstruction() {
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
}

/**
  * @brief Sets register select pin high in preparation for sending
  * 	   data to the display
  * @retval None
  */
void setRSData() {
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
}

/**
  * @brief Sends an enable pulse to the display by setting the enable
  * 	   pin to high, 1 us delay, then to low, 1 us delay.
  * 	   This is needed to tell the display to process currently set
  * 	   bit string on inputs DB4-DB7.
  * @retval None
  */
void send_enable_pulse() {
	HAL_GPIO_WritePin(LCD_Enable_GPIO_Port, LCD_Enable_Pin, GPIO_PIN_SET);
	delayUs(1);
	HAL_GPIO_WritePin(LCD_Enable_GPIO_Port, LCD_Enable_Pin, GPIO_PIN_RESET);
	delayUs(1);
}

/**
  * @brief First sends a high nibble then a low nibble to the display each followed
  * 	   by an enable pulse. This function is specific to 4 bit mode. As it splits
  * 	   the parameter byte into to nibbles and sends them separatly. Not usable
  * 	   for 8 bit mode.
  * @param uint8_t byte byte as data or instruction to send to the display. Can be
  * 					 a instruction from the defined instructions in this file or a char
  * 					 as data.
  * @retval None
  */
void send_byte_to_lcd(uint8_t byte) {

	// Send high nibble
	for (uint8_t i = 0; i < 4; i++ ) {
		if (byte >> (i + 4) & 1) HAL_GPIO_WritePin(LCD_PORTS[i], LCD_PINS[i], GPIO_PIN_SET);
		else HAL_GPIO_WritePin(LCD_PORTS[i], LCD_PINS[i], GPIO_PIN_RESET);
	}
	send_enable_pulse();

	// send low nibble
	for (uint8_t i = 0; i < 4; i++ ) {
		if (byte >> i & 1) HAL_GPIO_WritePin(LCD_PORTS[i], LCD_PINS[i], GPIO_PIN_SET);
		else HAL_GPIO_WritePin(LCD_PORTS[i], LCD_PINS[i], GPIO_PIN_RESET);
	}
	send_enable_pulse();
}

/**
  * @brief Separate function to specifically send instructions to the display.
  * 	   Encapsules register selection sending instruction and waiting for the display
  * 	   to process the instruction
  * @param uint8_t byte instruction to send. Use only instructions defined in this file.
  * @retval None
  */
void send_instruction(uint8_t byte) {
	setRSInstruction();
	send_byte_to_lcd(byte);
	if (byte == CLEAR_DISPLAY || byte == HOME) HAL_Delay(2);
	else delayUs(50);
}

/**
  * @brief Separate function to specifically send data to the display.
  * 	   Encapsules register selection sending data and waiting for the display
  * 	   to process the instruction
  * @param uint8_t byte instruction to send. Use only instruction defined in this file.
  * @retval None
  */
void send_data(uint8_t byte) {
	setRSData();
	send_byte_to_lcd(byte);
	delayUs(50);
}

/**
  * @brief Initializes the display by following the sequence from the datasheet
  * 	   and hints from problem definition. Sequence is:
  * 	   		- Wait for Power Delay 50ms
  * 	   		- Set 8 bit mode
  * 	   		- Set 8 bit mode
  * 	   		- Set 4 bit mode
  * 	   		- Configure display to 2 lines and 5*8 characters
  * 	   		- Turn display off
  * 	   		- Clear display
  * 	   		- Turn display on
  * 	   	Afterwards the display is ready to use.
  * @retval None
  */
void init_display() {
	HAL_Delay(50);						// wait power on delay
	send_instruction(LCD_8BIT_MODE); 	// 8 bit mode
	send_instruction(LCD_8BIT_MODE);	// 8 bit mode
	send_instruction(LCD_4BIT_MODE);	// 4 bit mode
	send_instruction(LCD_TWOLINES_5_8); // 2 lines, 5*8 chars
	send_instruction(DISPLAY_OFF); 		// turn off
	send_instruction(CLEAR_DISPLAY); 	// clear display
	send_instruction(DISPLAY_ON); 	    // turn on
}

/**
  * @brief Actually writes data to the screen. Fills templates specified by parameter mode
  * 	   using result from get_temperature() called in main, the fields hours, mins, secs
  * 	   from the RTC_TimeTypeDef and the humidity read by adc. Also sets cursor if necessary. And
  * 	   selects templates in toggle mode, depending on toggle_mode.
  * @param float temperature representation of the read temperature, value is received by get_temperature()
  * 						 in main
  * @param RTC_TimeTypeDef gTime typedef containing current time info, handled by RTC
  * @param enum View_mode mode currently selected view mode to choose from templates
  * @param enum Time_frac_selected selected if in Time_conf mode, where to set the cursor
  * @param uint8_t toggle_mode if in Toggle_mode which is the current state.
  * @retval None
  */
void write_to_display(uint16_t humidity, int16_t temperature, RTC_TimeTypeDef gTime, enum View_mode mode, enum Time_frac_selected selected, uint8_t toggle_mode) {
	// reset cursor to home for write
	send_instruction(HOME);

	// Setting up strings to display
	char first_row[64];
	char sec_row[64];


	// Preparation to print temperature
	int16_t temp_int = 0;
	int16_t temp_frac = 0;
	char *temp_sign;
	if (temperature < 0) {
		temp_sign = "-";
		temp_int = -temperature / 10;
		temp_frac = -(temperature + (temp_int * 10));
	} else {
		temp_sign = " ";
		temp_int = temperature / 10;
		temp_frac = temperature - (temp_int * 10);
	}

	// Use sprintf to prepare strings that should be written to display.
	// Dependend on currently set View mode.
	switch (mode) {
		case Time_only:
			sprintf(first_row, time_only_first_row, gTime.Hours, gTime.Minutes, gTime.Seconds);
			sprintf(sec_row, empty_row);
			break;

		case Time_and_Temp:
			sprintf(first_row, temp_row, temp_sign, temp_int, temp_frac);
			sprintf(sec_row, time_second_row, gTime.Hours, gTime.Minutes, gTime.Seconds);
			break;

		case Temp_and_humidity:
			sprintf(first_row, temp_row, temp_sign, temp_int, temp_frac);
			sprintf(sec_row, humidity_row , humidity);
			break;

		case Time_conf:
			sprintf(first_row, time_only_first_row, gTime.Hours, gTime.Minutes, gTime.Seconds);
			sprintf(sec_row, empty_row);
			break;

		case Temp_humi_and_clock:
			if (toggle_mode == TRUE) {
				sprintf(first_row, temp_row, temp_sign, temp_int, temp_frac);
				sprintf(sec_row, humidity_row , humidity);
			} else {
				sprintf(first_row, time_only_first_row, gTime.Hours, gTime.Minutes, gTime.Seconds);
				sprintf(sec_row, empty_row);
			}
			break;
	}

	// Write first line to display
	for (int i = 0; first_row[i] != 0; i++) {
		send_data(first_row[i]);
	}

	// set cursor to second line
	send_instruction(SET_CURSOR_SEC_LINE);

	// Write second line to display
	for (int i = 0; sec_row[i] != 0; i++) {
		send_data(sec_row[i]);
	}


	// If there is a timefrac selected, enable cursor and set to correct position
	if (selected != None_sel) {
		send_instruction(CURSOR_ON_BLINKING);
		if (selected == hours_sel) send_instruction(SET_CURSOR_HOURS);
		else if (selected == mins_sel) send_instruction(SET_CURSOR_MINS);
		else if (selected == secs_sel) send_instruction(SET_CURSOR_SECS);
	} else send_instruction(CURSOR_OFF);

}



