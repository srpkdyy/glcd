#include "Arduino.h"
#include "lcdio.h"
#include "images.h"
#include "font.h"
#include <avr/pgmspace.h>


#define SWAP(a,b) ((a)^=(b)^=(a)^=(b))

static const uint8_t OP_WRITE  = 0;
static const uint8_t OP_ERASE  = 1;
static const uint8_t OP_INVERT = 2;

static const uint8_t DATA_SIZE = 8;



void turnOnDisplay() {
	sendCode(0x3f);
}

void turnOffDisplay() {
	sendCode(0x3e);
}

void selectStartLine(uint8_t line) {
	line &= 0x3f;           // Display start line (0~63)
	sendCode(line | 0xc0); // convert to instruction code format.
}



static void selectPage(uint8_t page) {
	page &= 0x07;
	sendCode(page | 0xb8);
}

static void selectColumn(uint8_t column) {
	column &= 0x3f;
	sendCode(column | 0x40);
}



static uint8_t bitMask(uint8_t writeData, uint8_t readData, uint8_t op) {
	switch (op) {
	    case OP_WRITE:  return  writeData | readData;
	    case OP_ERASE:  return ~writeData & readData;
	    case OP_INVERT: return  writeData ^ readData;
	    case 100:       return  writeData;
	    default:        return              readData;
	}
}

//
static bool isOnLcd(uint8_t x, uint8_t y) {
	if (x >= COLUMN_NUM * IC_NUM)  return false;
	if (y >= DATA_SIZE * PAGE_NUM) return false;
	return true;
}

static uint8_t toPage(uint8_t y) {
	return y / DATA_SIZE;
}

static uint8_t toColumn(uint8_t x) {
	return x % COLUMN_NUM;
}

static uint8_t toIc(uint8_t x) {
	return x / COLUMN_NUM;
}


void clearScreen() {
	for (uint8_t ic = 0; ic < IC_NUM; ic++) {
		for (uint8_t page = 0; page < PAGE_NUM; page++) {
			selectPage(page);
			for (uint8_t column = 0; column < COLUMN_NUM; column++) {
				selectColumn(column);
				sendData(ic, 0x00);
			}
		}
	}
}

void drawImage(uint8_t index, uint8_t op) {
	for (uint8_t ic = 0; ic < IC_NUM; ic++) {
		for (uint8_t page = 0; page < PAGE_NUM; page++) {
			selectPage(page);
			for (uint8_t column = 0; column < COLUMN_NUM; column++){
				selectColumn(column);
				receiveData(ic);
				uint8_t readData = receiveData(ic);

				uint8_t writeData = pgm_read_byte(&IMAGES[index][ic][page][column]);

				selectColumn(column);
				sendData(ic, bitMask(writeData, readData, op));
			}
		}
	}
}

void drawCharacter(uint8_t x1, uint8_t y1, char ch, uint8_t op) {
	uint8_t x2 = x1 + ASCII_FONT_WIDTH -1;
	uint8_t y2 = y1 + ASCII_FONT_HEIGHT -1;
	if(!isOnLcd(x1, y1)) return;
	if(!isOnLcd(x2, y2)) return;
	
	for (uint8_t i = 0; i < ASCII_FONT_WIDTH; i++) {
		uint16_t allWriteData = pgm_read_word(&ASCII_FONT_TABLE[ch*ASCII_FONT_WIDTH + i]);

		for (uint8_t page = toPage(y1); page <= toPage(y2); page++) {
			uint8_t writeData = 0x00;
			if (page == toPage(y1)) {
				writeData = (uint8_t)allWriteData << (y1 - DATA_SIZE*page);
				allWriteData = allWriteData >> (DATA_SIZE*(page+1) - y1);
			}
			else {
				writeData = (uint8_t)allWriteData;
				allWriteData = allWriteData >> DATA_SIZE;
			}

			uint8_t x = x1 + i;

			selectPage(page);
			selectColumn(x);
			uint8_t readData;
			receiveData(toIc(x));
			readData = receiveData(toIc(x));

			selectColumn(x);
			sendData(toIc(x), bitMask(writeData, readData, op));
		}
	}

}

void drawString(uint8_t x, uint8_t y, String str, uint8_t op, uint8_t charGap) {
	for (uint8_t i = 0; i < str.length(); i++) {
		drawCharacter(x + (ASCII_FONT_WIDTH+charGap)*i, y, str[i], op);
	}
}


//
void drawHorzLine(uint8_t x1, uint8_t y, uint8_t x2, uint8_t op) {
  if (!isOnLcd(x1, y)) return;
  if (!isOnLcd(x2, y)) return;

  if (x1 > x2) SWAP(x1, x2);

  selectPage(toPage(y));

  uint8_t writeData = 0x01 << y % DATA_SIZE;
  for(uint8_t x = x1; x <= x2; x++ ) {
  	selectColumn(toColumn(x));
  	receiveData(toIc(x));
  	uint8_t readData = receiveData(toIc(x));

  	selectColumn(toColumn(x));
    sendData(toIc(x), bitMask(writeData, readData, op));
  }

}

void drawVertLine(uint8_t x, uint8_t y1, uint8_t y2, uint8_t op) {
	if (!isOnLcd(x, y1)) return;
	if (!isOnLcd(x, y2)) return;

	if (y1 > y2) SWAP(y1, y2);

	for (uint8_t page = toPage(y1); page <= toPage(y2); page++) {
		selectPage(page);
		selectColumn(toColumn(x));
		// when page is first or last page.
		uint8_t writeData = 0xff;
		if (page == toPage(y1)) writeData = writeData << (y1 - DATA_SIZE*page);
		if (page == toPage(y2)) writeData = writeData >> (DATA_SIZE*(page+1)-1 - y2);

		receiveData(toIc(x));
		uint8_t readData = receiveData(toIc(x));

		selectColumn(toColumn(x));
		sendData(toIc(x), bitMask(writeData, readData, op));
	}
}

void drawSquare(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, uint8_t op) {
	uint8_t x2 = x1 + width -1;
	uint8_t y2 = y1 + height -1;
	
	for (uint8_t y=y1; y<=y2; y++) {
		drawHorzLine(x1, y, x2, op);
	}
}

void drawSquareFrame(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, uint8_t op) {
	uint8_t x2 = x1 + width -1;
	uint8_t y2 = y1 + height -1;

	drawHorzLine(x1, y1, x2, op);
	drawHorzLine(x1, y2, x2, op);
	drawVertLine(x1, y1+1, y2-1, op);
	drawVertLine(x2, y1+1, y2-1, op);
}
