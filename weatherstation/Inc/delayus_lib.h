/**
  ******************************************************************************
  * @file           : delayus_lib.h
  * @brief          : Header for delayus_lib.c file.
  *                   This file contains the header of the function used to
  *                   implement a delay in microseconds
  ******************************************************************************
  *
  * @author			: Felix Lohse & Arne Bruhns
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DELAYUS_LIB_H
#define __DELAYUS_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Used for types like uint16_t ----------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Public function prototypes ------------------------------------------------*/
void delayUs(uint16_t us);


#ifdef __cplusplus
}
#endif
#endif /* __DELAYUS_LIB_H */
