#ifndef mUSART
#define mUSART

#define F_CPU                           (2666666UL)
#define USART0_BAUD_RATE(BAUD_RATE)     ((float)(2666666 * 64 / (16 * (float)BAUD_RATE)) + 0.5)

void USART0_init(void);
void USART0_sendChar(char c);
void dMsg(char *str);
void dIMsg(int val);

#endif