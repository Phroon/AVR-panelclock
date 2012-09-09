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
 *               *------*
 * !Reset - PA2 =|1 U 20|= VCC
 *          PD0 =|2   19|= PB7 - SCK  [ISP]
 *          PD1 =|3   18|= PB6 - MISO [ISP]
 *          PA1 =|4   17|= PB5 - MOSI [ISP]
 *          PA0 =|5   16|= PB4 - OC1B
 *   INT0 - PD2 =|6   15|= PB3 - OC1A
 *        - PD3 =|7   14|= PB2 - OC0A
 *          PD4 =|8   13|= PB1 - SCL i2c
 *   OC0B - PD5 =|9   12|= PB0 - SDA i2c
 *          GND =|10  11|= PD6
 *               *------* 
 *
 * OC0A pin14 - milliseconds
 * OC0B pin9  - seconds 
 * OC1A pin15 - minutes
 * OC1B pin16 - hours
 * 
 * INT0 pin6
 *
 * PB0 - SDA
 * PB1 - SCL
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "i2cmaster.h"

#define DevChronodot 0x68

#define tic_second 0x11	    // (0xFF*(1/60))*2^2
#define tic_minute 0x11		// (0xFF*(1/60))*2^2
#define tic_hour   0x55		// (0xFF*(1/24))*2^3

void tic(void);
void update_meters(void);

uint16_t volatile sec;
uint16_t volatile min;
uint16_t volatile hr;

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
 
	sec = seconds * tic_second;
	min = minutes * tic_minute;
	hr  = hours * tic_hour;
	
	update_meters();
	
	//Disable Analog Comparator pg.168
	ACSR |= _BV(ACD);
	
	//Enable INT0 interrupt on rising edge pg.51
	MCUCR |= _BV(ISC01) | _BV(ISC00);
	
	//Set up Fast PWM on Timer 0, OC0A & OC0B pg.82
	TCCR0A |= _BV(COM0A1)  //Clear OC0A on compare match, set OC0A at TOP
			| _BV(COM0B1)  //Clear OC0B on compare match, set OC0B at TOP
			| _BV(WGM01) | _BV(WGM00); // Fast PWM mode 3 pg.84 [WGM2 in TCRR0B = 0]
	
	TCCR0B |= _BV(CS00); //No clock prescaling
	
	//Set up Fast PWM on Timer 1, OC1A & OC1B pg.111
	TCCR1A |= _BV(COM1A1) | _BV(COM1B1) //Clear OC1A/OC1B on compare match, set at TOP
			| _BV(WGM10);  //With WGM12 set in TCRR1B, Fast PWM, 8-bit mode
			
	TCCR1B |= _BV(WGM12) //With WGM10 set in TCRR1A, Fast PWM, 8-bit mode
			| _BV(CS10);  //No clock prescaling
			
	//TIMSK	|= ???  //Timer Interupts pg.166
	
	DDRB |= _BV(DDB2) | _BV(DDB3) | _BV(DDB4); //Pins as output pg.70
	DDRD |= _BV(DDD5);
	
	sei(); // Enable Interrupts
	
	//millis();
	
	for(;;);
}

void tic(void)
{
	sec += tic_second;
	
	if (sec > 59*tic_second) {
		sec = 0;
		min += tic_minute;
		
		if (min > 59*tic_minute) { 
			min = 0;
			hr += tic_hour;
			
			if (hr > 23*tic_hour) {
				hr = 0;
				// It is now tomorrow.
			}
		}
	}
	
	update_meters();
}

/*
 * OC0A pin14 - milliseconds
 * OC0B pin9  - seconds 
 * OC1A pin15 - minutes
 * OC1B pin16 - hours
 */
 
void update_meters(void)
{
	OCR0B = sec >> 2;
	OCR1A = min >> 2;
	OCR1B   = hr >> 3;
}

ISR(INT0_vect)
{
	static uint8_t i;
	
	i++;
	
	if (i >= 1024/256) {
		i = 0;
		// I think this should work...
		if( OCR0A++ == 255) {
			tic();
		}
	}
}
