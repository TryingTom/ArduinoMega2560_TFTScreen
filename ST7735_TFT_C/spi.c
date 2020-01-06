#define F_CPU 16000000L

#include "./include/spi.h"
#include <util/delay.h>

void spi_init(void) {
	
	// Set MOSI and SCK, SS/CS output, all others input, EEPROM PB4
	DDRB = (1<<PB2) | (1<<PB1) | (1<<PB0) | (1<<PB4);

	// Enable SPI, Master, set clock rate fck/4, mode 0
	SPCR = (1<<SPE) | (1<<MSTR);

	// Set SS/CS high
	PORTB |= ((1 << PB0) | (1 << PB4));
}

//Address: 16 bit [MSB 8kpl] [LSB 8kpl]
//nBytes = how many bytes

void spi_read_eeprom(uint16_t address, uint8_t *pByte, uint8_t nBytes){
	uint8_t MSBaddr8bit = (address & 0x0F00)>>8;
	uint8_t LSBaddr8bit = (address & 0x00FF);
	uint8_t ByteCounter = 0;
	
	//Pull EEPROM CS low
	spi_unset_cs_eeprom();
	spi_transfer(eepromREAD);
	
	spi_transfer(MSBaddr8bit);
	spi_transfer(LSBaddr8bit);
	
	// last byte read or end of EEPROM?
	do 
	{
		*pByte=spi_transfer(eepromNOPE);
		pByte++;
		ByteCounter++;
	} while (address+ByteCounter < 0xFFF && ByteCounter <= nBytes);
	
	//Pull EEPROM CS high
	spi_set_cs_eeprom();
}

void spi_write_eeprom(uint16_t address, uint8_t *pByte, uint8_t nBytes){
	// pByte is a pointer to the list
	// nByte is how many characters in the list
	
	// the EEPROM can only handle 8 bit byte, so the 2 byte address is cut to two
	uint8_t MSBaddr8bit = (address & 0x0F00)>>8;
	uint8_t LSBaddr8bit = (address & 0x00FF);
	
	//Pull EEPROM CS low
	spi_unset_cs_eeprom();
	// enable write operations
	spi_transfer(eepromWREN);
	
	//Pull EEPROM CS high
	spi_set_cs_eeprom();
	//PORTB |= (1 << PB4);
	//Pull EEPROM CS low
	spi_unset_cs_eeprom();

	
	// write data to memory array
	spi_transfer(eepromWrite);
	
	// transfer two bytes
	spi_transfer(MSBaddr8bit);
	spi_transfer(LSBaddr8bit);
	
	// last byte read or end of EEPROM?
	do
	{
		spi_transfer(*pByte);
		pByte++;
		address++;
		nBytes--;
		
		if (!(address & 0x001F))
		{
			//Pull EEPROM CS high
			spi_set_cs_eeprom();
			// wait 5ms
			_delay_ms(5);
			//Pull EEPROM CS low
			spi_unset_cs_eeprom();
			
			// enable write operations
			spi_transfer(eepromWREN);
			
			//Pull EEPROM CS high
			spi_set_cs_eeprom();
			//Pull EEPROM CS low
			spi_unset_cs_eeprom();
			
			MSBaddr8bit = (address & 0x0F00)>>8;
			LSBaddr8bit = (address & 0x00FF);
			
			// write data to memory array
			spi_transfer(eepromWrite);
				
			spi_transfer(MSBaddr8bit);
			spi_transfer(LSBaddr8bit);
		}
	} while (address <= 0x0FFF && nBytes);
									// nBytes higher than zero
									
									

	//Pull EEPROM CS high
	spi_set_cs_eeprom();
	
	// delay from datasheet
	_delay_ms(5);
}