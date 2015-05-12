/*
 * przyciski.c
 *
 *  Created on: 2014-01-10
 *      Author: Przemek
 *  Obsluga przyciskow zaczerpnieta z http://mirekk36.blogspot.com/
 */

#include <avr/io.h>
#include "przyciski.h"

//Biblioteka napisana na podstaiwe informacji ze strony http://mirekk36.blogspot.com/

uint16_t key1_lock = 0, key2_lock = 0, key3_lock = 0,
			key4_lock = 0, key5_lock = 0;

void przyciski_init( void )
{

	przycisk1_port |= przycisk1;
	przycisk2_port |= przycisk2;
	przycisk3_port |= przycisk3;
	przycisk4_port |= przycisk4;
	przycisk5_port |= przycisk5;
}
uint8_t przycisk1_obsluga(void)
{
	if(!key1_lock && przycisk1_ON)
	{
		key1_lock = zwloka_czasowa;
		return 1;

	} else if( key1_lock && przycisk1_OFF) key1_lock++;
	return 0;
}

uint8_t przycisk2_obsluga(void)
{
	if(!key2_lock && przycisk2_ON)
	{
		key2_lock = zwloka_czasowa;
		return 1;
	} else if( key2_lock && przycisk2_OFF) key2_lock++;
	return 0;
}

uint8_t przycisk3_obsluga(void)
{
	if(!key3_lock && przycisk3_ON)
	{
		key3_lock = zwloka_czasowa;
		return 1;
	} else if( key3_lock && przycisk3_OFF) key3_lock++;
	return 0;
}

uint8_t przycisk4_obsluga(void)
{
	if(!key4_lock && przycisk4_ON)
	{
		key4_lock = zwloka_czasowa;
		return 1;
	} else if( key4_lock && przycisk4_OFF) key4_lock++;
	return 0;
}

uint8_t przycisk5_obsluga(void)
{
	if(!key5_lock && przycisk5_ON)
	{
		key5_lock = zwloka_czasowa;
		return 1;
	} else if( key5_lock && przycisk5_OFF) key5_lock++;
	return 0;
}

