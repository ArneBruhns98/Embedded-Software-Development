/**
  ******************************************************************************
  * @file           : onewire_DS1820.c
  * @brief          : Implements Functions to communicate with the DS1820 Sensor
  * 				  via 1wire protocol
  ******************************************************************************
  *
  * @author			: Felix Lohse & Arne Bruhns
  *
  *
  ******************************************************************************
  */
#include "onewire_DS1820.h"

/* Commands for DS1820 -------------------------------------------------------*/
#define SKIP_ROM 			0xCC
#define CONVERT_T 			0x44
#define READ_SCRATCHPAD		0xBE

/* Time definitions in us ----------------------------------------------------*/
#define RESET					500
#define SEND_LONG				60
#define SEND_SHORT				5
#define READ_LOW				1
#define READ_WAIT				10

/* Private prototypes --------------------------------------------------------*/
void set_pin_low_then_high(uint16_t low_time, uint16_t high_time);
void send_bit(uint8_t bit);
void send_byte(uint8_t byte);
uint8_t receive_bit();
uint8_t receive_byte();
void wait_for_pullup(uint16_t time);
void reset_bus();
void receive_scratchpad(uint8_t* scratchpad);
int16_t calculate_temp(uint8_t* scratchpad);

/**
  * @brief Sets OneWire_Pin to low -> delays -> sets pin to high -> delays
  * 	   Used to create bit writing slots and reading slots for the master.
  * 	   Also used to generate reset pulses.
  * @param uint16_t low_time amount of time in us the pin should be pulled to low
  * @param uint16_t high_time amount of time in us the pin be released so it can be pulled high by pull up
  * @retval None
  */
void set_pin_low_then_high(uint16_t low_time, uint16_t high_time) {
	HAL_GPIO_WritePin(OneWire_DS1820_GPIO_Port, OneWire_DS1820_Pin, GPIO_PIN_RESET);
	delayUs(low_time);
	HAL_GPIO_WritePin(OneWire_DS1820_GPIO_Port, OneWire_DS1820_Pin, GPIO_PIN_SET);
	delayUs(high_time);
}

/**
  * @brief Sends single bits to 1wire channel, by creating a write slot
  * @param uint8_t bit bit to send, should only be used with bit values, not bytes, even though
  * 		   it's possible
  * @retval None
  */
void send_bit(uint8_t bit) {
	bit == 0 ? set_pin_low_then_high(SEND_LONG, SEND_SHORT) : set_pin_low_then_high(SEND_SHORT, SEND_LONG);
}

/**
  * @brief Sends a full byte to 1wire channel, by creating write slots.
  * 	   Byte is shifted to right by for loops index and bitwise and
  * 	   to select the LSB first.
  * @param uint8_t byte byte to send
  */
void send_byte(uint8_t byte) {
	for (uint8_t i = 0; i < 8; i++) {
		send_bit(byte >> i & 1);
	}
}

/**
  * @brief Function used to receive one single bit from the 1wire channel.
  * 	   Creates a master read slot by pulling the channel to low. Afterwards
  * 	   samples the current state of the channel and interprets it as 1 or 0.
  * 	   Also handles reconfiguration for the pin.
  * @retval uint8_t bit bit read from the 1wire channel.
  */
uint8_t receive_bit() {
	uint8_t bit = 0;
	set_pin_low_then_high(READ_LOW, READ_WAIT);						// Create a read slot and wait 10 us
	bit = (HAL_GPIO_ReadPin(OneWire_DS1820_GPIO_Port, OneWire_DS1820_Pin) ? 1 : 0);
	delayUs(40);
	return bit;
}

/**
  * @brief Receives a full byte from the 1wire channel, by reading eight single
  * 	   bits. Inserts bit to correct index of the byte. First bit in is MSB in return byte.
  * @retval uint8_t bit byte read from the 1wire channel.
  */
uint8_t receive_byte() {
	uint8_t byte = 0;
	for (uint8_t i = 0; i < 8; i++) {
		byte += receive_bit() << i;
	}
	return byte;
}

/**
  * @brief Function used to let the master wait till temperature conversion is done on DS1820.
  *        DS1820 pulls 1wire to low after receiving Convert T (44h) command, releases the channel and
  *        the channel gets pulled to high by pull up, after conversion is done.
  * @param uint16_t time amount of time to wait before sampling.
  * @retval None
  */
void wait_for_pullup(uint16_t time) {
	delayUs(time);
	while (HAL_GPIO_ReadPin(OneWire_DS1820_GPIO_Port, OneWire_DS1820_Pin) == GPIO_PIN_RESET);
}

/**
  * @brief Resets the bus by sending 1wire specific reset pulse
  * 	   DS1820 will answer with presence pulse. But the presence pulse will be
  * 	   skipped, because there is a 500 us high_time scheduled.
  * @retval None
  */
void reset_bus() {
	set_pin_low_then_high(RESET, RESET);
}


/**
  * @brief Function used to get temperature data from DS1820.
  * 	   Follows protocol by:
  * 	   First Transaction:
  * 	   		- Send Reset pulse
  * 	   		- SKIP ROM (CCH)
  * 	   		- CONVERT T (44h)
  * 	   		- Wait till conversion is done
  *
  * 	   Second Transaction:
  * 	  	 	- Send Reset pulse
  * 	  	 	- READ SCRATCHPAD (BEh) by function receive_scratchpad
  *
  * 	  With the full scratchpad read, temperature calculation is done by
  * 	  calculate_temp.
  * @retval int16_t Returns the value returned by temperature calculation.
  */
int16_t get_temperature() {
	uint8_t scratchpad[] = {0,0,0,0,0,0,0,0,0};
	reset_bus();
	send_byte(SKIP_ROM);
	send_byte(CONVERT_T);
	wait_for_pullup(20);
	delayUs(20);
	reset_bus();
	send_byte(SKIP_ROM);
	send_byte(READ_SCRATCHPAD);

	receive_scratchpad(scratchpad);

	return calculate_temp(scratchpad);
}

/**
  * @brief Function used to read the whole scratchpad into a given pad.
  * @param uint8_t* scratchpad local scratchpad to fill
  * @retval None
  */
void receive_scratchpad(uint8_t* scratchpad) {
	for (uint8_t i = 0; i < 9; i++) {
		scratchpad[i] = receive_byte();
	}
}

/**
  * @brief Function used to encapsulate temperature calculation.
  * @param uint8_t* scratchpad local scratchpad to use for calculation
  * @retval int16_t the temperature in degrees C with 1 decimal place * 10
  */
int16_t calculate_temp(uint8_t* scratchpad) {
	int16_t val = (scratchpad[1] << 8) | scratchpad[0];

	uint8_t count_per_c = scratchpad[7];
	uint8_t count_remain = scratchpad[6];

    val = val >> 1;                   	  	// temp in degs C

    val <<= 4;								// times 16
    val -= 4;								// -0.25
    val = val + (count_per_c-count_remain);	// no division needed, because is already in 1/16 resolution
    val *= 10;								// for 1 decimal place
    val >>= 4;								// divide by 16

    return val;
}

/**
  * @brief Resets the bus by sending 1wire specific reset pulse.
  * 	   DS1820 will answer with presence pulse. Sampling for
  * 	   presence pulse and returning value if or if not presence
  * 	   pulse was received.
  * @retval uint8_t FALSE if not present, TRUE otherwise
  */
uint8_t get_presence() {
	uint8_t present = FALSE;
	set_pin_low_then_high(RESET, 30);		// Create Reset Pulse and wait 30 us
	present = (HAL_GPIO_ReadPin(OneWire_DS1820_GPIO_Port, OneWire_DS1820_Pin) ? TRUE : FALSE);
	delayUs(400);
	return present;
}



