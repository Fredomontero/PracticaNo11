#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/io.h>

#define BUFFER_SIZE 64
#define MOD(n) n&(BUFFER_SIZE-1)
#define IS_BUFFER_EMPTY(buffer) buffer.in_idx == buffer.out_idx
#define IS_BUFFER_FULL(buffer) buffer.in_idx == MOD(buffer.out_idx-1)

/*Definimos la estructura a utilizar*/
typedef struct{
	char buffer [BUFFER_SIZE];
	volatile unsigned char in_idx:6;
	volatile unsigned char out_idx:6;
}cola_circular;

cola_circular bufferTX;
cola_circular bufferRX;


void UART0_Init(){
	UCSR0A = 1<<U2X0;							/*Doble velocidad*/
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);				/*Habilitamos receptor y transmisor*/
	UCSR0C= (3<<UCSZ00);						/*Estableecemos el formato*/
	TCCR0A = 0x00;								/*Timer 0: Modo normal*/
	TCCR0B = 2<<CS00;							/*Preescalador: 8*/
	sei();
	UCSR0B|=(1<<RXCIE0);						/*Activamos la interrupción de recepción completa*/
	
	bufferRX.in_idx = 0;
	bufferRX.out_idx = 0;
	bufferTX.in_idx = 0;
	bufferTX.out_idx = 0;
}

/*función AutoBaudRate*/
void UART0_AutoBaudRate(void){
	while(PINE&(1<<PE0));
	TCNT0 = 0;
	while(!(PINE&(1<<PE0))){
	}
	TCCR0B = 0x00;
	UBRR0 = TCNT0-1;
}

uint8_t UART0_available(){
	return (IS_BUFFER_EMPTY(bufferRX))?1:0;
}

/*Función que introduce datos a la cola de transmisión*/
void UART0_putchar(uint8_t dato){
	while(IS_BUFFER_FULL(bufferTX));
	/*Si esta vacia, mete dato y activa la interrupcion, sino solo mete dato*/
	if(IS_BUFFER_EMPTY(bufferTX)){
		bufferTX.buffer[bufferTX.in_idx++] = dato;
		UCSR0B|=(1<<UDRIE0);
	}
	else
	bufferTX.buffer[bufferTX.in_idx++] = dato;
}

/*Interrupción que saca datos de la cola de transmisión*/
ISR(USART0_UDRE_vect){
	if(IS_BUFFER_EMPTY(bufferTX))
	UCSR0B &=(~(1<<UDRIE0));
	else
	UDR0 = bufferTX.buffer[bufferTX.out_idx++];
}

/*Función que retorna el byte recibido por el puerto serie UART0*/
uint8_t UART0_getchar(void){
	while(IS_BUFFER_EMPTY(bufferRX));
	return bufferRX.buffer[bufferRX.out_idx++];
}

/*Interrupción que introduce datos a la cola de recepción*/
ISR(USART0_RX_vect){
	while(IS_BUFFER_FULL(bufferRX));
	bufferRX.buffer[bufferRX.in_idx++] = UDR0;
}

/*Función que retorna una cadena mediante UART_getchar, la cadena se retorna en el apuntador str*/
void UART0_gets(char *str){
	uint8_t c,i=0;
	do{
		c = UART0_getchar();
		UART0_putchar(c);
		if(c != 8){
			str[i++] = c;
		}
		else {
			UART0_putchar(' ');
			UART0_putchar('\b');
			if(i != 0)
			i--;
		}
	}while(c!=13);
	str[i] = '\0';
}


/*función que transmite una cadena mediante UART0_putchar*/
void UART0_puts(uint8_t *str){
	while(*str)
	UART0_putchar(*str++);
}

/*alphanumeric to Integer*/
void itoa( uint8_t *str, uint16_t number, uint8_t base){
	if(base<37){                            //verficamos que la base sea menor a 37
		char aux[16];
		int i = 0;
		if(!number)
		aux[i++]=number;
		while(number){
			aux[i++] = number%base;
			number/=base;
		}
		i--;                                  //Ajustamos el contador
		for(i; i>=0; i--){
			if(aux[i]>9)
			aux[i]+=7;
			aux[i]+=48;
			*str++ = aux[i];
		}
		*str='\0';                            //Cerramos la cadena
	}
	else
	UART0_puts("Ingrese una base menor a 37\n");
}

uint16_t atoi( uint8_t *str ){
	uint16_t res = 0;
	uint8_t i = 0;
	for(i = 0; str[i]!='\0'; ++i){
		if((str[i]>47)&&(str[i]<58))
		res = res*10+str[i]-'0';
		else
		break;
	}
	return res;
}