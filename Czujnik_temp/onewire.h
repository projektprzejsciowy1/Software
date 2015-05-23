#ifndef onewire
#define onewire

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define DQ 	PB0
#define OWPORT PORTB
#define OWDDR DDRB
#define OWPIN PINB
#define PULLUP PB1

char blad;
unsigned char obecnosc;

unsigned int temperatura(void);

unsigned char tempH;
unsigned char tempL;
unsigned char temp;
 int temperatura=0;
 int pow=128;

char buf[8]; //konwersja temperatury
int temperatura
#endif
