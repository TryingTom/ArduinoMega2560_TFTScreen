#ifndef _SPI_H_
#define _SPI_H_

#define eepromREAD 0x03 // read data from memory array beginning at selected address
#define eepromWrite 0x02 // write data to memory array beginning at selected address
#define eepromWRDI 0x04 // reset the write enable latch (disable write opertaions)
#define eepromWREN 0x06 // set the write enable latch (enable write operations
#define eepromRDSR 0x05 // read status register
#define eepromWRSR 0x01 // write status register 
#define eepromNOPE 0x00 // no operation

//#define MSBaddr8bit(address) (address&0x0F00)>>8
//#define LSBaddr8bit(address) (address&0x00FF)


#include<avr/io.h>

void spi_init(void);

static inline uint8_t spi_transfer(uint8_t byte) {
	SPDR = byte;
	while(!(SPSR & (1<<SPIF))); // waits for flag
	return SPDR;
}

void spi_read_eeprom(uint16_t address, uint8_t *pByte, uint8_t nBytes);
void spi_write_eeprom(uint16_t address, uint8_t *pByte, uint8_t nBytes);

// chip select = slave select
static inline void spi_set_cs_tft(void) {
	PORTB |= (1 << PB0);	// Mega D53
}

static inline void spi_unset_cs_tft(void) {
	PORTB &= ~(1 << PB0);
}

// chip select for eeprom 
static inline void spi_set_cs_eeprom(void) {
	PORTB |= (1 << PB4);	
}

static inline void spi_unset_cs_eeprom(void) {
	PORTB &= ~(1 << PB4);
}	


#endif


