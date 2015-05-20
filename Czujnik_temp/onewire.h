#ifndef onewire
#define onewire

#include <avr/io.h>
#include <util/delay.h>
#include <avr/signal.h>

#define DQ 	PB0
#define OWPORT PORTB
#define OWDDR DDRB
#define OWPIN PINB
#define PULLUP PB1

char blad;
#endif