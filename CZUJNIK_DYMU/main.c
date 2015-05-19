#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "LCD/lcd44780.h"

//definicja pinu do czujnika i skroty ulatwiajace dostep
#define czujnik_dymu_ddr DDRA
#define czujnik_dymu_port PORTA
#define czujnik_dymu_pin PINA
#define czujnik_dymu (1 << PA2)
#define czujnik_dymu_ktory 1
#define czujnik_dymu_ON (!(czujnik_dymu_pin & czujnik_dymu))
#define czujnik_dymu_OFF (czujnik_dymu_pin & czujnik_dymu)

uint16_t pomiar(uint8_t kanal)
{
	ADMUX = (ADMUX & 0xF8) | kanal;
	ADCSRA |= ( 1 << ADSC); //uruchomienie pomiaru
	while( ADCSRA & ( 1 << ADSC )); //oczekiwanie na zakonczenie
	return ADCW;
}

void init_adc(void)
{
	ADMUX |= ( 1 << REFS0); // AVV jako odniesienie
	ADCSRA |= ( 1 << ADPS2 ) | ( 1 << ADPS1 ); // presk x64
	ADCSRA |= ( 1 << ADEN ); // enable
}


uint16_t wartosc_pomiaru;

int main(void)
{
	lcd_init();
	init_adc();
	while(1)
	{
		if(czujnik_dymu_ON)
		{
			lcd_locate(0,0);
			lcd_str("WYKRYTO DYM");
		}
		else
		{
			lcd_locate(0,0);
			lcd_str("BRAK DYMU  ");
		}
		wartosc_pomiaru = pomiar(czujnik_dymu_ktory);
		lcd_locate(1,0);
		lcd_int(wartosc_pomiaru);
		lcd_str("   ");
		_delay_ms(500);
	}
}

