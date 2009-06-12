#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "mapping.h"

// roughly based on my sat2ps2 code, but rewritten in C.

//might have to swap these around... check later. Don't forget to swap
//EVERYTHING.
#define PSX_ATT (PIND & (1 << 4))
#define PSX_ACK (PIND & (1 << 3))
/*
#define ACK_DOWN() sbi (DDRD, 3)
#define ACK_UP() cbi(DDRD, 3)
*/

#define ACK_DOWN() cbi(PORTD, 3)
#define ACK_UP() sbi(PORTD, 3)

/*
#define DATA_DOWN() sbi(DDRB, 4)
#define DATA_UP() cbi(DDRB, 4)
*/
#define DATA_UP() sbi(PORTB, 4)
#define DATA_DOWN() cbi(PORTB, 4)

typedef unsigned char uchar;

//the sequence of bytes the PSX will receive
static uchar bytes[] = {0xFF, 0x41, 0x5A, 0xFF, 0xFF}; //41 or 82???
//static uchar bytes[] = {0x33, 0x82, 0x5A, 0xFF, 0xFF}; //more interesting byte 0

#define BYTE0 0xFF
#define BYTE1 0x41
#define BYTE2 0x5A


static uchar *PSX0 = &bytes[3];
static uchar *PSX1 = &bytes[4]; //convenience stuff

//why are these _functions?
void delay_ack() {
	// somehow delay for 3us
	_delay_us(3);
}

void delay_wait() {
	//delay 10us
	_delay_us(10);
}

//wait for ATT to go low
//could set an interrupt, but meh

void wait_attL() {
	while (PSX_ATT);
	return;
}

void wait_attH() {
	while (!PSX_ATT);
	return;
}

inline void do_ack() {
	//call after SPI transmission has finished
//	delay_wait();
//	if (PSX_ATT)
//		return;
	_delay_us(2);
	DATA_UP();
	_delay_us(8);
	
	ACK_DOWN();
	delay_ack();
	ACK_UP();
	//using DDRB to use low/Hi-Z
	return;
}

uchar nextbit;
uchar tripped = 0;

ISR(PCINT0_vect) {

	if (nextbit)
		DATA_UP();
	else
		DATA_DOWN();
	tripped = 1;
	return;
}

// TODO: This is spitting out the wrong values. It's not reversed, but maybe
// shifted one.
uchar xfer_byte(uchar byte) {
	uchar out = 0;
	sbi(PORTB, 1);
	//simulate the USI.
	//SPI doesn't work because the SS line is being used.
	
	//clock is PB5
	//MISO is output (PB4)
	//MOSI is input (PB3)
	//at 500khz (ps2 clock) one clock period is 2e-6s.
	//one clock cycle at 16mhz is 6.3e-6s... lots of time wasting in here
	
	//clock starts out high, wait for it to drop...
	
	//transmissions in both directions are the reverse of AVR USI
	//bytes are reversed to match.
	
	int i;
	for (i = 8;i;i--) {
		out >>= 1;
		while (PINB & (1 << 5));
		if (PSX_ATT)
			return 0;
		//clock is down, move the first bit
		if (byte & 0x1)
			DATA_UP();
		else
			DATA_DOWN();
		byte >>= 1;

		while (!(PINB & (1 << 5)));

		if (PSX_ATT)
			return 0;
		// read
		// SPI is LSB first, so stick it at the top and shift down
		// this might reverse values compared to AVR USI
		out |= (PINB & (1 << 3)) << 4;
	}
	cbi(PORTB, 1);
	return out;
}

void psx_init() {
	//similar to the USB version
	// D0-D2, D5-D7
	// 11100111
	// maybe faster transitions?
//	MCUCR |= (1 << PUD);
	PORTD = 0xE7;

	// C0-C5
	// 00111111
	PORTC = 0x3F;

	// B0, B1, B2
	PORTB = 0x7;
/*
	//but a few more things
	//D3 (ACK) and B4 (MISO) are actually outputs
	//leave them hi (hi-z) on idle
	cbi(DDRD, 3);
	cbi(DDRB, 4);
	cbi(PORTD, 3);
	cbi(PORTB, 4);

	//ACK (D4) = hi-z input
	cbi(DDRD, 4);
	cbi(PORTD, 4);

	//MOSI (B3) = hi-z input
	cbi(DDRB, 3);
	cbi(PORTB, 3);
	
	
	//SCK (B5) = hi-z input
	cbi(DDRB, 5);
	cbi(PORTB, 5);
*/
//	sbi(DDRB, 4);
	
	//D3 = ACK output
	sbi(DDRD, 3);
	sbi(PORTD, 3);

	//B1 = debug output
	sbi(DDRB, 1);
}

//this function probably needs to be timed, or at least time-counted... or just
//run a timer over it.
//seems to be fairly slow?
void update_input() {
	*PSX0 = *PSX1 = 0xff;

#define MAP(key, byte, bit) if (key)\
	*byte &= ~(1 << bit);

	MAP(IN_SQ, PSX1, 7);
	MAP(IN_X,  PSX1, 6);
	MAP(IN_CI, PSX1, 5);
	MAP(IN_TR, PSX1, 4);
//	MAP(IN_R1, PSX1, 3);
	MAP(IN_L1, PSX1, 2);
	MAP(IN_R2, PSX1, 1);
	MAP(IN_L2, PSX1, 0);

	MAP(IN_LT, PSX0, 7);
	MAP(IN_DN, PSX0, 6);
	MAP(IN_RT, PSX0, 5);
	MAP(IN_UP, PSX0, 4);
	MAP(IN_ST, PSX0, 3);
	MAP(IN_SE, PSX0, 0);
	// for DC stick set up macros for stuff
}

void psx_main() {
	psx_init();
	update_input();
	//wait for the start of the next packet
	
	while(1) {
		cbi(DDRB, 4);
		wait_attH();

		wait_attL();
		sbi(DDRB, 4);
		//wake up the SPI

		update_input();
		if (xfer_byte(bytes[0]) != 0x1) {
			goto abort;
		}
//		xfer_byte(BYTE0);
		if (PSX_ATT)
			continue;
		do_ack();
		if (PSX_ATT)
			continue;
		if (xfer_byte(bytes[1]) != 0x42) { //palindrome
			goto abort;
		}
	
//		xfer_byte(BYTE1);
		if (PSX_ATT)
			continue;
		do_ack();
		if (PSX_ATT)
			continue;	
		xfer_byte(BYTE2);
		if (PSX_ATT)
			continue;
		do_ack();
		if (PSX_ATT)
			continue;	
	
		xfer_byte(bytes[3]);
//		if (PSX_ATT)
//			continue;
		do_ack();
//		if (PSX_ATT)
//			continue;

		xfer_byte(bytes[4]);
		delay_wait(); //don't change MISO too quickly

		if (PSX_ATT)
			continue; //done
		
		//if it hasn't gone low by here ACK it
		DATA_UP();
		ACK_DOWN();
		delay_ack();
		ACK_UP();

		
abort:
		while (!PSX_ATT) {
			xfer_byte(0);
			if (PSX_ATT)
				break;
			do_ack();
		}

	}

	return;
}

