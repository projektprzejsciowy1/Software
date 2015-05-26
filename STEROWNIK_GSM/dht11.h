/*
 * dht11.h
 *
 *  Created on: 17 lut 2015
 *      Author: Marcin
 */

#ifndef DHT11_H_
#define DHT11_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define DHT_DATA PC7									// numer pinu do ktorego podlaczono wyjscie data dht11
#define DHT_DIR	 DDRC									//rejestr kierunku powyzszego pinu
#define DHT_PORT PORTC
#define DHT_PIN PINC


volatile uint8_t block[5];								//40-bitowa tablica na odczyt
uint8_t crc;											//suma kontrolna
uint8_t* dht_read(void);								//funkcja odczytujaca dane z czujnika zwraca wskaznik do tablicy block

#endif /* DHT11_H_ */
