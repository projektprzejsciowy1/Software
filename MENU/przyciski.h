/*
 * przyciski.h
 *
 *  Created on: 2014-01-10
 *      Author: Przemek
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

//Biblioteka napisana na podstaiwe informacji ze strony http://mirekk36.blogspot.com/

#ifndef PRZYCISKI_H_
#define PRZYCISKI_H_

#define przycisk1_ddr DDRB
#define przycisk1_port PORTB
#define przycisk1_pin PINB
#define przycisk1 (1 << PB0)

#define przycisk2_ddr DDRB
#define przycisk2_port PORTB
#define przycisk2_pin PINB
#define przycisk2 (1 << PB1)

#define przycisk3_ddr DDRB
#define przycisk3_port PORTB
#define przycisk3_pin PINB
#define przycisk3 (1 << PB2)

#define przycisk4_ddr DDRB
#define przycisk4_port PORTB
#define przycisk4_pin PINB
#define przycisk4 (1 << PB3)

#define przycisk5_ddr DDRB
#define przycisk5_port PORTB
#define przycisk5_pin PINB
#define przycisk5 (1 << PB4)

#define przycisk1_ON (!(przycisk1_pin & przycisk1))
#define przycisk1_OFF (przycisk1_pin & przycisk1)

#define przycisk2_ON (!(przycisk2_pin & przycisk2))
#define przycisk2_OFF (przycisk2_pin & przycisk2)

#define przycisk3_ON (!(przycisk3_pin & przycisk3))
#define przycisk3_OFF (przycisk3_pin & przycisk3)

#define przycisk4_ON (!(przycisk4_pin & przycisk4))
#define przycisk4_OFF (przycisk4_pin & przycisk4)

#define przycisk5_ON (!(przycisk5_pin & przycisk5))
#define przycisk5_OFF (przycisk5_pin & przycisk5)


#define zwloka_czasowa 65500
#define czas_przytrzymania 63000

void przyciski_init();
uint8_t przycisk1_obsluga();
uint8_t przycisk2_obsluga();
uint8_t przycisk3_obsluga();
uint8_t przycisk4_obsluga();
uint8_t przycisk5_obsluga();



#endif /* PRZYCISKI_H_ */
