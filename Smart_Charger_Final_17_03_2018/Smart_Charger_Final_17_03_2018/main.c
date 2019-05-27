/*
 * Smart_Charger_with_EEPROM.c 
 *
 * Created: 17-03-2018 11:22:50
 *  Author: Ramalingeswar
 */
#define F_CPU 1000000UL														//FOR DELAY
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<stdio.h>
volatile unsigned int Hours,Count;											//INITAITING HOUR & TIMER_COUNT VALUE
void EEPROM_write(unsigned int ucAddress, unsigned int cnt_Data, unsigned int hr_data)
{
	unsigned char EEPR_data[3];
	int i;
	EEPR_data[0]=hr_data;													//CONVERTING INTEGER FORMAT TO BYTE FORMAT
	EEPR_data[1]=cnt_Data>>8;												//CONVERTING INTEGER FORMAT TO BYTE FORMAT
	EEPR_data[2]=cnt_Data;													//CONVERTING INTEGER FORMAT TO BYTE FORMAT
	for(i=0;i<=2;i++,ucAddress++)
	{
		while(EECR & (1<<EEPE));												/* Wait for completion of previous write */
		EECR = (0<<EEPM1)|(0<<EEPM0);											/* Set Programming mode */
		EEAR = ucAddress;														/* Set up address and data registers */
		EEDR = EEPR_data[i];
		EECR |= (1<<EEMPE);														/* Write logical one to EEMPE */
		EECR |= (1<<EEPE);														/* Start eeprom write by setting EEPE */
	}
}

unsigned char EEPROM_read(unsigned int ucAddress)
{
	while(EECR & (1<<EEPE));												/* Wait for completion of previous write */
	EEAR = ucAddress;														/* Set up address register */
	EECR |= (1<<EERE);														/* Start eeprom read by writing EERE */
	return EEDR;															/* Return data from data register */
}

void Call_Timer()
{
	int Previous_hour=Hours,EE_wr_time=/*57*/3446;							//NUMBER OF COUNTS (57) FOR 15 SEC & (3446) FOR 15 MIN
	TCCR0B =(1 << CS02)|(1 << CS00);										//PRESCALLING TIMER0 - 1024
	EEPROM_write(0x08,Count,Hours);											//WRITING TO EEPROM INITIALLY FOR SAFETY
	while(1)
	{
		while((TIFR0 & 0x01)==0);											//WAIT UNTILL OVERFLOW (TOV0 GOES TO ZERO)
		TIFR0=0x01;															//SETTING TOV0
		Count++;															//INCREMENTING THE COUNT FOR 1 HOUR
		if((Count==EE_wr_time)||(Count==(2*EE_wr_time))||(Count==(3*EE_wr_time)))	//FOR TESTING EVERY 15 MIN
		{
			EEPROM_write(0x08,Count,Hours);									//WRITING TO EEPROM FOR EVERY 15 MIN
		}
		if(Count>=/*229*/13786)												//FOR TESTING NUMBER OF COUNTS (229) FOR 1	MIN & (13786) FOR 1 HOUR
		{
			Count=0;														//RESETTING COUNT
			Hours--;														//DECREASIG HOUR
			if(Hours==0)
			{
				EEPROM_write(0x08,Count,Hours);								//WRITTING AS ZERO, BOTH HOURS & COUNT.
				PORTA|=0xaf;												//ALL LED'S AND P-MOS(CHARGING PIN) PIN ARE OFF CONDITION
				break;														//ROUTING TO MAIN()
			}
			break;															//ROUTING TO MAIN()
		}
		if(Hours>Previous_hour)												//BY THIS CONDITION WE ARE ROUTING TO MAIN()
		{
			Count=0;
			break;															//ROUTING TO MAIN()
		}
	}
}
ISR(EXT_INT0_vect)															// INTERRUPT SERVICE ROUTINE FOR INT0
{
	Hours++;																//NUMBER OF HOURS
}
void INTO_init()
{
	MCUCR=(1<<ISC01)|(1<<ISC00);											//FOR RAISING EDGE TRIGGERING
	GIMSK|=(1<<INT0);														//FOR ENABLING INTERRUPT 0
}
int main(void)
{
	sei();																	//SET ALL INTERRUPTS ENABLE
	DDRA |=0xef;															//PA0 &PA1 &PA2 &PA3 &PA5 &PA7 AS OUTPUT
	PORTA |=0xaf;															//INITIALLY PMOS OFF (CHARGING OFF)
	INTO_init();															//INTERRUPT-0 INITIALIZATION
	PORTA &=0x20;															//ALL LED'S ON
	_delay_ms(500);															//DELAY OF HALF SEC
	PORTA |=0xbf;															//ALL LED'S OFF
	_delay_ms(500);															//DELAY OF HALF SEC
	Hours=(int)EEPROM_read(0x08);											//CONVERTING CHAR TO INT
	Count=(((int)EEPROM_read(0x09))<<8)|((int)EEPROM_read(0x0A));			//CONVERTING CHAR TO INT
	while (1)
	{
		if(Hours>=6)
		{
			Hours=0;														//LIMMITTING TO 6 CLICKS
			EEPROM_write(0x08,Count,Hours);									//WRITTING AS ZERO, BOTH HOURS & COUNT.
			PORTA|=0xaf;													//ALL LED'S AND P-MOS(CHARGING PIN) PIN ARE OFF CONDITION
		}
		if(Count>13786)														//FOR SAFETY WHEN EEPROM FAIL CONDITION(NO NEED THIS IF CODITION)
		{
			Count=0;
		}
		switch(Hours)
		{
			case 1:
				PORTA|=0x8f;											//CLEARING LED'S
				PORTA&=0x8e;											//CHARGING ON WITH ONE LED BLINKING
				Call_Timer();											//CALLING 1 HOUR
				break;
			case 2:
				PORTA|=0x8f;											//CLEARING LED'S
				PORTA&=0x8c;											//CHARGING ON WITH TWO LED'S BLINKING
				Call_Timer();											//CALLING 1 HOUR
				break;
			case 3:
				PORTA|=0x8f;											//CLEARING LED'S
				PORTA&=0x88;											//CHARGING ON WITH THREE LED BLINKING
				Call_Timer();											//CALLING 1 HOUR
				break;
			case 4:
				PORTA|=0x8f;											//CLEARING LED'S
				PORTA&=0x80;											//CHARGING ON WITH FOUR LED'S BLINKING
				Call_Timer();											//CALLING 1 HOUR
				break;
			case 5:
				PORTA|=0x8f;											//CLEARING LED'S
				PORTA&=0x00;											//CHARGING ON WITH FIVE LED'S BLINKING
				Call_Timer();											//CALLING 1 HOUR
				break;
			default:
				break;
		}
	}
}



