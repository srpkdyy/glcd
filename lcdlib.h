#ifndef _LCDLIB_H_
#define _LCDLIB_H_

void turnOnDisplay();

void selectStartLine(uint8_t line);

void clearScreen();

void drawImage(uint8_t index, uint8_t op);

void drawCharacter(uint8_t x, uint8_t y, char character, uint8_t op);

void drawString(uint8_t x, uint8_t y, String str,  uint8_t op, uint8_t charGap = 0);

void drawHorzLine(uint8_t x1, uint8_t y, uint8_t x2, uint8_t op);

void drawVertLine(uint8_t x, uint8_t y1, uint8_t y2, uint8_t op);

void drawSquare(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t op);

void drawSquareFrame(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t op);


#endif // _LCDLIB_H_