//-----------------------------------------------------------------------------------------------//
/* Code written by Tomi and Jari

The basic premise of the code is to connect a TFT display and EEPROM memory with SPI-connection to 
Arduino Mega 2560.

The code includes different libraries for writing the text on the screen, also getting that text
from EEPROM memory, including some hard coded text. Arduino is also connected to the computer
with UART-TTL cable, and by using UART2 TX pin, the user can turn the screen into programming 
state, where you can write whatever you want, up to 100 letters. Pressing space will make the
program start from a new line, also the text is centered. Once the user presses enter, the 
text from the screen is saved into the EEPROM then it starts scrolling from down to up on the 
screen. If the micro controller is reset, it will get the text from EEPROM and start scrolling
it on the screen.	

Code-wise the program works as follows:
-The SPI and UART connections are initialized by following the data sheet information
-EEPROM is being read into an char list, and the state is put to "scrolling", which now starts
	scrolling the text which had been saved up into the memory.
-Once the user writes something, an interrupt gets triggered. The interrupt clears the
	old char list as the counter is zero, places the char written by the user into the list and
	changes the state to "programming". "Scrolling" state has an if-statement to break the scrolling
	when the state is changed.
-The programming state checks if the pressed letter was enter, space or backspace, and then changes
	the state to "stop". If enter is pressed, it copies the char list inside the EEPROM and 
	changes the state to "scrolling" again and clears the counter value.
-The stop state shows what is written inside the char list.

Most of the code from:
https://github.com/LongHairedHacker/avr-st7735																			 

Components used:
Arduino mega 2560
https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf
ST7735 TFT
https://cdn-shop.adafruit.com/datasheets/ST7735R_V0.2.pdf
EEPROM
http://ww1.microchip.com/downloads/en/devicedoc/21227e.pdf
																								 */
//-----------------------------------------------------------------------------------------------//



#define F_CPU 16000000L

#include <avr/io.h>
#include <util/delay.h>

#define BAUD 9600 // set BAUD to usual PC terminal speed
#include <util/setbaud.h>

#include <stdbool.h> // gives bools
#include <avr/interrupt.h> // interrupts
#include <string.h> // some 

// include the other libraries in MAIN
#include "./include/spi.h"
#include "./include/st7735.h"
#include "./include/st7735_gfx.h"
#include "./include/st7735_font.h"

//#include "logo_bw.h"
#include "./fonts/free_sans.h"

// empties the screen
#define EMPTY_SCREEN st7735_fill_rect(0, 0, 160, 128, ST7735_COLOR_BLACK)

// The size of EEPROM
#define MAX_EEPROM_BYTES 0x1000


// UART2 variables
// how many letters you can write on the screen
#define RX_BUFFER_SIZE 100

// the list consisting the characters which are written on the screen and to the EEPROM
uint8_t rxBuffer[RX_BUFFER_SIZE];
uint8_t rxReadPos = 0;

// three different states, named to make it a bit easier to understand
enum States{
	stop,
	scrolling,
	programming
	};

// start by scrolling the text read from EEPROM
uint8_t volatile State = scrolling;

// for counting the numbers in the list
uint8_t volatile counter = 0;


// --------------------------------------
// UART functions
// --------------------------------------
// initialize UART2
void UART2_init (void)
{
	UBRR2H = UBRRH_VALUE;              // baud rate high value (8 bits set in setbaud.h)
	UBRR2L = UBRRL_VALUE;              // low value
	
	UCSR2B |= (1<<TXEN2) |(1<<RXEN2);  // enable receiver and transmitter
	UCSR2C |= (1<<UCSZ20)|(1<<UCSZ21); // 8 bit data format, (default: 1 stop bit, no parity)
	UCSR2B |= (1<<RXCIE2); // receive complete interrupt enable
}

// UART2 valmis

int main(void) 
{
	uint8_t y;
	
	// init spi connection
    spi_init();
	
	// init UART2
	UART2_init();
	
	// init st7735 
    st7735_init();

	// set lights on the screen
	st7735_set_bl();

	// set orientation
	st7735_set_orientation(ST7735_LANDSCAPE_INV);
	// empty the screen
	st7735_fill_rect(0, 0, 160, 128, ST7735_COLOR_BLACK);
	// Welcome message
	st7735_draw_text(25, 60, "ST7735\nWelcome!", &FreeSans, 1, ST7735_COLOR_MAGENTA);
	_delay_ms(200);
	
	sei(); // enable global interrupts
	 
	 // alustetaan rxBuffer
	 for (int i=0; i<sizeof(rxBuffer); i++)
	 {
		 rxBuffer[i] = 0;
	 }
	 
	 counter = 0;
	 
	 _delay_ms(100);
	 
	 // empty rxBuffer
	 memset(rxBuffer,0,sizeof(rxBuffer));
	 
	 //toinenReadPos = 0;
	 
	 // get the string from EEPROM
	 spi_read_eeprom((uint8_t)0x00, rxBuffer, sizeof(rxBuffer));

	
	EMPTY_SCREEN;
	
	//st7735_draw_text(25, 40, "Start by \nwriting in the\nSerial port!", &FreeSans, 1, ST7735_COLOR_MAGENTA);

	while(1) {


		if (State == scrolling)
		{
			for (y = 160; y > -160; y -= 2 ) {
				
				cli(); // disable interrupts so it doesn't create a glitch on the screen
				st7735_draw_text(0, y, (char*)rxBuffer, &FreeSans, 1, ST7735_COLOR_WHITE);
				_delay_ms(50);
				st7735_draw_text(0, y, (char*)rxBuffer, &FreeSans, 1, ST7735_COLOR_BLACK);
				sei(); // enable interrupts again
				
				// if the ISR interrupts and suddenly changes the State
				if (State == programming)
				{
					// go to programming state now
					break;
				}
			}
		}// if end


		else if (State == programming)
		{
				cli(); // disable interrupts
				
				// until enter is pressed
				if (rxBuffer[counter-1] == '\0')
				{
					UCSR2B &= (~(1<<RXEN2));	// disable receiver
					UCSR2B |= (1<<RXEN2);		// enable receiver
					
					// kirjoitetaan eepromiin
					spi_write_eeprom((uint16_t)0x00, rxBuffer, sizeof(rxBuffer));
					
					// start counter from start again
					counter = 0;
					
					// go back to scrolling
					State = scrolling;
				}else if (rxBuffer[counter-1] == ' ')
				{
					// if space write '\n'
					rxBuffer[counter-1] = '\n';
				}
				// if backspace
				else if (rxBuffer[counter-1] == 0x7f)
				{
					// make the last char disappear, 2 because the interrupt already made the counter +1
					rxBuffer[counter - 2] = 0;
					// -2 because there is ++ after this
					counter -=2;
				}

				// unless it just stops again
				if (State == programming)
				{
					State = stop; 
				}
				sei(); // enable interrupts again
		} // else if end
		 
		
		else if (State == stop)
		 {
			 cli();
			 st7735_draw_text(0, 40, (char*)rxBuffer, &FreeSans, 1, ST7735_COLOR_WHITE);
			 _delay_ms(50);
			 st7735_draw_text(0, 40, (char*)rxBuffer, &FreeSans, 1, ST7735_COLOR_BLACK);
			 
			 sei();
		 }
		 
	}
}


ISR(USART2_RX_vect)
{
	// add char to the list
	if (counter == 0)
	{
		// empty rxBuffer
		memset(rxBuffer,0,sizeof(rxBuffer));
	}
	
	rxBuffer[counter] = UDR2;
	counter++;
	
	// if we have too many letters
	if (counter > RX_BUFFER_SIZE){
		counter = 0;
	}
	
	State = programming;
	
	// counter - 1 because we just added a counter++
	if (rxBuffer[counter-1] < 0x20)
	{	
		rxBuffer[counter-1] = '\0';	
	}
}