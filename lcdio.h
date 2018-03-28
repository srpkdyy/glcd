#ifndef _LCDIO_H_
#define _LCDIO_H_


void sendCode(uint8_t code);

void sendData(uint8_t ic, uint8_t data);

uint8_t receiveData(uint8_t ic);

#endif // _LCDIO_H_