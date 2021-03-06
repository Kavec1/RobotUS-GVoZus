#define F_CPU 1000000
#include <avr/io.h>
//kniznica
#include <util/delay.h>
//kniznica preruseni
#include <avr/interrupt.h>
//kniznica LEDKY
#include <YRobot_LEDDisplay.h>
//maticka kniznica
#include <math.h>

//rychlost pri rovnosti
#define VFWD 120
//rychlost pri toceni
#define VTRN 90
//spomalovanie pri toceni
#define DIVISION 5

//port ktory snimame
int readchannel;
//hodnota 
int channelvalue[3];
//adc beziace
char ADCRunning=0;

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

//nastavovanie rychlosti robota
void MotorVpred(int L, int R){
    OCR1AL=abs(R);
    OCR1BL=abs(L);
    //nastavovanie smeru motora (left)
    if (L >= 0) {
        PORTD &= ~(1<<7);
    }else{
        PORTD |= 1<<7;
    }
    //nastavovanie smeru motora (right)
    if (R >= 0) {
        PORTD &= ~(1<<6);
    }else{
        PORTD |= 1<<6;
    }
}

//maina
void main(void) {
    init();
    YRobotDisplayInit(2);
    ADCRead(0);
    while (1) {
        if ((channelvalue[0]>200) && (channelvalue[2]>200) && (channelvalue[1]>200)) {
            //vsetky
            MotorVpred(VFWD, VFWD);
        } else if (channelvalue[0]>200) {  
            //lavy
            MotorVpred(VTRN/DIVISION,VTRN);
            PORTA|=(1<<5);
            PORTA&=~(1<<6);
        } else if (channelvalue[2]>200) {
            //pravy
            MotorVpred(VTRN,VTRN/DIVISION);
            //spravovanie lediek
            PORTA|=(1<<6);
            PORTA&=~(1<<5);
            
        } else if (channelvalue[1]>200) {
            //stred
            MotorVpred(VFWD,VFWD);
            //spravovanie lediek
            PORTA|=(1<<5);
            PORTA|=(1<<6);
        } else {
            //default
            MotorVpred(VTRN,VTRN);
            //spracovanie lediek
            PORTA&=~(1<<5);
            PORTA&=~(1<<6);
        }
    }
}