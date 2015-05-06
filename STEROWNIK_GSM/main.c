/*
 * main.c
 *
 *  Created on: 3 maj 2015
 *      Author: Marcin Kowalski, £ukasz Tim, Adrian Wrotecki
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "dht11.h"
#include "lcd44780.h"

/*** definicje wzprowadzen ***/

#define 	GAS		PA0							//czujnik gazu
#define 	LCD_BL	PA7							//podswietlenie wyswietlacza

#define 	VREF	PB2							//wejscie odwracajace komparatora(napiecie referencyjne)
#define 	ZERO	PB3							//impuls przejscia przez zero
#define 	PWRK	PB4							//zalaczanie modulu GSM
#define		MOSI	PB5							//
#define 	MISO	PB6							//koncowki SPI
#define 	SCK		PB7							//

#define 	ERR		PC0							//dioda sygnalizujaca blad
#define 	BUZZ	PC1							//buzzer
#define 	IN1		PC2							//wejscie cyfrowe 1
#define 	IN2		PC3							//wejscie cyfrowe 2
#define 	IN3		PC4							//wejscie cyfrowe 3
#define		IN4		PC5							//wejscie cyfrowe 4
#define		TR5		PC6							//wyjscie sterujace triakiem(sterowanie fazowe)

#define 	RX		PD0							//USART
#define		TX		PD1							//
#define 	WR		PD2							//wyjscie przepisania z bufora wewnetrznego na wyjscia rozszerzonego PORTU
#define		OK		PD3							//klawisz OK
#define 	UP		PD4							//klawisz UP
#define		DN		PD5							//klawisz DOWN
#define		LT		PD6							//klawisz LEFT
#define		RT		PD7							//klawisz RIGHT

#define 	SWPORT 	PORTD						//PORT z przyciskami
#define		SWDDR	DDRD						//DDR z przyciskami
#define 	SWPIN	PIND						//PIN z przyciskami

#define    SPI_SEND SPDR						//bufor SPI



int main(void)
{
	/*** inicjalizacja wyjsc/wejsc ***/

	DDRA|=1<<LCD_BL;								//wyprowadzenie podswietlenia wyswietlacza jako wyjscie
	PORTA|=1<<LCD_BL;								//zalaczenie podswietlenia wyswietlacza

	DDRB|=1<<PWRK;									//wyprowadzenie zalaczania modulu jako wyjscie

	DDRC|=1<<ERR|1<<BUZZ|1<<TR5;					//wyprowadzenia diody, buzzera, triaka jako wyjscia
	PORTC|=1<<ERR|1<<BUZZ;							//zalaczenie diody i buzzera

	SWPORT|=1<<OK|1<<UP|1<<DN|1<<LT|1<<RT;			//podciaganie do VCC dla klawiszy


	/*** inicjalizacja UART ***/



	/*** inicjalizacja SPI ***/

	DDRB |= (1<<MOSI)|(1<<SCK);						//MOSI,SCK jako wyjscia
	SPCR |= (1<<SPE)|(1<<MSTR)/*|(1<<SPR1)*/;		//tryb master, predkosc fosc/64


	/*** inicjalizacja TIMER 1 ***/

}
