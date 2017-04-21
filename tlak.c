#include <stdbool.h>
#include <avr/io.h>

 DDRD = 0x00;   //PORTD input
 PORTD = 0xff;
 bool tlak(){
	 if( (!(PIND & (1<<PD2)) & !(PIND & (1<<PD2))) )
	 _delay_ms(5);
	 return true;
	 
	 else
	 _delay_ms(5);
		return false;
 }
