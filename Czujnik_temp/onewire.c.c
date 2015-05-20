#include <onewire.h>



void ow_reset(void)	        // Inicjalizacja 1-wire
{
	OWPORT|= (1<<DQ);		//Stan wysoki na wyjsciu DQ
	OWDDR|= (1<<DQ);		//  Ustawiamy port o nr DQ jako wyjœcie
	OWPORT&= ~(1<<DQ);		// stan niski na wyjsciu danych DQ
	_ delay_us(480);			// 480-960 
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
	{	return 1;else 
		return 0;
	} 
	sei();                  //odblokowanie przerwañ 
	
  }
  
 unsigned char ow_read_byte(void) //podczyt ca³ego bajtu - 8bitów
 {
	unsigned char i;
	unsigned char bajt=0;
	
	for(i=0;i<8;i++)  
	{
		if(ow_read_bit())  
		bajt=1<<i; // przepisuje wartosci 1 do kolejnych miejsc w bicie
		_delay_us(30);
	}
	return(value);
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
 
 void ow_write_byte(unsigned char bajt) //wyslanie bajtu
 {
	unsigned char i;
	unsigned char temp;
	for(i=0;i<8;i++)
	{
		temp = bajt>>i; //przypisanie do pom o bit zmiennej bajt przesuniety o i miejsc  (zeby byl najmlodszy bit)
		temp &= 0x01; // pozostawienie najmlodszego bitu
		ow_write_bit(temp); //wyslanie kolejnych bitów 
	}

	_delay_us(8);
 }	
