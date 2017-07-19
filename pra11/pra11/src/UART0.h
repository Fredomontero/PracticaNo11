#define BUFFER_SIZE 64
#define MOD(n) n&(BUFFER_SIZE-1)
#define IS_BUFFER_EMPTY(buffer) buffer.in_idx == buffer.out_idx
#define IS_BUFFER_FULL(buffer) buffer.in_idx == MOD(buffer.out_idx-1)

/*Prototipos de funciones*/
void UART0_Init();
uint8_t UART0_available();
uint8_t UART0_getchar(void);
void UART0_putchar(uint8_t dato);
void UART0_gets(uint8_t *str);
void UART0_puts(uint8_t *str);
void itoa(uint8_t *str, uint16_t numero, uint8_t base);
uint16_t atoi(uint8_t *str);
void UART0_AutoBaudRate(void);