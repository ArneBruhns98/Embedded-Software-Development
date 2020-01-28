/**
  ******************************************************************************
  * @file           : LCD_2x16.h
  * @brief          : Header for LCD_2x16.h.c file.
  *                   This file contains the headers of the functions used for
  *                   handling the LCD screen in our project. E.g. Settings modes,
  *                   printing text to screen...
  *                   This is not general purpose but rather specific for our
  *                   weatherstation.
  ******************************************************************************
  *
  * @author			: Felix Lohse & Arne Bruhns
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_2X16_H
#define __LCD_2X16_H

#ifdef __cplusplus
extern "C" {
#endif


/* Used for types like uint16_t ----------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Used for GPIO Ports and Pins ----------------------------------------------*/
#include "main.h"

/* Used for delay in us function ---------------------------------------------*/
#include "delayus_lib.h"

/* Used for sprintf() --------------------------------------------------------*/
#include <stdio.h>


/* View_mode represents the currently set mode, for template selection ---------*/
enum View_mode
{
	Time_and_Temp,				// Temp in first row, time in second
	Temp_and_humidity,			// Temp in first row, humidity in second
	Time_only,					// Only current Time is shown in first row
	Temp_humi_and_clock,		// Temp and humidity alternating with Time_only
	Time_conf					// Time is shown in first row, has blinking cursor
};

/* Used to show selected part of time for changing purpose in Time_conf---------*/
enum Time_frac_selected
{
	hours_sel,					// cursor is set on hours
	mins_sel,					// cursor is set on minutes
	secs_sel,					// cursor is set on seconds
	None_sel					// cursor is disabled
};

/* Public function prototypes ---------------------------------------------------*/
void init_display();
void write_to_display(uint16_t humidity, int16_t temperature, RTC_TimeTypeDef gTime, enum View_mode mode, enum Time_frac_selected selected, uint8_t toggle_mode);


#ifdef __cplusplus
}
#endif
#endif /* __LCD_2X16_H */
