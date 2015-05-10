/*
 * main.c
 *
 *  Created on: 3 maj 2015
 *      Author: Marcin Kowalski, £ukasz Tim, Adrian Wrotecki
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "dht11.h"
#include "lcd44780.h"

/*** definicje wzprowadzen ***/

#define 	GAS		PA0										//czujnik gazu
#define 	LCD_BL	PA7										//podswietlenie wyswietlacza

#define 	VREF	PB2										//wejscie odwracajace komparatora(napiecie referencyjne)
#define 	ZERO	PB3										//impuls przejscia przez zero
#define 	PWRK	PB4										//zalaczanie modulu GSM
#define		MOSI	PB5										//
#define 	MISO	PB6										//koncowki SPI
#define 	SCK		PB7										//

#define 	ERR		PC0										//dioda sygnalizujaca blad
#define 	BUZZ	PC1										//buzzer
#define 	IN1		PC2										//wejscie cyfrowe 1
#define 	IN2		PC3										//wejscie cyfrowe 2
#define 	IN3		PC4										//wejscie cyfrowe 3
#define		IN4		PC5										//wejscie cyfrowe 4
#define		TR5		PC6										//wyjscie sterujace triakiem(sterowanie fazowe)

#define 	RX		PD0										//USART
#define		TX		PD1										//
#define 	WR		PD2										//wyjscie przepisania z bufora wewnetrznego na wyjscia rozszerzonego PORTU
#define		OK		PD3										//klawisz OK
#define 	UP		PD4										//klawisz UP
#define		DN		PD5										//klawisz DOWN
#define		LT		PD6										//klawisz LEFT
#define		RT		PD7										//klawisz RIGHT

#define 	SWPORT 	PORTD									//PORT z przyciskami
#define		SWDDR	DDRD									//DDR z przyciskami
#define 	SWPIN	PIND									//PIN z przyciskami

/*** definicje SPI ***/
#define    SPI_SEND SPDR									//bufor SPI

/*** definicje bledow ***/
#define RX_ERR 					0							//flaga wystapienia bledu

/*** definicje flag sterujacych ***/
#define CMD_RDY					0							//flaga odebrania komendy AT od modulu

/*** definicje i zmienne UART ***/
#define UART_BAUD				19200						//predkosc UART w bodach
#define	UBRR_REG				(F_CPU/16/UART_BAUD-1)		//wyznaczenie wartosci rejestru okreslajacego predkosc
#define UART_BUF_SIZE			256							//rozmiar buforow nadawczego i odbiorczego w bajtach
#define UART_MASK				(UART_BUF_SIZE-1)			//maska dla bufora odbiorczego
#define UART_SEND				UCSRB |=(1<<UDRIE);			//uruchomienie przerwania wysylajacego dane z bufora

volatile char 		UART_RxBuf[UART_BUF_SIZE];				//utworzenie bufora nadawczego
volatile char 		UART_TxBuf[UART_BUF_SIZE];				//utworzenie bufora odbiorczego

volatile uint8_t 	tx_head;								//poczatek bufora cyklicznego nadawczego
volatile uint8_t	tx_tail;								//koniec bufora cyklicznego odbiorczego
volatile uint8_t	rx_head;								//poczatek bufora cyklicznego nadawczego
volatile uint8_t	rx_tail;								//koniec bufora cyklicznego odbiorczego

volatile uint8_t    errors;									//zmienna obslugujaca bledy
volatile uint8_t	ster;									//zmienna z flagami sterujacymi programem

char* commands[10];											//adresy do komend zapisanych w pamieci flash



int main(void)
{
	//commands[0]=PSTR("AT+CPIN");							//utworzenie odpowiedzi na komende w pamieci flash

	/*** inicjalizacja wyjsc/wejsc ***/

	DDRA|=1<<LCD_BL;										//wyprowadzenie podswietlenia wyswietlacza jako wyjscie
	PORTA|=1<<LCD_BL;										//zalaczenie podswietlenia wyswietlacza

	DDRB|=1<<PWRK;											//wyprowadzenie zalaczania modulu jako wyjscie

	DDRC|=1<<ERR|1<<BUZZ|1<<TR5;							//wyprowadzenia diody, buzzera, triaka jako wyjscia
	PORTC|=1<<ERR|1<<BUZZ;									//zalaczenie diody i buzzera

	SWPORT|=1<<OK|1<<UP|1<<DN|1<<LT|1<<RT;					//podciaganie do VCC dla klawiszy


	/*** inicjalizacja UART ***/

	UBRRH = (uint8_t) (UBRR_REG>>8);						// ustawienie predkoci
	UBRRL = (uint8_t) UBRR_REG;								//
	UCSRB |= (1<< RXEN)|(1<<TXEN);							//wlaczenie nadajnika i odbiornika
	UCSRC |= (1<<URSEL)|(3<<UCSZ0);							//8-bitowa ramka danych
	UCSRB |=(1<<RXCIE)|(1<<UDRIE);							//aktywacja przerwan od pustego bufora nadawczego i od odebrania


	/*** inicjalizacja SPI ***/

	DDRB |= (1<<MOSI)|(1<<SCK);								//MOSI,SCK jako wyjscia
	SPCR |= (1<<SPE)|(1<<MSTR)/*|(1<<SPR1)*/;				//tryb master, predkosc fosc/64


	/*** inicjalizacja TIMER 1 ***/

}

ISR(USART_UDRE_vect)										//przerwanie po oproznieniu rejestru nadawczego
{															//wykonuje sie dopoki jest cokolwiek do wyslania w buforze nadawczym
	if(tx_head != tx_tail)									//sprawdzamy czy jest cos do wyslania
	{
		tx_tail = (tx_tail+1) & UART_MASK;					//zwiekszamy indeks konca bufora nadawczego
		UDR =UART_TxBuf[tx_tail];							//przepisujemy znak do rejestru nadawczego
	}
	else UCSRB &= ~(1<<UDRIE);								//jesli wyslano wszystkie znaki wylaczenie przerwania
}

ISR(USART_RXC_vect)											//przerwanie od odebrania znaku
{
	uint8_t tmp_head;
	char data;

	data=UDR;												//pobieramy bajt danych
	tmp_head = (rx_head+1) & UART_MASK;						//obliczenie indeksu poczatku bufora
	if (tmp_head == rx_tail) errors|=1<<RX_ERR;				//blad nadpisania bufora odbiorczego, ustaw flage bledu odbiornika
	else
	{
		rx_head=tmp_head;									//zwiekszamy indeks poczatku bufora
		UART_RxBuf[tmp_head]=data;							//przepisujemy odebrany znak do bufora odbiorczego
	}
	if(data == 0x0D) ster|=1<<CMD_RDY;						//jesli odebrano znak konca komendy ustaw flage sterujaca

}
