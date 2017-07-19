#include<avr/io.h>
#include "UART0.h"
#include "Timer.h"


/*Macro que retorna el valor normalizado de 0 a 100, tomando en cuenta la relacion de los
valores maximos y minimos. */
#define ADC_Normalize(valor) (((valor-vMin)*100)/(vMax-vMin))
#define PHOTORESISTOR PORTA0

/*Prototipos de funciones*/
uint8_t ADC_Read(uint8_t channel);
void ADC_Ini (void);
void ADC_MinMax(uint8_t channel);
void Timer2_Set_Volume(uint8_t volume);
char numero[20];

uint8_t vMax = 0;
uint8_t vMin = 0;
uint8_t offset = 0;
uint8_t trash;

const struct note ImperialMarch[]={
	//for the sheet music see:
	//http://www.musicnotes.com/sheetmusic/mtd.asp?ppn=MN0016254
	//this is just a translation of said sheet music to frequencies / time in ms
	//used TEMPO ms for a quart note
	{a, TEMPO},
	{a, TEMPO},
	{a, TEMPO},
	{f, TEMPO*3/4},
	{cH, TEMPO*1/4},

	{a, TEMPO},
	{f, TEMPO*3/4},
	{cH, TEMPO*1/4},
	{a, TEMPO*2},
	//first bit

	{eH, TEMPO},
	{eH, TEMPO},
	{eH, TEMPO},
	{fH, TEMPO*3/4},
	{cH, TEMPO*1/4},

	{gS, TEMPO},
	{f, TEMPO*3/4},
	{cH, TEMPO*1/4},
	{a, TEMPO*2},
	//second bit...

	{aH, TEMPO},
	{a, TEMPO*3/4},
	{a, TEMPO*1/4},
	{aH, TEMPO},
	{gSH, TEMPO/2},
	{gH, TEMPO/2},

	{fSH, TEMPO*1/4},
	{fH, TEMPO*1/4},
	{fSH, TEMPO/2},
	{0,TEMPO/2},
	{aS, TEMPO/2},
	{dSH, TEMPO},
	{dH, TEMPO/2},
	{cSH, TEMPO/2},
	//start of the interesting bit

	{cH, TEMPO*1/4},
	{b, TEMPO*1/4},
	{cH, TEMPO/2},
	{0,TEMPO/2},
	{f, TEMPO*1/4},
	{gS, TEMPO},
	{f, TEMPO*3/4},
	{a, TEMPO*1/4},

	{cH, TEMPO},
	{a, TEMPO*3/4},
	{cH, TEMPO*1/4},
	{eH, TEMPO*2},
	//more interesting stuff (this doesn't quite get it right somehow)

	{aH, TEMPO},
	{a, TEMPO*3/4},
	{a, TEMPO*1/4},
	{aH, TEMPO},
	{gSH, TEMPO/2},
	{gH, TEMPO/2},

	{fSH, TEMPO*1/4},
	{fH, TEMPO*1/4},
	{fSH, TEMPO/2},
	{0,TEMPO/2},
	{aS, TEMPO/2},
	{dSH, TEMPO},
	{dH, TEMPO/2},
	{cSH, TEMPO/2},
	//repeat... repeat

	{cH, TEMPO*1/4},
	{b, TEMPO*1/4},
	{cH, TEMPO/2},
	{0,TEMPO/2},
	{f, TEMPO/2},
	{gS, TEMPO},
	{f, TEMPO*3/4},
	{cH, TEMPO*1/4},

	{a, TEMPO},
	{f, TEMPO*3/4},
	{c, TEMPO*1/4},
	{a, TEMPO*2}
	//and we're done
};

int main(void)
{
	UART0_Init();
	UART0_AutoBaudRate();	
	UART0_puts("Autobaudarate done...\n\r");
	Timer0_Ini();
	ADC_Ini();
	ADC_MinMax(PHOTORESISTOR);
	itoa(numero, vMin, 10);
	UART0_puts("El valor minimo es: ");
	UART0_puts(numero);
	UART0_puts("\n\r");
	itoa(numero, vMax, 10);
	UART0_puts("El valor máximo es: ");
	UART0_puts(numero);
	UART0_puts("\n\r");
	while(1){
		if (UART0_available()){
			if(UART0_getchar() == 'p'){
				Timer2_Play(ImperialMarch,sizeof(ImperialMarch)/sizeof(struct note));
			}
		}
		Timer2_Set_Volume(ADC_Normalize(ADC_Read(PHOTORESISTOR)));
	}
	return 0;
}

/*Está función lo que realiza es una lectura del ADC usando el canal correcto y retornando el
valor de 8 bits acorde a la aplicación (ver figura 1), compensando el desplazamiento de la
medición. */
uint8_t ADC_Read(uint8_t channel){
	uint8_t valor;
	DDRA &= ~(1<<channel);						/*Establecemos el bit como entrada*/
	ADMUX = (channel<<MUX0)|(1<<REFS0);			/*Seleccionamos el canal en los bits MUX4:0*/
	ADCSRB &= ~(1<<MUX5);						/*ultimo bit de selección de canal*/
	ADCSRA = (1<<ADEN)|(7<<ADPS0)|(1<<ADSC);	/*Habilitamos el ADC y PS:128*/
	while(!(ADCSRA&(1<<ADIF)));					/*Mientras que la conversión no termine*/
	valor = ADCL;
	trash = ADCH;
	return valor-offset;
}

/*Esta función inicializa para 8 bits de resolución y habilita el ADC del microcontrolador de
forma generica. Encontrar el desplazamiento (offset) de la medición y almacenarla.*/
void ADC_Ini (){
	ADMUX = (1<<REFS0)|(0x1F<<MUX0);			/*Canal = 0001 1111*/
	ADCSRB &= ~(1<<MUX5);						/*ultimo bit de selección de canal*/
	ADCSRA = (1<<ADEN)|(7<<ADPS0)|(1<<ADSC);	/*Habilitamos el ADC y PS:128*/
	while(!(ADCSRA&(1<<ADIF)));					/*Mientras no se complete la transmisión*/
	offset = ADCL;								/*Guardamos el offset*/
	itoa(numero, offset, 10);
	UART0_puts("El offset es: ");
	UART0_puts(numero);
	UART0_puts("\n\r");
	trash = ADCH;								/*Leemos la parte alta*/
}


/*Función que captura el rango de valores, encuestando primero por el mínimo. */
void ADC_MinMax(uint8_t channel){
	UART0_getchar();
	UART0_puts("Valor minimo\n\r");
	vMin = ADC_Read(channel);
	UART0_getchar();
	UART0_getchar();
	UART0_puts("Valor maximo\n\r");
	vMax = ADC_Read(channel);
	UART0_getchar();
	UART0_puts("Fotoresistor calibrado\n\r");
}


/*Ajusta el ancho de pulso que es producido sobre la terminal OC2B. El rango del valor de
entrada sera de 0 a 100.*/
void Timer2_Set_Volume(uint8_t volume){
	Timer2_Volume(volume);
}