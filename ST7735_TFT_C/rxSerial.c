/*
 * rxSerial.c
 *
 * Created: 19.11.2019 15.51.56
 *  Author: Tomi
 */ 

#include <avr/io.h>

// set bit to 0
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
// set bit to 1
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= ~_BV(bit))

#define F_CPU 16000000
#define BUAD 9600
#define BRC ((F_CPU/16/BUAD)-1)

#include <util/delay.h>
#include <avr/interrupt.h>

#define RX_BUFFER_SIZE 128

char rxBuffer[RX_BUFFER_SIZE];
uint8_t rxReadPos = 0;
uint8_t rxWritePos = 0;

char getChar(void);
char peekChar(void);

int main(void)
{
	// baud rate register
	UBRR0H = (BRC >> 8);
	UBRR0L = BRC;
	
	// enable receiver
	UCSR0B = (1 << RXEN0) | (1 << RXCIE0);
	
	// character size (8 bit)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
	// set as output
	DDRB = (1 << PORTB5);
	
	// enable interrupts
	sei();
	
	while (1)
	{
		char c = getChar();
		
		if (c == '1')
		{
			sbi(PORTB,PORTB5);
		}
		else if (c == '0')
		{
			cbi(PORTB,PORTB5);
		}
	}
}

char peekChar(void)
{
	char ret = '\0';
	
	if(rxReadPos != rxWritePos)
	{
		// just check what's in there, but not change it
		ret = rxBuffer[rxReadPos];
	}
	
	return ret;
}

char getChar(void)
{
	char ret = '\0';
	
	if (rxReadPos != rxWritePos)
	{
		ret = rxBuffer[rxReadPos];
		
		rxReadPos++;
		
		if (rxReadPos >= RX_BUFFER_SIZE)
		{
			rxReadPos = 0;
		}
	}
	return ret;
}

// will fire when new data in data register
ISR(USART_TX_vect)
{
	rxBuffer[rxWritePos] = UDR0;
	
	rxWritePos++;
	
	if(rxWritePos >= RX_BUFFER_SIZE)
	{
		rxWritePos = 0;
	}
}