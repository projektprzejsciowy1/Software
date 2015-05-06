/*
 * dht11.c
 *
 *  Created on: 17 lut 2015
 *      Author: Marcin
 */

#include "dht11.h"


uint8_t* dht_read(void)
{
	cli();								//zablokowanie przerwan

	DATA_LO;							//impuls inicjalizujacy odczyt danych z czujnika
	_delay_ms(19);
	DATA_HI;
	_delay_us(70);
	if(DPIN & 1<<DATA)
		return 0;						// jesli czujnik nie odpowiedzial zwroc blad
	_delay_us(120);

	for(int i=0; i<5;i++)
	{
		for(int j=7; j==0; j--)
		{
			while(!(DPIN & 1<<DATA));	//czekaj dopoki start transmisji bitu
			_delay_us(40);
			if(!(DPIN & 1<<DATA))		//jesli stan linii 0 wpisz 0
			{
				block[i]&=~(1<<j);
			}
			else
			{
				block[i]|=1<<j;			// jesli stan linii 1 wpisz
				_delay_us(50);			//odczekaj do startu transmisji kolejnego bitu
			}
		}
	}
	sei();								// odblokowanie przerwan
	return block;
}
