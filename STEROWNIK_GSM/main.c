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

/*** definicje indeksow tresci wiadomosci sms ***/

#define SMS_RST		0										//powiadomienie o uruchomieniu sterownika
#define SMS_SET		1										//naglowek sms ustawiajacego wyjscia
#define SMS_GET		2										//naglowek sms pobierajacego nastawy
#define SMS_CRI		3										//naglowek sms zmieniajacego wartosi krytyczne
#define SMS_MOV 	4										//powiadomienie o wykryciu ruchu
#define SMS_OK		5										//powiadomienie o poprawnym wykonaniu komendy
#define SMS_ERR		6										//powiadomienie o blednym bledzie
#define SMS_TEMP	7
#define SMS_HUM		8
#define SMS_GAS		9
#define SMS_MTEMP	10
#define SMS_MHUM	11
#define SMS_MGAS	12

/*** definicje napisow dla wyswietlacza ***/

#define	CLK_POS_Y	1										//pozycja zegara na wyswietlaczu - wiersz
#define	CLK_POS_X	0										//pozycja zegara na wyswietlaczu - kolumna

#define DATE_POS_Y 	0										//pozycja daty na wyswietlaczu - wiersz
#define DATE_POS_X 	0										//pozycja daty na wyswietlaczu - kolumna

/*** definicje indeksow komend AT i odpowiedzi SIM900D***/

#define CHGM_A		0
#define CHG2NM_A	1
#define RDY_A		2
#define CFUN1_A		3
#define CPINR_A		4
#define CALLR_A		5
#define ATE0_C		6
#define ATE0_A		7
#define OK_A		8
#define ERROR		9
#define CREG_C		10
#define CREG_A		11
#define CSQ_C		12
#define CBC_C		13
#define CCLK_C		14
#define CCLK_Q		15
#define CMGF1_C		16
#define CMGS_C		17
#define CMT_A		18


volatile char 		UART_RxBuf[UART_BUF_SIZE];				//utworzenie bufora nadawczego
volatile char 		UART_TxBuf[UART_BUF_SIZE];				//utworzenie bufora odbiorczego

volatile uint8_t 	tx_head;								//poczatek bufora cyklicznego nadawczego
volatile uint8_t	tx_tail;								//koniec bufora cyklicznego odbiorczego
volatile uint8_t	rx_head;								//poczatek bufora cyklicznego nadawczego
volatile uint8_t	rx_tail;								//koniec bufora cyklicznego odbiorczego
volatile uint8_t	licz;

volatile uint8_t	time;									//licznik podstawy czasu
volatile uint8_t    errors;									//zmienna obslugujaca bledy
volatile uint8_t	ster;									//zmienna z flagami sterujacymi programem

uint8_t 			signal;									//moc sygnalu RF
uint8_t				batt;									//procent naladowania baterii
uint8_t				temp=25;								//aktualna temperatura
uint8_t				hum=50;									//aktualna wilgotnosc
uint8_t				gas=0;									//aktualny poziom gazow
uint8_t				max_temp=35;							//maksymalna temperatura
uint8_t				max_hum=50;								//aktualna wilgotnosc
uint8_t				max_gas=50;								//maksymalny poziom gazow
uint8_t				phase;									//procentowa wartosc mocy

char 				ans[50];
uint8_t 			users[5][12];							//numery telefonow uzytkownikow format +48xxxxxxxxx
char* 				cmd[19];								//adresy do komend zapisanych w pamieci flash
char*				disp[10];								//adresy do napisow w pamieci flash
char* 				sms[13];								//adresy do powiadomien sms w pamieci flash

char				pom;

struct
{
	uint8_t year,month,day,hour,min,sec;					//struktura zawieraj¹ca date i godzine
}date;

uint8_t str_cmp(char* str);
void send_cmd(char* str);
uint8_t send_sms(char* str, uint8_t mem, uint8_t dest);
uint8_t read_date(void);
uint8_t get_sms(void);
uint8_t sms_analyze(uint8_t dest);


int main(void)
{
	/*** napisy ***/

	sms[SMS_RST]	= PSTR("System Start/");				//powiadomoenie o uruchomieniu sterownika
	sms[SMS_SET]	= PSTR("SET: ");						//poczatek sms ustawiajacego wyjscia
	sms[SMS_GET]	= PSTR("GET;");							//tersc sms pobrania przez uzytkownika aktualnych parametrow z czujnikow i nastaw wyjsc
	sms[SMS_CRI]	= PSTR("CRI;");							//naglowek powiadomienia
	sms[SMS_MOV]	= PSTR("Movement/");					//powiadomienie o wykryciu ruchu
	sms[SMS_OK]		= PSTR("OK/");							//powiadomienie o poprawnym wykonaniu komendy
	sms[SMS_ERR]	= PSTR("ERR/");							//powiadomienie o bledzie
	sms[SMS_TEMP]	= PSTR("TEMP: /");
	sms[SMS_HUM]	= PSTR("HUM: /");
	sms[SMS_GAS]	= PSTR("GAS: /");
	sms[SMS_MTEMP]	= PSTR("MAX TEMP: /");
	sms[SMS_MHUM]	= PSTR("MAX HUM: /");
	sms[SMS_MGAS]	= PSTR("MAX GAS: /");

	/*** komendy AT i odpowiedzi SIM900D ***/

	cmd[CHGM_A]  = PSTR("CHARGE-ONLY MODE");				//utworzenie poprawnych odpowiedzi modulu po wlaczeniu zasilania
	cmd[CHG2NM_A]= PSTR("From CHARGE-ONLY MODE to NORMAL");	//							//
	cmd[RDY_A]   = PSTR("RDY");								//
	cmd[CFUN1_A] = PSTR("+CFUN: 1");						//
	cmd[CPINR_A] = PSTR("+CPIN: READY");					//
	cmd[CALLR_A] = PSTR("Call Ready");						//
	cmd[ATE0_C]  = PSTR("ATE0 ");							//wylaczenie echa (po kazdym restarcie modulu)
	cmd[ATE0_A]  = PSTR("ATE0");							//wylaczenie echa (po kazdym restarcie modulu)
	cmd[OK_A]    = PSTR("OK");								//potwierdzenie przyjecia poprawnej komendy
	cmd[ERROR]   = PSTR("ERROR");							//zwrocenie bledu
	cmd[CREG_C]	 = PSTR("AT+CREG? ");						//komenda sprawdzajaca status zalogowania do sieci
	cmd[CREG_A]	 = PSTR("+CREG: 0,1");						//odpowiedz na powyzsza komende jesli zalogowany do sieci
	cmd[CSQ_C]	 = PSTR("AT+CSQ ");							//sprawdzenie poziomu sygnalu
	cmd[CBC_C]	 = PSTR("AT+CBC ");							//sprawdzenie stanu baterii
	cmd[CCLK_C]	 = PSTR("AT+CCLK=");						//wpisanie daty i godziny
	cmd[CCLK_Q]	 = PSTR("AT+CCLK? ");						//sprawdzenie daty i godziny
	cmd[CMGF1_C] = PSTR("AT+CMGF=1 ");						//ustawienie trybu tekstowego sms
	cmd[CMGS_C]	 = PSTR("AT+CMGS=\"");						//wyslanie sms bez zapisu w pamieci sim "+48nr"
	cmd[CMT_A]	 = PSTR("+CMT: \"");							//odebranie sms
	/*** domyslny uzytkownik ***/

	/*users[0][0]='+';
	users[0][1]='4';
	users[0][2]='8';
	users[0][3]='6';
	users[0][4]='9';
	users[0][5]='4';
	users[0][6]='9';
	users[0][7]='8';
	users[0][8]='8';
	users[0][9]='0';
	users[0][10]='2';
	users[0][11]='8';*/

	users[1][0]='+';
	users[1][1]='4';
	users[1][2]='8';
	users[1][3]='7';
	users[1][4]='2';
	users[1][5]='1';
	users[1][6]='1';
	users[1][7]='5';
	users[1][8]='6';
	users[1][9]='3';
	users[1][10]='0';
	users[1][11]='2';



	/*** inicjalizacja wyjsc/wejsc ***/

	DDRA|=1<<LCD_BL;										//wyprowadzenie podswietlenia wyswietlacza jako wyjscie
	PORTA|=1<<LCD_BL;										//zalaczenie podswietlenia wyswietlacza

	DDRB|=1<<PWRK;											//wyprowadzenie zalaczania modulu jako wyjscie

	DDRC|=1<<ERR|1<<BUZZ|1<<TR5;							//wyprowadzenia diody, buzzera, triaka jako wyjscia
	PORTC|=1<<ERR/*|1<<BUZZ*/;									//zalaczenie diody i buzzera

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
	SPI_SEND=0;												//wyslanie zer na wyjscia

	/*** inicjalizacja TIMER 1 ***/

	/*** inicjalizacja TIMER 0- podstawa czasu 20ms***/
	TCCR0|=(1<<WGM01);										//tryb CTC
	TCCR0|=(1<<CS00)|(1<<CS02);								//preskaler 1024
	OCR0=216;												//czestotliwosc 20ms dla zegara 11059200Hz


	/*** inicjalizacja wyswietlacza ***/
	lcd_init();
	lcd_cls();
	lcd_str("Domowy sterownik");
	lcd_locate(1,6);
	lcd_str("GSM");

	DDRB|=1<<PB1;

	/*** wlaczenie modulu ***/

	PORTB |= 1<<PWRK;									//sekwencja zalaczenia modulu
	_delay_ms(800);
	PORTB &= ~(1<<PWRK);
	_delay_ms(200);										//oczekiwanie na uruchomienie uart
	sei();
	//while(!str_cmp(cmd[CHGM_A]));						//oczekiwanie na CHARGE MODE	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//while(!str_cmp(cmd[CHG2NM_A]));						//oczekiwanie na From CHARGE MODE to NORMAL
	while(!str_cmp(cmd[RDY_A]));						//oczekiwanie na RDY
	while(!str_cmp(cmd[CFUN1_A]));						//+CFUN: 1
	while(!str_cmp(cmd[CPINR_A]));						//+CPIN: READY
	while(!str_cmp(cmd[CALLR_A]));						//Call Ready
	lcd_cls();
	lcd_str("SIM900D: OK");

	send_cmd(cmd[ATE0_C]);								//ATE0 - wylaczenie echa
	rx_tail = (rx_tail-2) & UART_MASK;					//dodanie do odpowiedzi do echa konendy ATE0 /r/n
	while(!str_cmp(cmd[ATE0_A]));						//odebranie echa komendy ATE0
	while(!str_cmp(cmd[OK_A]));
	_delay_ms(500);
	lcd_locate(1,0);									//drugi wiersz wyswietlacza
	lcd_str("ECHO: OFF");
	_delay_ms(500);

	send_cmd(cmd[CREG_C]);								//CREG - pytanie o status logowania do sieci
	while(!str_cmp(cmd[CREG_A]));						//odebranie odpowiedzi jesli zalogowany
	while(!str_cmp(cmd[OK_A]));							//odebranie potwierdzenia komendy "OK"
	lcd_cls();
	lcd_str("LOG: OK ");
	_delay_ms(380);

	send_cmd(cmd[CMGF1_C]);								//wlaczenie trybu tekstowego sms
	while(!str_cmp(cmd[OK_A]));							//oczekiwanie na potwierdzenie

	send_cmd(cmd[CSQ_C]);								//wyslanie zapytania o moc sygnalu
	_delay_ms(200);										//oczekiwanie na odpowiedz modulu
	lcd_locate(1,0);
	if(UART_RxBuf[(rx_tail+17) & UART_MASK]=='O' &&
		UART_RxBuf[(rx_tail+18) & UART_MASK]=='K' 	)	//jesli odpowiedz OK
	{
		signal=(UART_RxBuf[(rx_tail+9) & UART_MASK]-48)*10;	//cyfra dziesiatek
		signal+=(UART_RxBuf[(rx_tail+10) & UART_MASK]-48);	//cyfra jednosci
		rx_tail = rx_head;									//wyczyszczenie bufora
		lcd_str("SIG: ");
		lcd_int(signal);
		lcd_str("  ");
	}

	send_cmd(cmd[CBC_C]);								//wyslanie zapytania o stan baterii
	_delay_ms(200);

	if(UART_RxBuf[(rx_tail+22) & UART_MASK]=='O' &&
			UART_RxBuf[(rx_tail+23) & UART_MASK]=='K' 	)	//jesli odpowiedz OK
	{
		batt=(UART_RxBuf[(rx_tail+11) & UART_MASK]-48)*10;	//cyfra dziesiatek
		batt+=(UART_RxBuf[(rx_tail+12) & UART_MASK]-48);	//cyfra jednosci

		lcd_cls();
		lcd_str("BAT: ");
		lcd_int(batt);
		lcd_str("% ");
		rx_tail = rx_head;									//wyczyszczenie bufora
	}
	lcd_cls();
	read_date();
	//TIMSK|=(1<<OCIE0);										//uruchomienie zegara

	//while(!send_sms(sms[SMS_RST],0));							//wyslij sms o stracie sterownika z pamieci programu

	while(1)												//petla glowna programu
	{
		pom=get_sms();
		//if(pom) PORTB|=1<<PB1;
		_delay_ms(500);


	}

}

uint8_t get_sms(void) /***OK***/
{
	uint8_t pom=1,pom1=1, dest=255;
	if(rx_head == rx_tail) return 0;
	_delay_ms(70);

	for(int i=0; i<7; i++)
	{
		if(UART_RxBuf[((rx_tail+i+3) & UART_MASK)]!=pgm_read_byte(cmd[CMT_A]+i)) return 0;		//sprawdzenie czy naglowek sms
		if(i==6) rx_tail=(rx_tail+i+4) & UART_MASK;												//jesli sms to skasuj naglowek wiadomoci
	}


	for(int i=0; i<5;i++)
	{
		if(users[i][0] == '+')
		{	for(int j=0; j<12;j++)
			{
				if(UART_RxBuf[((rx_tail+j) & UART_MASK)]!=users[i][j]) {pom=0; break;}					//sprawdzenie zgodny numer
				if(j==11)
				{
					rx_tail=(rx_tail+j+3) & UART_MASK;											//jesli tak to skasuj numer
					pom=1;
					dest=i;																		//indeks nadawcy w tablicy uzytkownikow
				}
			}
		 if(pom != 0) break;
		}
	}

		while(pom)
		{

			if(UART_RxBuf[((rx_tail+pom1) & UART_MASK)] == '"')
			{
				rx_tail=(rx_tail+pom1+25) & UART_MASK;													//skasowanie nazwy uzytkownika i daty otrzymania sms
				pom=0;
			}
			else pom1++;
		}

/*** dzialania w zaleznosci od tresci ***/

			return sms_analyze(dest);



}

uint8_t sms_analyze(uint8_t dest)
{/*** analiza tresci sms***/
	uint8_t pom=1,pom2=100,nr=0,outputs=0,m=0,set=0,get=0,crit=0,phase1=0;


	/*** jesli nadano SET ***/
	if(UART_RxBuf[(rx_tail+1) & UART_MASK] == pgm_read_byte(sms[SMS_SET]))/**** czasami zostawia ukosniki w wysylanej tablicy,***/
	{
		_delay_ms(50);
		rx_tail=(rx_tail+1) & UART_MASK;
		for(int i=1; i<5; i++)
		{
			if(UART_RxBuf[((rx_tail+i) & UART_MASK)] != pgm_read_byte(sms[SMS_SET]+i)) {pom=0; break;}
			if(i == 4) rx_tail=(rx_tail+i) & UART_MASK;													//skasowanie naglowka
		}
		while(pom)
		{
			if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == 'T')
			{
				pom++;
				if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == 'R')
				{
					pom++;
					nr=UART_RxBuf[((rx_tail+pom) & UART_MASK)]-'1';										//pobranie numeru bitu w bajcie ustawien
					if(nr<4)
					{
						pom++;
						if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == '=')
						{
							pom++;
							if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == '1') outputs|=1<<nr;			//
							else if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == '0')  outputs&=~(1<<nr);
							pom++;
							if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == ';')
								{
									if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == ' ')
										rx_tail=(rx_tail+pom+3) & UART_MASK;
									else
									{
										rx_tail=(rx_tail+pom+2) & UART_MASK;									//skasowanie tresci
										pom=0;
									}
								}
							else pom++;
						}
					}
					else if(nr==4)
					{

						pom++;
						if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == '=')
						{

							pom++;
							while(UART_RxBuf[((rx_tail+pom) & UART_MASK)] != '%')
							{

								phase1+=(UART_RxBuf[((rx_tail+pom) & UART_MASK)]-'0')*pom2;				//przemyslec obliczenia
								pom2=pom2/10;
								pom++;
							}

							phase=phase1;
							pom++;
							if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == ';')
							{
								if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == ' ')
									rx_tail=(rx_tail+pom+3) & UART_MASK;
								else
								{
									rx_tail=(rx_tail+pom+2) & UART_MASK;									//skasowanie tresci
									pom=0;

								}
							}
							else pom++;
						}

					}

				}
			}

			if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == 'R')
			{
				pom++;
				if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == 'E')
				{
					pom++;
					nr=UART_RxBuf[((rx_tail+pom) & UART_MASK)]-'1';										//pobranie numeru bitu w bajcie ustawien
					if(nr<4)
					{
						pom++;
						if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == '=')
						{
							pom++;
							if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == ';')
							{
								if(UART_RxBuf[((rx_tail+pom) & UART_MASK)] == ' ')
									rx_tail=(rx_tail+pom+3) & UART_MASK;
								else
								{
									rx_tail=(rx_tail+pom+2) & UART_MASK;									//skasowanie tresci
									pom=0;
								}
							}
							else pom++;
						}
					}
				}
			}
		}
				lcd_cls();
				lcd_int(outputs);
				lcd_char(' ');
				lcd_int(phase);
				lcd_char(' ');
				lcd_int(rx_tail);
				lcd_char(' ');
				lcd_int(rx_head);
				_delay_ms(1000);
		SPI_SEND=outputs; 																				//zapisanie wyjsc'
		rx_tail=rx_head;																				//roznica indeksow o 1 ??
		for(int i=0;;i++)
		{
			m=i;
			if(pgm_read_byte(sms[SMS_OK]+i) =='/') break;
			ans[i]= pgm_read_byte(sms[SMS_OK]+i);														//wpisanie potwierdzenia wykonania komendy

		}
		set=1;
	}



	/*** jesli odebrano get ***/
	if(UART_RxBuf[((rx_tail+1) & UART_MASK)] == pgm_read_byte(sms[SMS_GET]))
	{

		pom=1;
		rx_tail=(rx_tail+1) & UART_MASK;
		for(int i=1; i<4; i++)
		{
			if(UART_RxBuf[((rx_tail+i) & UART_MASK)] != pgm_read_byte(sms[SMS_GET]+i)) {pom=0; break;}
			if(i == 3) rx_tail=(rx_tail+i) & UART_MASK;												//skasowanie naglowka
			if(UART_RxBuf[(rx_tail+1) & UART_MASK] ==' ' ) rx_tail=(rx_tail+1) & UART_MASK;			//jesli dodatkowa spacja
			if(UART_RxBuf[(rx_tail+1) & UART_MASK] ==0x0D) rx_tail=(rx_tail+2) & UART_MASK;			//jesli znak konca wiadomosci
		}


		if(pom)
		{
			if(ans[0] == 'O')
			{
				if(ans[2] !='.' )
				{
					ans[2]='.';
					if(m==1)m++;
				}

			}

			for(int i=0;;i++)
			{
				if(pgm_read_byte(sms[SMS_TEMP]+i) =='/')
				{
					m=i+m;
					break;
				}
				ans[i+m]= pgm_read_byte(sms[SMS_TEMP]+i);												//wpisanie potwierdzenia wykonania komendy

			}

				ans[m]=(temp/10)+'0';
				m++;
				ans[m]=(temp%10)+'0';
				m++;
				ans[m]='*';
				m++;
				ans[m]='C';
				m++;
				ans[m]=',';
				m++;

			for(int i=0;;i++)
			{
				if(pgm_read_byte(sms[SMS_HUM]+i) =='/')
				{
					m=i+m;
					break;
				}
				ans[i+m]= pgm_read_byte(sms[SMS_HUM]+i);												//wpisanie potwierdzenia wykonania komendy

			}
			ans[m]=(hum/10)+'0';
			m++;
			ans[m]=(hum%10)+'0';
			m++;
			ans[m]='%';
			m++;
			ans[m]=',';
			m++;

			for(int i=0;;i++)
			{
				if(pgm_read_byte(sms[SMS_GAS]+i) =='/')
				{
					m=i+m;
					break;
				}
				ans[i+m]= pgm_read_byte(sms[SMS_GAS]+i);												//wpisanie potwierdzenia wykonania komendy

			}

			ans[m]=(gas/10)+'0';
			m++;
			ans[m]=(gas%10)+'0';
			m++;
			ans[m]='%';
			m++;
			ans[m]=';';
			m++;

			get=1;
		}

	}

	/*** jesli odebrano CRI; ***/
		if(UART_RxBuf[((rx_tail+1) & UART_MASK)] == pgm_read_byte(sms[SMS_CRI]))
		{
			pom=1;
			rx_tail=(rx_tail+1) & UART_MASK;
			for(int i=1; i<4; i++)
			{
				if(UART_RxBuf[((rx_tail+i) & UART_MASK)] != pgm_read_byte(sms[SMS_CRI]+i)) {pom=0; break;}
				if(i == 3) rx_tail=(rx_tail+i) & UART_MASK;													//skasowanie naglowka
				if(UART_RxBuf[(rx_tail+1) & UART_MASK] ==' ' ) rx_tail=(rx_tail+1) & UART_MASK;			//jesli dodatkowa spacja
				if(UART_RxBuf[(rx_tail+1) & UART_MASK] ==0x0D) rx_tail=(rx_tail+2) & UART_MASK;			//jesli znak konca wiadomosci
			}

			if(pom)
			{
				if(ans[0] == 'O')
				{
					if(ans[2] !='.' )
					{
						ans[2]='.';
						if(m==1)m++;
				}

			}

				for(int i=0;;i++)
				{
					if(pgm_read_byte(sms[SMS_MTEMP]+i) =='/')
					{
						m=i+m;
						break;
					}
					ans[i+m]= pgm_read_byte(sms[SMS_MTEMP]+i);												//wpisanie potwierdzenia wykonania komendy

				}

					ans[m]=(max_temp/10)+'0';
					m++;
					ans[m]=(max_temp%10)+'0';
					m++;
					ans[m]='*';
					m++;
					ans[m]='C';
					m++;
					ans[m]=',';
					m++;

				for(int i=0;;i++)
				{
					if(pgm_read_byte(sms[SMS_MHUM]+i) =='/')
					{
						m=i+m;
						break;
					}
					ans[i+m]= pgm_read_byte(sms[SMS_MHUM]+i);												//wpisanie potwierdzenia wykonania komendy

				}
				ans[m]=(max_hum/10)+'0';
				m++;
				ans[m]=(max_hum%10)+'0';
				m++;
				ans[m]='%';
				m++;
				ans[m]=',';
				m++;

				for(int i=0;;i++)
				{
					if(pgm_read_byte(sms[SMS_MGAS]+i) =='/')
					{
						m=i+m;
						break;
					}
					ans[i+m]= pgm_read_byte(sms[SMS_MGAS]+i);												//wpisanie potwierdzenia wykonania komendy

				}

				ans[m]=(max_gas/10)+'0';
				m++;
				ans[m]=(max_gas%10)+'0';
				m++;
				ans[m]='%';
				m++;
				ans[m]=';';
				m++;

				crit=1;
			}

		}

	if(set || get || crit)
	{

		ans[m]='/';

		if(send_sms(ans,1,dest))
		return 1;
	}

	return 0;
}

uint8_t read_date(void)
{
	/*** odczyt daty i godziny, wypisanie na wyswietlacz ***/
		send_cmd(cmd[CCLK_Q]);								//wyslanie zapytania o date i godzine
		_delay_ms(200);
		if(UART_RxBuf[(rx_tail+36) & UART_MASK]=='O' &&
			UART_RxBuf[(rx_tail+37) & UART_MASK]=='K' 	)	//jesli odpowiedz OK
		{
			date.year =(UART_RxBuf[(rx_tail+11) & UART_MASK]-48)*10;	//cyfra dziesiatek roku
			date.year=(UART_RxBuf[(rx_tail+12) & UART_MASK]-48);		//cyfra jednosci roku
			date.month =(UART_RxBuf[(rx_tail+14) & UART_MASK]-48)*10;	//cyfra dziesiatek miesiaca
			date.month=(UART_RxBuf[(rx_tail+15) & UART_MASK]-48);		//cyfra jednosci miesiaca
			date.day =(UART_RxBuf[(rx_tail+17) & UART_MASK]-48)*10;		//cyfra dziesiatek dnia
			date.day=(UART_RxBuf[(rx_tail+18) & UART_MASK]-48);			//cyfra jednosci dnia
			date.hour=(UART_RxBuf[(rx_tail+20) & UART_MASK]-48)*10;		//cyfra jednosci godzin
			date.hour=(UART_RxBuf[(rx_tail+21) & UART_MASK]-48);		//cyfra jednosci godzin
			date.min=(UART_RxBuf[(rx_tail+23) & UART_MASK]-48)*10;		//cyfra jednosci minut
			date.min=(UART_RxBuf[(rx_tail+24) & UART_MASK]-48);			//cyfra jednosci minut
			date.sec=(UART_RxBuf[(rx_tail+26) & UART_MASK]-48)*10;		//cyfra jednosci sekund
			date.sec=(UART_RxBuf[(rx_tail+27) & UART_MASK]-48);			//cyfra jednosci sekund


			rx_tail = rx_head;								//wyczyszczenie bufora
			lcd_locate(DATE_POS_Y,DATE_POS_X);;
			if(date.day<10) lcd_int(0);
			lcd_int(date.day);
			lcd_str("/");
			if(date.month<10) lcd_int(0);
			lcd_int(date.month);
			lcd_str("/");
			if(date.year<10) lcd_int(0);
			lcd_int(date.year);
			lcd_locate(CLK_POS_Y,CLK_POS_X);
			if(date.hour<10) lcd_int(0);
			lcd_int(date.hour);
			lcd_str(":");
			if(date.min<10) lcd_int(0);
			lcd_int(date.min);
			lcd_str(":");
			if(date.sec<10) lcd_int(0);
			lcd_int(date.sec);
		}
		return 1;
}

uint8_t send_sms(char* str, uint8_t mem, uint8_t dest)/***zmienic zeby nie rozsylal do wszystkich!!!!!!!! wysyla rozne dlugosci znakow GET krzaczy po wyslaniu do jednego odbiorcy do reszty wysyla czesc komunikatu***/
{ /*** wyslanie sms do wszystkich uzytkownikow zdefiniowanych w konfiguracji ***/

	uint8_t	spom = 0, k=0;													//zmienna pomocnicza

	if(dest<5) k=dest;													//jesli wybrano wiadomosc do jednego uzytkownika

	for(; k<5; k++)
	{
		if(users[k][0] == '+')
		{

				for(int i=0;i<9; i++)									//wpisanie komendy AT+CMGS=" do bufora nadawczego
				{
					tx_head = (tx_head+1) & UART_MASK;
					UART_TxBuf[tx_head]=pgm_read_byte(cmd[CMGS_C]+i);
				}

				for(int i=0;i<12; i++)									//wpisanie numeru
				{
					tx_head = (tx_head+1) & UART_MASK;					//zwiekszenie indeksu bufora nadawczego
					UART_TxBuf[tx_head]=users[k][i];					//wpisanie kolejnych cyfr numeru do bufora nadawczego
				}

				tx_head = (tx_head+1) & UART_MASK;						//zwiekszenie indeksu bufora nadawczego
				UART_TxBuf[tx_head]='"';								//znak cudzyslowu
				tx_head = (tx_head+1) & UART_MASK;						//zwiekszenie indeksu bufora nadawczego
				UART_TxBuf[tx_head]=0x0D;								//znak \r
				tx_head = (tx_head+1) & UART_MASK;						//zwiekszenie indeksu bufora nadawczego
				UART_TxBuf[tx_head]=0x0A;								//znak \n
				UART_SEND;												//wyslanie numeru

				while(tx_tail!=tx_head);

				while(1)
				{
					if(UART_RxBuf[(rx_tail+3) & UART_MASK]=='>' &&		//sprawdzenie czy mozna wyslac tresc wiadomosci
							UART_RxBuf[(rx_tail+4) & UART_MASK]==' ')
					{
						rx_tail=rx_head;								//oproznienie bufora odbiorczego
						break;
					}

				}

				while(UART_TxBuf[((tx_head+1) & UART_MASK)] != '/')			//jesli kolejny element
				{

					tx_head = (tx_head+1) & UART_MASK;						//zwiekszenie indeksu bufora nadawczego
					if(mem==0)												//jesli napis w pamieci programu
					{
						UART_TxBuf[tx_head] = pgm_read_byte(str+spom);		//przepisanie komunikatu do bufora nadawczego
					}
					else if(mem == 1)										//jesli napis w pamieci ram
					{
						UART_TxBuf[tx_head] = str[spom];					//przepisanie komunikatu do bufora nadawczego
					}
					spom++;
				}

				spom=0;
				tx_head = (tx_head+1) & UART_MASK;						//zwiekszenie indeksu bufora nadawczego
				UART_TxBuf[tx_head] = 0x1A;								//znak konca wiadomosci
				UART_SEND												//wyslanie komunikatu

				while(!((UART_RxBuf[(rx_tail+15) & UART_MASK]=='O' &&	//w zaleznosci jaki nr sms w pamieci
						UART_RxBuf[(rx_tail+16) & UART_MASK]=='K')||
						(UART_RxBuf[(rx_tail+16) & UART_MASK]=='O' &&
						UART_RxBuf[(rx_tail+17) & UART_MASK]=='K')||
						(UART_RxBuf[(rx_tail+17) & UART_MASK]=='O' &&
						UART_RxBuf[(rx_tail+18) & UART_MASK]=='K')));
				_delay_ms(60);
				rx_tail=rx_head;										//oproznienie bufora odbiorczego

		}
		if(dest<5) return 1;											// jesli wiadomosc tylko do jednego odbiorcy to konec funkcji
	}

	return 1;
}

void send_cmd(char* str)
{ /***wysylanie komendy AT do modulu***/
	uint8_t tmp=0;

	while(1)
	{

		tx_head=(tx_head+1) & UART_MASK;												//wyznaczenie nowego indeksu poczatku bufora nadawczego
		UART_TxBuf[tx_head] = pgm_read_byte(str+tmp);									//przepisanie kolejnego znaku do bufora wyjsciowego

			if(pgm_read_byte(str+tmp+1) == ' ')											//jesli znak konca komendy
			{
				tx_head=(tx_head+1) & UART_MASK;
				UART_TxBuf[tx_head]=0x0D;												//znak cr zakonczenie komendy dla modulu
				tx_head=(tx_head+1) & UART_MASK;
				UART_TxBuf[tx_head]=0x0A;
				UART_SEND;																//wyslij dane
				break;																	//zakoncz petle wysylajaca
			}
			tmp++;
	}
}

uint8_t str_cmp(char* str)
{ /***porownanie dopowiedzi modulu z wzorcem***/

	uint8_t	crx_head=rx_head;																		//kopie do pracy funkcji
	uint8_t tmp=0;
	uint8_t	tmp_tail=(rx_tail+3) & UART_MASK;


		if(UART_RxBuf[((rx_tail+1) & UART_MASK)] == 0x0D && 										//jesli pierwsze znaki sa rowne /r/n porownaj tekst
			UART_RxBuf[((rx_tail+2) & UART_MASK)] == 0x0A &&										//moze byc problem z +2
			rx_tail != crx_head)
		{

			do
			{
				if(UART_RxBuf[tmp_tail] == 0x0D) 												//jesli dotarlismy do konca komendy/odpowiedzi zakoncz funkcje
				{
					rx_tail=(tmp_tail+1) & UART_MASK;										//wykasowanie odczytanej komendy
					return 1;
				}
				else if(UART_RxBuf[tmp_tail] != pgm_read_byte(str+tmp)) 						//jesli ktorys element jest rozny od wzorca zwroc blad
				{
					return 0;
				}

				tmp++;																			//zwiekszenie wskaznika znaku flash
				tmp_tail=(tmp_tail+1) & UART_MASK;												//zwiekszenie indeksu bufora cyklicznego
			}while(tmp);
		}

		else
		{
			return 0;																			//bledna komenda
		}

		return 0;
}

ISR(TIMER0_COMP_vect)
{
	time++;
	if(time == 50) 								//jesli minela sekunda to zwieksz zmienna sekund
	{
		time = 0;
		date.sec++;
		if(date.sec == 60)
		{
			date.sec = 0;
			date.min++;
			if(date.min == 60)
			{
				date.min = 0;
				date.hour++;
				if(date.hour == 24)
				{
					date.hour = 0;
					read_date();

				}
				lcd_locate(CLK_POS_Y,CLK_POS_X);
				if(date.hour<10) lcd_int(0);
				lcd_int(date.hour);
				lcd_str(":");

			}
			lcd_locate(CLK_POS_Y,CLK_POS_X+3);
			if(date.min<10) lcd_int(0);
			lcd_int(date.min);
			lcd_str(":");
		}
		lcd_locate(CLK_POS_Y,CLK_POS_X+6);
		if(date.sec<10) lcd_int(0);
		lcd_int(date.sec);





	}

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
	if(licz<3)
		{
			licz++;
			return;
		}
	//lcd_char(data);
	tmp_head = (rx_head+1) & UART_MASK;						//obliczenie indeksu poczatku bufora
	/*if (tmp_head == rx_tail) errors|=1<<RX_ERR;				//blad nadpisania bufora odbiorczego, ustaw flage bledu odbiornika
	else
	{*/
		rx_head=tmp_head;									//zwiekszamy indeks poczatku bufora
		UART_RxBuf[tmp_head]=data;							//przepisujemy odebrany znak do bufora odbiorczego
	//}
	//if(data == 0x0D) ster|=1<<CMD_RDY;						//jesli odebrano znak konca komendy ustaw flage sterujaca

}
