#define F_CPU 1000000
#include <avr/io.h>
//kniznica
#include <util/delay.h>
//kniznica preruseni
#include <avr/interrupt.h>
//kniznica tlaku
#include "tlak.h"


//rychlost pri rovnosti
#define VFWD 90
//rychlost pri toceni
#define VTRN 70
#define VSKVR 40
//spomalovanie pri toceni
#define DIVISION 10
#define motorovyZosilovac 25

//port ktory snimame
int readchannel;
//hodnota
int channelvalue[3];
//adc beziace
char ADCRunning=0;

uint8_t counter;
uint8_t whiteCounter;
uint8_t skrvnaSwitch;
#define skrvnaHranica 13

#define hranica 150

#define vzad -40
#define vpred 110
#define stop 0
#define otacanievzad -40
#define otacacicas 120
#define stopovacicas 250
#define dozaducas 50
#define dopreducas 300

void MotorVpred(int L, int R);

void tehla(){
	//dozadu
	MotorVpred(vzad,vzad);
	_delay_ms(dozaducas - 20);
	//stop
	MotorVpred(stop,stop);
	_delay_ms(stopovacicas);
	//dolava
	MotorVpred(otacanievzad,-otacanievzad);
	_delay_ms(otacacicas + 70);
	//stop
	MotorVpred(stop,stop);
	_delay_ms(stopovacicas);
	//dopredu
	MotorVpred(vpred,vpred);
	_delay_ms(dopreducas + 400);
	//stop
	MotorVpred(stop,stop);
	_delay_ms(stopovacicas);
	//doprava
	MotorVpred(-otacanievzad + 10,otacanievzad);
	_delay_ms(otacacicas);
	//stop
	MotorVpred(stop,stop);
	_delay_ms(stopovacicas);
	//dopredu
	MotorVpred(vpred,vpred);
	_delay_ms(dopreducas + 600);
	//stop
	MotorVpred(stop,stop);
	_delay_ms(stopovacicas);
	//doprava
	MotorVpred(-otacanievzad,otacanievzad);
	_delay_ms(otacacicas+25);
	//stop
	MotorVpred(stop,stop);
	_delay_ms(stopovacicas);
	//dopredu 
	MotorVpred(vpred / 3 * 2,vpred / 3 * 2);
	_delay_ms(dozaducas);
}




void skvrnaAdd() {
	counter++;
	if(counter > skrvnaHranica) {
		skrvnaSwitch = 1;
	} else {
		skrvnaSwitch = 0;
	}
}


//initing
void init(void)
{
	//setovanie lediek
	DDRA|=1<<5|1<<6;
	PORTA|=1<<5|1<<6;

	//motory
	DDRD = (1<<PD5)|(1<<PD6) | (1<<PD4)|(1<<PD7); //z MOTOR_Tutorial
	PORTD&=~(1<<PD5)|(1<<PD6) | (1<<PD4)|(1<<PD7);
	
	TCCR1A = (1<<COM1A1) | (1<<COM1B1) | (1<<WGM10);
	TCCR1B = (1<<CS11);

	OCR1AH = 0;
	OCR1AL = 0;
	
	OCR1BH = 0;
	OCR1BL = 0;

	//inicializacia ADC
	ADMUX|=1<<REFS0;
	ADCSRA|=1<<ADEN | 1<<ADIE | 1<<ADPS2;
	
	//inits
	counter = 0;
	
	//zapnutie preruseni
	sei();
}

//readovanie ADC
void ADCRead(int channel)
{
	//
	readchannel=channel;
	switch (channel) {
		//kanal 0
		case 0:
		ADMUX&= ~(1<<MUX0);
		ADMUX&= ~(1<<MUX1);
		ADCSRA|=1<<ADSC;
		break;
		//kanal 1
		case 1:
		ADMUX&=~(1<<MUX1);
		ADMUX|=1<<MUX0;
		ADCSRA|=1<<ADSC;
		break;
		//kanal 2
		case 2:
		ADMUX&= ~(1<<MUX0);
		ADMUX|=1<<MUX1;
		ADCSRA|=1<<ADSC;
		break;
		//zaklad
		default:
		break;
	}
}

//prerusenia
ISR(ADC_vect) {
	//spracovanie hodnnoty (low + hight)
	uint8_t ADCLow = ADCL;
	uint16_t Result = ADCH<<8 | ADCLow;
	switch (readchannel) {
	case 0:
		channelvalue[0]=Result;
		ADCRead(1);
		break;
	case 1:
		channelvalue[1]=Result;
		ADCRead(2);
		break;
	case 2:
		channelvalue[2]=Result;
		ADCRead(0);
		break;
	default:
		break;
	}
}

uint8_t abs(uint8_t value) {
	if(value >= 0) {
		return value;
		} else {
		return value * -1;
	}
}

//nastavovanie rychlosti robota
void MotorVpred(int R, int L){
	
	if (L <= 0) {
		PORTD &= ~(1<<7);
		}else{
		PORTD |= 1<<7;
	}
	//nastavovanie smeru motora (right)
	if (R <= 0) {
		PORTD &= ~(1<<6);
		}else{
		PORTD |= 1<<6;
	}
	if(255 - abs(R) < motorovyZosilovac) {
		OCR1AL=abs(R) + motorovyZosilovac;
	} else {
		OCR1AL=abs(R);		
	}
	OCR1BL=abs(L);
}

//maina
int main(void) {
	init();
	ADCRead(0);
	DDRB &= ~(1<<2);
	PORTB = 255;
	while (1) {
		if(tlak()) {
			tehla();
		}
		if ((channelvalue[0]>hranica) && (channelvalue[2]>hranica) && (channelvalue[1]>hranica)) {
				//vsetky
				PORTA&=~(1<<5);
				PORTA&=~(1<<6);
				skvrnaAdd();
				if(skrvnaSwitch) {
					MotorVpred(VSKVR,VSKVR);
				} else {
					MotorVpred(VFWD, VFWD);				
				}
		} else if (channelvalue[2]>hranica) {
			//pravy
			MotorVpred(VTRN,-VTRN/DIVISION);
			//spravovanie lediek
			PORTA|=(1<<6);
			PORTA&=~(1<<5);
			counter = 0;
		} else if (channelvalue[0]>hranica) {
			//lavy
			MotorVpred(-VTRN/DIVISION,VTRN);
			PORTA|=(1<<5);
			PORTA&=~(1<<6);
			counter = 0;
		} else if (channelvalue[1]>hranica) {
			//stred
			MotorVpred(VFWD,VFWD);
			//spravovanie lediek
			PORTA&=~(1<<5);
			PORTA&=~(1<<6);
			counter = 0;
		} else {
			MotorVpred(VFWD,VFWD);
			//spracovanie lediek
			PORTA|= (1<<5);
			PORTA|= (1<<6);
			counter = 0;
		}
	}
	return 0;
}

/*
 * GccApplication3.c
 *
 * Created: 4/22/2017 8:49:42 AM
 *  Author: Tomasko
 */ 
