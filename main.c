/* Name: main.c - An AVR Panel Meter Clock
 * 
 *
 * Copyright (C) 2011-2012 Michael Wren
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
 /* Current Layout:
 *
 *               *----*
 * !Reset - PB5 =|o   |= VCC
 *          PB3 =|    |= PB2
 *   OC1B - PB4 =|    |= PB1 - OC1A
 *          GND =|    |= PB0 - OC0A
 *               *----* 
 *
 * OC0A pin5 = Blue
 * OC1A pin6 = Green
 * OC1B pin3 = Red
 * 
 * PB2 pin7 SDA
 * PB3 pin2 SCL
 */

#include <avr/io.h>
#include "i2cmaster.h"
#include "millis.h"

#define DevChronodot 0x68

int main(void)
{

	//unsigned char ret;
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	i2c_init();								// init I2C interface
	
	i2c_start_wait(DevChronodot+I2C_WRITE);	// set device address and write mode
	i2c_write(0x00);						// write address = 0
	
	i2c_rep_start(DevChronodot+I2C_READ);       // set device address and read mode
	seconds = i2c_readAck();                    // read one byte form address 0
	minutes = i2c_readAck();                    //  "    "    "    "     "    1
	hours = i2c_readNak();                      //  "    "    "    "     "    2
	i2c_stop();                                 // set stop condition = release bus
	
    seconds = (((seconds & 0b11110000)>>4)*10 + (seconds & 0b00001111)); // convert BCD to decimal
    minutes = (((minutes & 0b11110000)>>4)*10 + (minutes & 0b00001111)); // convert BCD to decimal
    hours = (((hours & 0b00100000)>>5)*20 + ((hours & 0b00010000)>>4)*10 + (hours & 0b00001111)); // convert BCD to decimal (assume 24 hour mode)
    
	millis();
	
	for(;;);
}


