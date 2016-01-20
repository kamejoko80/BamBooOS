#ifndef __UART_H__
#define __UART_H__

typedef void( * CallBack_Type)(uint8_t * buffer, uint8_t len);

void UART0_PutChar(char c);
void UART0_Init(void);
void UART0_AsyncReceive(uint8_t len, uint8_t * buffer, CallBack_Type CallBack);

#endif /* __UART_H__ */
