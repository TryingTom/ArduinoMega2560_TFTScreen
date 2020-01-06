#include "./include/st7735_font.h"

#include <string.h>
#include <avr/pgmspace.h>

#include "./include/st7735.h"

#define RX_BUFFER_SIZE 100
#define LETTER_WIDTH 10

void st7735_draw_char(int16_t x, int16_t y, const GFXglyph *glyph, const GFXfont *font,
                        uint8_t size, uint16_t color) {
    if(size < 1) {
        return;
    }

    // Clipping will be done by fill rect if necessary

    uint8_t  *bitmap = font->bitmap;
    uint16_t bo = glyph->bitmapOffset;
    uint8_t bits = 0, bit = 0;
    uint16_t set_pixels = 0;

    uint8_t  cur_x, cur_y;
    for(cur_y = 0; cur_y < glyph->height; cur_y++) {
        for(cur_x = 0; cur_x < glyph->width; cur_x++) {
            if(bit == 0) {
                bits = pgm_read_byte(&bitmap[bo++]);
                bit  = 0x80;
            }

            if(bits & bit) {
                set_pixels++;
            } else if (set_pixels > 0) {
                st7735_fill_rect(x + (glyph->xOffset + cur_x-set_pixels) * size,
                                y + (glyph->yOffset+cur_y) * size,
                                size * set_pixels,
                                size,
                                color);
                set_pixels=0;
            }

            bit >>= 1;
        }

        // Draw rest of line
        if (set_pixels > 0) {
            st7735_fill_rect(x + (glyph->xOffset + cur_x-set_pixels) * size,
                            y + (glyph->yOffset + cur_y) * size,
                            size * set_pixels,
                            size,
                            color);
            set_pixels=0;
        }
    }
}



void st7735_draw_text(int8_t x, int8_t y, char *text, const GFXfont *p_font,
                        uint8_t size, uint16_t color) {

	// keskitysArray has all the sizes of different words after every space
	uint8_t keskitysArray[RX_BUFFER_SIZE];
	uint8_t counter = 0;
	uint8_t summa = 0;
	
	int16_t cursor_x = x;
    int16_t cursor_y = y;
	
	// alustetaan
	memset(keskitysArray,0,sizeof(keskitysArray));

	// set keskitysArray
	for (uint8_t i = 0; i < RX_BUFFER_SIZE; i++){
		// if space
		if (text[i] == '\n' || text[i] == '\0')
		{
			// if first index
			if (counter > 0)
			{
				// add the last string size to summa
				summa = summa + keskitysArray[counter-1];
			}
			
			// new index gets the i - summa, meaning all the current i minus all the string sizes of the list
			keskitysArray[counter] = i - summa;
			
			counter++;
		}
		// if text[i] is end
		if (text[i] == '\0')
		{
			// get i high enough that the for loop breaks
			i = RX_BUFFER_SIZE;
		}
	}
	
	// put the cursor at the first coordinates depending on the first keskitysArray spot, where the first number of characters reach space
	cursor_x = (80-(keskitysArray[0] * LETTER_WIDTH * size / 2));
	// Then raise counter to 1 
	counter = 1;
	
    GFXfont font;
    memcpy_P(&font, p_font, sizeof(GFXfont));
	
	

	// now to write everything on the string list which was sent to the function
    for(uint16_t text_pos = 0; text_pos < strlen(text); text_pos++) {
        char c = text[text_pos];
		
		// if there is a space
        if(c == '\n') {
			// same as below, but now starting from index 1
            cursor_x = (80-(keskitysArray[counter] * LETTER_WIDTH * size / 2));
			// next line on the screen
            cursor_y += font.yAdvance * size;
			counter++;
        }
        else if(c >= font.first && c <= font.last && c != '\r') {
            GFXglyph glyph;
            memcpy_P(&glyph, &font.glyph[c - font.first], sizeof(GFXglyph));

			// draw one character
            st7735_draw_char(cursor_x, cursor_y, &glyph, &font, size, color);
            cursor_x += glyph.xAdvance * size;
        }

    }

}
