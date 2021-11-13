#include "mUSART.h"

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>


void USART0_init(void)
{
    // CLKCTRL_MCLKCTRLA = 0; //use 16mhz internal
    // CLKCTRL_MCLKCTRLB = 7; //prescalar of 0
    PORTA.DIRSET = PIN0_bm;
    PORTA.DIRCLR = PIN1_bm;
    
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(115200);

    USART0.CTRLB |= USART_TXEN_bm; 
    USART0.CTRLC = USART_CHSIZE0_bm
                 | USART_CHSIZE1_bm; 
}

void USART0_sendChar(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
    {
        ;
    }        
    USART0.TXDATAL = c;
}

void dMsg(char *str)
{
    for(size_t i = 0; i < strlen(str); i++)
    {
        USART0_sendChar(str[i]);
    }
    USART0_sendChar('\r');
    USART0_sendChar('\n');


}