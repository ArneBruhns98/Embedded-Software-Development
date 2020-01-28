/**
  ******************************************************************************
  * @file           : onewire_DS1820.h
  * @brief          : Header for 1wire_DS1820.c file.
  *                   This file contains the headers of the functions used for
  *                   communicating to DS1820 via 1wire protocol
  ******************************************************************************
  *
  * @author			: Felix Lohse & Arne Bruhns
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ONEWIRE_DS1820_H
#define __ONEWIRE_DS1820_H

#ifdef __cplusplus
extern "C" {
#endif

/* Used for types like uint16_t ----------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Used for GPIO Ports and Pins ----------------------------------------------*/
#include "main.h"

/* Used for delay in us function ---------------------------------------------*/
#include "delayus_lib.h"


/* Public function prototypes ------------------------------------------------*/
int16_t get_temperature();
uint8_t get_presence();


#ifdef __cplusplus
}
#endif
#endif /* __ONEWIRE_DS1820_H */
