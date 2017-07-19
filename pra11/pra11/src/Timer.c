#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/io.h>
#include "Timer.h"
#include "UART0.h"

/*Definir el macro que calcula los ticks en base a al parametro de frecuencia (f).*/
#define TICKS(f) 62500/f				/*16000000/256*/
 
static volatile uint8_t SecFlag, play, frecuencia;
static volatile uint16_t mSeconds, tiempo, index, size, vol;
static struct note *SongPtr;
char numero [20];

void Timer0_Ini (void){
	TCNT0 = 0;															/*Clear Timer*/
	TCCR0A = 2<<WGM00;													/*CTC*/
	TCCR0B = 3<<CS00;													/*PS:64*/
	OCR0A = 249;														/*Valor a comparar*/
	TIFR0 = 1<<OCF0A;													/*Limpiamos la bandera de interrupción*/
	TIMSK0 = 1<<OCIE0A;													/*Habilitamos la interrupción de comapración de A*/
	sei();
}

void Timer2_Freq_Gen(uint8_t ticks){
	if(ticks > 0){
		TCCR2B = (6<<CS20)|(1<<WGM22);		
		TCCR2A = (3<<WGM20)|(2<<COM2B0);		
		OCR2A = ticks-1;  
		OCR2B = OCR2A*vol/100;
		/*itoa(numero,volumen,10);
		UART0_puts(numero);
		UART0_puts("\n\r");*/
	}
	else{
		TCCR2B = 0x00;	
	}
}


ISR(TIMER0_COMPA_vect){ 
	static uint16_t mSecCnt;
	mSecCnt++; 				
	if( mSecCnt==1000 ){
		mSecCnt=0;
		SecFlag=1; 			
	}
	mSeconds++;
	if( mSeconds >= tiempo ){
		mSeconds = 0;
			if(play && index!= size){							
				play = 0;										
				Timer2_Freq_Gen(TICKS(SongPtr[index].freq));	
				tiempo = SongPtr[index].delay;					
				index++;
			}
			else {										
				play = 1;
				Timer2_Freq_Gen(0);
				tiempo = SILENCE;					
			}
	}
}

void Timer2_Play(const struct note song[], unsigned int len)
{
	SongPtr = song;														/*Asignamos la canción al apuntador*/
	size = len;															/*Asignamos la dimensión del arreglo a la variable size*/
	play = 0;															/*Activamos la bandera para que empiece a tocar*/
	index= 0;															/*Inicializamos el indice a 0*/
	tiempo = 0;															/*Inicializamos el tiempo a 0*/
	DDRH |= (1<<PH6);													/*Establecemos PB4 como salida, corresponde a OC2A*/
}


void Timer2_Volume(uint8_t direction){
	vol = direction;
	/*if(direction == 1)
		{
			if(inc+10<=80)
				inc+=10;
		}
	else
		{
			if((inc-10>=0) && (inc-10<65526))
				inc-=10;
		}*/
}