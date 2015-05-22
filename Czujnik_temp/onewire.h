#ifndef onewire
#define onewire

#include <avr/io.h>
#include <util/delay.h>
#include <avr/signal.h>
#include <lcd.h>
#define DQ 	PB0
#define OWPORT PORTB
#define OWDDR DDRB
#define OWPIN PINB
#define PULLUP PB1

char blad;
unsigned char obecnosc;
int temperatura;
unsigned char tempH;
unsigned char tempH1;
unsigned char tempL;

#endif