/************************************************************************
* Copyright 2020 ITESM                                                  *
*                                                                       *
*                                                                       *
* Second Partial Project                                                *
*                                                                       *
* Authors:                                                              *
* Jesús Enrique Luna Medina          A01632334                          *
* Daniela abigail Parrales Mejía     A01228629                          *
* Luis Cortés Leal                   A01631163                          *
*                                                                       *
* May  2020                                                             *
*                                                                       *
* This is a temperature sensor that reports in serial port (UART)		*
* and in a 16x2 LCD I2C													*
*                                                                       *
************************************************************************/

#include "stm32f1xx_hal.h"

/***********************************************************************/

void lcd_init (void);   /* !< initialize lcd */

void lcd_send_cmd (char cmd);  /* !< send command to the lcd */

void lcd_send_data (char data);  /* !< send data to the lcd */

void lcd_send_string (char *str);  /* !< send string to the lcd */

void lcd_clear (void);  

/***********************************************************************/