/*
 * main.c
 *
 *  Created on: 2010-03-31
 *       Autor: Miros≈Çaw Karda≈õ
 */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "LCD/lcd44780.h"
#include "przyciski.h"

//stworzone poszczegolne "ekrany" jakie beda sie przewijac na wyswietlaczu
enum{ekran_glowny,ustawienia_czasu,ustawienia_daty,
	ustawienia_wilgotnosci,ustawienia_temperatury,ustawienia_dymu,
	ustawienia_ruchu,ustawienia_godziny,ustawienia_minut,ustawienia_dnia,
	ustawienia_miesiaca,ustawienia_roku};

//zmiene ktÛre beda ustawiane na wysiwtlaczu - wartosci domysle ustawione dowolnie
uint8_t godzina = 15;
uint8_t minuta = 13;
uint8_t ruch = 0;
uint8_t wilgotnosc = 47;
uint8_t temperatura = 25;
uint8_t dym = 0;
uint8_t dzien = 5;
uint8_t miesiac = 8;
uint8_t rok = 15;
uint8_t stan_menu = ekran_glowny;

//zmienne wykorzystane do przerwania
volatile uint8_t bezczynnosc = 0;
volatile uint8_t flaga_powrotu = 0;
volatile uint8_t odliczanie = 0;

//fukcja ktora wyswietla cyfry w postaci 01, 02, ...
void pisz_int(uint8_t liczba)
{
	if(liczba < 10)
	{
		lcd_char('0');
	}
	lcd_int(liczba);
}

//przerwanie
void inline init_timer2_32ms(void)
{
	TCCR2 |= ( 1 << CS22 ) | ( 1 << CS21 ) | ( 1 << CS20 ); // preskaler 1024
	TIMSK |= ( 1 << TOIE2); // uruchomienie przerwania od przepelnienia
}

//fukcja ktora tworzy staly wyglad drugiej lini wyswietlacza, ktora bedzie wygladac tak   - jakas wartosc +
void zeruj_dol(void)
{
	lcd_locate(1,0);
	lcd_str("     -    +     ");
}

//fukcja wyswierlajaca dany parametr
void wyswietl_parametr(uint8_t liczba)
{
	lcd_locate(1,7);
	pisz_int(liczba);
}

//funkcja wyswietlajaca ekran glowny
void wyswietl_ekran_glowny(void)
{
	lcd_locate(0,0);
	lcd_str("   STEROWNIK    "); // jakis tekst powitalny do wymyslenia
	lcd_locate(1,0);
	lcd_str("      GSM       ");
}

//fuckja wyswietlajaca czas
void wyswietl_ustawienia_czasu(void)
{
	lcd_locate(0,0);
	lcd_str("   USTAW CZAS   "); //jakis tekst do wpisania
	lcd_locate(1,0);
	lcd_str("     ");
	pisz_int(godzina);
	lcd_char(':');
	pisz_int(minuta);
	lcd_str("      ");
}

//fuckja wyswietlajaca date
void wyswietl_ustawienia_daty(void)
{
	lcd_locate(0,0);
	lcd_str("   USTAW DATE   "); //jakis tekst do wpisania
	lcd_locate(1,0);
	lcd_str("     ");
	pisz_int(dzien);
	lcd_char('.');
	pisz_int(miesiac);
	lcd_char('.');
	pisz_int(rok);
	lcd_str("   ");
}

//fuckja wyswietlajaca wilgotnosc
void wyswietl_ustawienia_wilgotnosci(void)
{
	lcd_locate(0,0);
	lcd_str("USTAW WILGOTNOSC"); //jakis tekst do wpisania
	zeruj_dol();
	wyswietl_parametr(wilgotnosc);
}

//fuckja wyswietlajaca temperature
void wyswietl_ustawienia_temperatury(void)
{
	lcd_locate(0,0);
	lcd_str("USTAW TEMPERATUR"); //jakis tekst do wpisania
	zeruj_dol();
	wyswietl_parametr(temperatura);
}

//fuckja wyswietlajaca dym
void wyswietl_ustawienia_dymu(void)
{
	lcd_locate(0,0);
	lcd_str("   USTAW DYM    "); //jakis tekst do wpisania
	zeruj_dol();
	wyswietl_parametr(dym);
}

//fuckja wyswietlajaca ruch
void wyswietl_ustawienia_ruchu(void)
{
	lcd_locate(0,0);
	lcd_str("   USTAW RUCH   "); //jakis tekst do wpisania
	zeruj_dol();
	wyswietl_parametr(ruch);
}

//fuckja wyswietlajaca godzine
void wyswietl_ustawienia_godziny(void)
{
	lcd_locate(0,0);
	lcd_str("   USTAW GODZ   "); //jakis tekst do wpisania
	zeruj_dol();
	wyswietl_parametr(godzina);
}

//fuckja wyswietlajaca minute
void wyswietl_ustawienia_minuty(void)
{
	lcd_locate(0,0);
	lcd_str("   USTAW MIN.   "); //jakis tekst do wpisania
	zeruj_dol();
	wyswietl_parametr(minuta);
}

//fuckja wyswietlajaca dzien
void wyswietl_ustawienia_dnia(void)
{
	lcd_locate(0,0);
	lcd_str("   USTAW DZIEN  "); //jakis tekst do wpisania
	zeruj_dol();
	wyswietl_parametr(dzien);
}

//fuckja wyswietlajaca miesiac
void wyswietl_ustawienia_miesiaca(void)
{
	lcd_locate(0,0);
	lcd_str("   USTAW MIES   "); //jakis tekst do wpisania
	zeruj_dol();
	wyswietl_parametr(miesiac);
}

//fuckja wyswietlajaca rok
void wyswietl_ustawienia_roku(void)
{
	lcd_locate(0,0);
	lcd_str("   USTAW ROK    "); //jakis tekst do wpisania
	zeruj_dol();
	wyswietl_parametr(rok);
}
//powitanie
void powitanie(void)
{
	lcd_locate(0,0);
	lcd_str("WITAMY");
	_delay_ms(500);
	lcd_cls();
	_delay_ms(500);

	lcd_locate(0,0);
	lcd_str("WITAMY");
	_delay_ms(500);
	lcd_cls();
	_delay_ms(500);

	lcd_locate(0,0);
	lcd_str("WITAMY");
	_delay_ms(500);
	//lcd_cls();
	_delay_ms(500);
}

//funkcja ktora ustawia co dzieje sie po wcisnieciu przycisku "enter", wybiera i ustawia odpowiedni "ekran" na wyswietlaczu
void inline przycisk_enter(void)
{
	if(stan_menu == ekran_glowny)
	{
		stan_menu = ustawienia_czasu;
		wyswietl_ustawienia_czasu();
	}
	else if(stan_menu == ustawienia_czasu)
	{
		stan_menu = ustawienia_godziny;
		wyswietl_ustawienia_godziny();
	}
	else if(stan_menu == ustawienia_godziny || stan_menu == ustawienia_minut)
	{
		stan_menu = ustawienia_czasu;
		wyswietl_ustawienia_czasu();
	}
	else if(stan_menu == ustawienia_daty)
	{
		stan_menu = ustawienia_dnia;
		wyswietl_ustawienia_dnia();
	}
	else if(stan_menu == ustawienia_dnia || stan_menu == ustawienia_miesiaca || stan_menu == ustawienia_roku)
	{
		stan_menu = ustawienia_daty;
		wyswietl_ustawienia_daty();
	}
}

//funkcja ktora ustawia co dzieje sie po wcisnieciu przycisku "prawo", wybiera i ustawia odpowiedni "ekran" na wyswietlaczu
void inline przycisk_prawo(void)
{
	if(stan_menu == ustawienia_czasu)
	{
		stan_menu = ustawienia_daty;
		wyswietl_ustawienia_daty();
	}
	else if(stan_menu == ustawienia_daty)
	{
		stan_menu = ustawienia_wilgotnosci;
		wyswietl_ustawienia_wilgotnosci();
	}
	else if(stan_menu == ustawienia_wilgotnosci)
	{
		stan_menu = ustawienia_temperatury;
		wyswietl_ustawienia_temperatury();
	}
	else if(stan_menu == ustawienia_temperatury)
	{
		stan_menu = ustawienia_dymu;
		wyswietl_ustawienia_dymu();
	}
	else if(stan_menu == ustawienia_dymu)
	{
		stan_menu = ustawienia_ruchu;
		wyswietl_ustawienia_ruchu();
	}
	else if(stan_menu == ustawienia_ruchu)
	{
		stan_menu = ustawienia_czasu;
		wyswietl_ustawienia_czasu();
	}
	else if(stan_menu == ustawienia_godziny)
	{
		stan_menu = ustawienia_minut;
		wyswietl_ustawienia_minuty();
	}
	else if(stan_menu == ustawienia_minut)
	{
		stan_menu = ustawienia_godziny;
		wyswietl_ustawienia_godziny();
	}
	else if(stan_menu == ustawienia_dnia)
	{
		stan_menu = ustawienia_miesiaca;
		wyswietl_ustawienia_miesiaca();
	}
	else if(stan_menu == ustawienia_miesiaca)
	{
		stan_menu = ustawienia_roku;
		wyswietl_ustawienia_roku();
	}
	else if(stan_menu == ustawienia_roku)
	{
		stan_menu = ustawienia_dnia;
		wyswietl_ustawienia_dnia();
	}

}

//funkcja ktora ustawia co dzieje sie po wcisnieciu przycisku "lewo", wybiera i ustawia odpowiedni "ekran" na wyswietlaczu
void inline przycisk_lewo(void)
{
	if(stan_menu == ustawienia_daty)
	{
		stan_menu = ustawienia_czasu;
		wyswietl_ustawienia_czasu();
	}
	else if(stan_menu == ustawienia_wilgotnosci)
	{
		stan_menu = ustawienia_daty;
		wyswietl_ustawienia_daty();
	}
	else if(stan_menu == ustawienia_czasu)
	{
		stan_menu = ustawienia_ruchu;
		wyswietl_ustawienia_ruchu();
	}
	else if(stan_menu == ustawienia_ruchu)
	{
		stan_menu = ustawienia_dymu;
		wyswietl_ustawienia_dymu();
	}
	else if(stan_menu == ustawienia_dymu)
	{
		stan_menu = ustawienia_temperatury;
		wyswietl_ustawienia_temperatury();
	}
	else if(stan_menu == ustawienia_temperatury)
	{
		stan_menu = ustawienia_wilgotnosci;
		wyswietl_ustawienia_wilgotnosci();
	}
	else if(stan_menu == ustawienia_minut)
	{
		stan_menu = ustawienia_godziny;
		wyswietl_ustawienia_godziny();
	}
	else if(stan_menu == ustawienia_godziny)
	{
		stan_menu = ustawienia_minut;
		wyswietl_ustawienia_minuty();
	}
	else if(stan_menu == ustawienia_roku)
	{
		stan_menu = ustawienia_miesiaca;
		wyswietl_ustawienia_miesiaca();
	}
	else if(stan_menu == ustawienia_miesiaca)
	{
		stan_menu= ustawienia_dnia;
		wyswietl_ustawienia_dnia();
	}
	else if(stan_menu == ustawienia_dnia)
	{
		stan_menu = ustawienia_roku;
		wyswietl_ustawienia_roku();
	}
}

//funkcja ktora ustawia co dzieje sie po wcisnieciu przycisku "+", wybiera i ustawia odpowiedni "ekran" na wyswietlaczu
void inline przycisk_gora()
{
	if(stan_menu == ustawienia_wilgotnosci)
	{
		if(wilgotnosc < 99)
		{
			wilgotnosc++;
			wyswietl_parametr(wilgotnosc);
		}
	}
	else if(stan_menu == ustawienia_temperatury)
	{
		if(temperatura < 99)
		{
			temperatura++;
			wyswietl_parametr(temperatura);
		}
	}
	else if(stan_menu == ustawienia_dymu)
	{
		if(dym < 1)
		{
			dym++;
			wyswietl_parametr(dym);
		}
	}
	else if(stan_menu == ustawienia_ruchu)
	{
		if(ruch < 1)
		{
			ruch++;
			wyswietl_parametr(ruch);
		}
	}
	else if(stan_menu == ustawienia_godziny)
	{
		if(godzina < 23)
		{
			godzina++;
			wyswietl_parametr(godzina);
		}
	}
	else if(stan_menu == ustawienia_minut)
	{
		if(minuta < 59)
		{
			minuta++;
			wyswietl_parametr(minuta);
		}
	}
	else if(stan_menu == ustawienia_dnia)
	{
		if(dzien < 31)
		{
			dzien++;
			wyswietl_parametr(dzien);
		}
	}
	else if(stan_menu == ustawienia_miesiaca)
	{
		if(miesiac < 12)
		{
			miesiac++;
			wyswietl_parametr(miesiac);
		}
	}
	else if(stan_menu == ustawienia_roku)
	{
		if(rok < 99)
		{
			rok++;
			wyswietl_parametr(rok);
		}
	}
}

//funkcja ktora ustawia co dzieje sie po wcisnieciu przycisku "-", wybiera i ustawia odpowiedni "ekran" na wyswietlaczu
void inline przycisk_dol()
{
	if(stan_menu == ustawienia_wilgotnosci)
	{
		if(wilgotnosc > 0)
		{
			wilgotnosc--;
			wyswietl_parametr(wilgotnosc);
		}
	}
	else if(stan_menu == ustawienia_temperatury)
	{
		if(temperatura > 0)
		{
			temperatura--;
			wyswietl_parametr(temperatura);
		}
	}
	else if(stan_menu == ustawienia_dymu)
	{
		if(dym > 0)
		{
			dym--;
			wyswietl_parametr(dym);
		}
	}
	else if(stan_menu == ustawienia_ruchu)
	{
		if(ruch > 0)
		{
			ruch--;
			wyswietl_parametr(ruch);
		}
	}
	else if(stan_menu == ustawienia_godziny)
	{
		if(godzina > 0)
		{
			godzina--;
			wyswietl_parametr(godzina);
		}
	}
	else if(stan_menu == ustawienia_minut)
	{
		if(minuta > 0)
		{
			minuta--;
			wyswietl_parametr(minuta);
		}
	}
	else if(stan_menu == ustawienia_dnia)
	{
		if(dzien > 0)
		{
			dzien--;
			wyswietl_parametr(dzien);
		}
	}
	else if(stan_menu == ustawienia_miesiaca)
	{
		if(miesiac > 0)
		{
			miesiac--;
			wyswietl_parametr(miesiac);
		}
	}
	else if(stan_menu == ustawienia_roku)
	{
		if(rok > 0)
		{
			rok--;
			wyswietl_parametr(rok);
		}
	}
}

int main(void)
{
	lcd_init();
	_delay_ms(2000);
	powitanie();
	init_timer2_32ms();
	wyswietl_ekran_glowny();
	przyciski_init();
	sei(); //globalne w≥aczenie przerwan
	while(1)
	{
		if(flaga_powrotu)
		{
			flaga_powrotu = 0;
			stan_menu = ekran_glowny;
			wyswietl_ekran_glowny();
		}
		if(przycisk1_obsluga())
		{
			przycisk_lewo();
			bezczynnosc = 1;
		}
		if(przycisk2_obsluga())
		{
			przycisk_prawo();
			bezczynnosc = 1;
		}
		if(przycisk3_obsluga())
		{
			przycisk_enter();
			bezczynnosc = 1;
		}
		if(przycisk5_obsluga())
		{
			przycisk_gora();
			bezczynnosc = 1;
		}
		if(przycisk4_obsluga())
		{
			przycisk_dol();
			bezczynnosc = 1;
		}
	}
}

ISR( TIMER2_OVF_vect )
{
	uint8_t static licznik;
	if(bezczynnosc)
	{
		bezczynnosc = 0;
		odliczanie = 1;
		licznik = 0;
	}
	if(odliczanie)
	{
		licznik++;
		if(licznik == 150)
		{
			odliczanie = 0;
			licznik = 0;
			flaga_powrotu = 1;
		}
	}
}
