/*
 * dht11.c
 *
 *  Created on: 17 lut 2015
 *      Author: Marcin
 */

#include "dht11.h"


uint8_t* dht_read(void)
{
	block[0]=0;
	block[1]=0;
	block[2]=0;
	block[3]=0;
	cli();								//zablokowanie przerwan

	DHT_DIR|=1<<DHT_DATA;				//linia danych jako wyjscie
	DHT_PORT|=1<<DHT_DATA;				//linia danych w stan wysoki
	_delay_ms(100);

	DHT_PORT&=~(1<<DHT_DATA);			//impuls inicjalizujacy odczyt danych z czujnika
	_delay_ms(18);
	DHT_PORT|=1<<DHT_DATA;				//stan wysoki
	DHT_DIR&=~(1<<DHT_DATA);			//wejscie

	_delay_us(40);
	if(DHT_PIN & (1<<DHT_DATA))
		return 0;						// jesli czujnik nie odpowiedzial zwroc blad
	_delay_us(80);

	if(!(DHT_PIN & (1<<DHT_DATA)))
			return 0;				    // jesli czujnik nie odpowiedzial zwroc blad
	_delay_us(80);

	for(int i=0; i<5;i++)
	{
		for(int j=0; j<8; j++)
		{
			while(!(DHT_PIN & (1<<DHT_DATA)));	//czekaj dopoki start transmisji bitu

			_delay_us(30);
			if(DHT_PIN & (1<<DHT_DATA))		//jesli stan linii 1 wpisz 1
			{
				block[i]|=(1<<(7-j));
				_delay_us(40);
			}
			else
			{
				block[i]&=~(1<<(7-j));			// jesli stan linii 0 wpisz 0
			}
		}
	}
	crc=block[0]+block[1]+block[2]+block[3];	//wyliczenie crc
	if(crc!=block[4])
		return 0;

	sei();								// odblokowanie przerwan

	return block;
}
