#include "onewire.h"



char ow_reset(void)	        // Inicjalizacja 1-wire
{
	OWPORT|= (1<<DQ);		//Stan wysoki na wyjsciu DQ
	OWDDR|= (1<<DQ);		//  Ustawiamy port o nr DQ jako wyjœcie
	OWPORT&= ~(1<<DQ);		// stan niski na wyjsciu danych DQ
	_delay_us(480);			// 480-960
	OWDDR &= ~(1<<DQ);		// port o nr DQ jako wejœcie
	OWPORT|= (1<<DQ);		//Stan wysoki na wyjsciu DQ
	_delay_us(60);          // 15-60 
	if((OWPIN & (1<<DQ))==0) //sprawdzenie obecnosci czujnika
		blad=1;
	return blad;

}
	
char ow_read_bit(void)	//odczyt 1 bitu
{
	cli();    // zablokowanie przerwañ
	OWDDR|= (1<<DQ);		// DQ jako wyjœcie
	OWPORT|= (1<<DQ);      //stan wysoki na DQ
	_delay_us(2);
	OWPORT&= ~(1<<DQ);		//Stan niski na DQ aby wymusic zmiane na 1
	_delay_us(10);			
	OWPORT|= (1<<DQ);      //stan wysoki na DQ
	OWDDR&= ~(1<<DQ);      //DQ jako wejœcie
	_delay_us(15);
	if(OWPIN|= 1<<DQ)//odczyt stanu linii danych -sprawdzenie wartosci linii po 27us od rozpoczecia
		return 1;
	else
		return 0;
	sei();                  //odblokowanie przerwañ 
	
  }
  
 unsigned char ow_read_bajt(void) //podczyt ca³ego bajtu - 8bitów
 {
	unsigned char i;
	unsigned char byte=0;
	
	for(i=0;i<8;i++)  
	{
		if(ow_read_bit())  
		byte=1<<i; // przepisuje wartosci 1 do kolejnych miejsc w bicie
		_delay_us(30);
	}
	return byte;
  }
 
 void ow_write_bit(unsigned char bit) //wyslanie bitu
 {
	cli(); //zablokowanie przerwan 
	OWPORT|= (1<<DQ);      //stan wysoki na DQ
	OWDDR|= (1<<DQ); // port DQ jako wyjscie
	OWPORT&= ~(1<<DQ); //DQ stan niski - w celu informacji o rozpoczeciu nadawania
	_delay_us(10); //1-15 us
	if(bit==1) 
	OWPORT|=(1<<DQ); //jesli bit==1 to linia danych na 1 i nadaje 1
	_delay_us(100);
	OWPORT|=(1<<DQ);  //powrot do stanu wejsciowego, aby byla mozliwosc nadania kolejnego bitu
	sei(); // zgoda na przerwania
 }
 
 void ow_write_bajt(unsigned char byte) //wyslanie bajtu
 {
	unsigned char i;
	unsigned char temp;
	for(i=0;i<8;i++)
	{
		temp = byte>>i; //przypisanie do pom o bit zmiennej bajt przesuniety o i miejsc  (zeby byl najmlodszy bit)
		temp &= 0x01; // pozostawienie najmlodszego bitu
		ow_write_bit(temp); //wyslanie kolejnych bitów 
	}

	_delay_us(8);
 }	


 unsigned int temperatura(void)									//ta funkcja ma mi zwracac temperature ja juz ja sobie wyswietle!!!!
 {																// i pamietaj ze temperatura jest zapisana w kodzie U2 wiec trzeba ja przeliczyc

	 //unsigned char ulamek=0;
	 
	 obecnosc=ow_reset; // sprawdzenie obecnosci wyswietlacza
	 if(obecnosc==1) // gdy jest wykryty czujnik
	 {
     ow_write_bajt(0x44); //uruchomienie konwersji temperatury
	 OWPORT|=(1<<PULLUP);  //tryb parasite
	 _delay_us(800); //konwersja
	 ow_reset();
	 OWPORT&=~(1<<PULLUP);
	 tempH=0x00;
	 tempL=0x00;
	 ow_write_bajt(0xCC); //pomijamy ROM
	 ow_write_bajt(0xbe); //odczyt temperatury
	 tempL=ow_read_bajt(); // w naszym przypadku zawsze 0x00 
	 tempH=ow_read_bajt();
	 
	
	  for (int i=0;i<7; i++) //przelicza binarny na dziesietny
   {pow/=2;
    if(i==8-2)  //-2 bo ostatnia odpowiada za ulamki
      temperatura+=buf[i]-'0';
    else
       temperatura+=(pow)*(buf[i]-'0');
    
  }

	 return temperatura
	 }

	 else //nie ma czujnika
	 {
			lcd_locate(0,0); //w adresie 0.0 wyswietlacza wyswietla ponizszy komunikat
			lcd_str("Brak czujnika temperatury");
	}
 }

