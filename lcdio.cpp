/*
#define  DB0 (2)  // H/L Data Bus Line (Same as above)
#define  DB1 (3)
#define  DB2 (4)
#define  DB3 (5)
#define  DB4 (6)
#define  DB5 (7)
#define  DB6 (8)
#define  DB7 (9)

#define  DI  (A0)  // D/I H/L H:Data L:Instruction Code
#define  RW  (A1)  // R/W H/L H:Read L:Write
#define  EN  (A2)  // H→L Enable Signal
#define  CS1 (A3)  // H Chip Select Signal for IC1
#define  CS2 (A4)  // H Chip Select Signal for IC2
#define  RES (A5)  // L Reset Signal
*/

#include "Arduino.h"
#include "lcdio.h"

static const uint8_t AVAIL_PIN_C = 0x3f;
static const uint8_t AVAIL_PIN_B = 0x03;
static const uint8_t AVAIL_PIN_D = 0xfc;

static const uint8_t EXCEPT_SIGNAL_PIN = 0x1b;


static void setPinModeOut() {
	DDRC |= AVAIL_PIN_C;
	DDRB |= AVAIL_PIN_B;
	DDRD |= AVAIL_PIN_D;
}

static void setPinModeIn() {
	DDRC |=  AVAIL_PIN_C;
	DDRB &= ~AVAIL_PIN_B;
	DDRD &= ~AVAIL_PIN_D;
}

static void clearPortRegister() {
	PORTC &= ~EXCEPT_SIGNAL_PIN;
	PORTB &= ~AVAIL_PIN_B;
	PORTD &= ~AVAIL_PIN_D;
}

static void enableSignal() {
	PORTC |=  0x04;
	PORTC &= ~0x04;
	delayMicroseconds(8);
}


static void sendRequest(uint8_t portc, uint8_t request) {
	setPinModeOut();
	clearPortRegister();

	PORTC |= portc & EXCEPT_SIGNAL_PIN;
	PORTB |= (request >> 6) & AVAIL_PIN_B;
	PORTD |= (request << 2) & AVAIL_PIN_D;

	enableSignal();
}

static uint8_t receiveReply(uint8_t portc) {
	setPinModeIn();
	clearPortRegister();

	PORTC |= portc & EXCEPT_SIGNAL_PIN;

	enableSignal();

	uint8_t receive = 0x00;
	receive |= (PINB & AVAIL_PIN_B) << 6;
	receive |= (PIND & AVAIL_PIN_D) >> 2;
	return receive;
}


static uint8_t swithIc(uint8_t ic) {
	return ic ? 0x10 : 0x08;
}

void sendCode(uint8_t code) {
	sendRequest(0x18, code);
}

void sendData(uint8_t ic, uint8_t data) {
	sendRequest(0x01 | swithIc(ic), data);
}

uint8_t receiveData(uint8_t ic) {
	return receiveReply(0x03 | swithIc(ic));
}
