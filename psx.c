#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "mapping.h"

// roughly based on my sat2ps2 code, but rewritten in C.

//might have to swap these around... check later. Don't forget to swap
//EVERYTHING.
#define PSX_ATT (PIND & (1 << 4))
#define PSX_ACK (PIND & (1 << 3))

#define ACK_DOWN() sbi (DDRD, 3)
#define ACK_UP() cbi(DDRD, 3)


//#define ACK_DOWN() cbi(PORTD, 3)
//#define ACK_UP() sbi(PORTD, 3)

/*
#define DATA_DOWN() sbi(DDRB, 4)
#define DATA_UP() cbi(DDRB, 4)
*/
#define DATA_UP() sbi(PORTB, 4)
#define DATA_DOWN() cbi(PORTB, 4)

typedef unsigned char uchar;

//the sequence of bytes the PSX will receive
//static uchar bytes[] = {0xFF, 0x41, 0x5A, 0xFF, 0xFF}; //41 or 82???
//static uchar bytes[] = {0x33, 0x82, 0x5A, 0xFF, 0xFF}; //more interesting byte 0

#define BYTE0 0xFF
#define BYTE1 0x41
#define BYTE2 0x5A


static uchar PSX0;
static uchar PSX1; //convenience stuff

//why are these _functions?
void delay_ack() {
	// somehow delay for 3us
	_delay_us(3);
}

void delay_wait() {
	//delay 10us
//	_delay_us(10);
	_delay_us(2);
	DATA_UP();
	_delay_us(8);
}

//wait for ATT to go low
//could set an interrupt, but meh

inline void wait_attL() {
	while (PSX_ATT);
	return;
}

inline void wait_attH() {
	while (!PSX_ATT);
	return;
}

inline void do_ack() {
	//call after SPI transmission has finished
	delay_wait();
	if (PSX_ATT)
		return;
	
	ACK_DOWN();
	delay_ack();
	ACK_UP();
	return;
}

uchar recv_byte() {
	uchar out = 0;
	int i;
	for (i = 8;i;i--) {
		out >>= 1;
		while (PINB & (1 << 5) && !(PSX_ATT));
		if (PSX_ATT) {
			return 0;
		}
		//clock is down, move the first bit

		while (!(PINB & (1 << 5)) && !(PSX_ATT));

		if (PSX_ATT) {
			return 0;
		}
		// read
		// SPI is LSB first, so stick it at the top and shift down
		// this might reverse values compared to AVR USI
		out |= (PINB & (1 << 3)) << 4;
	}
	return out;
}

uchar xfer_byte(uchar byte) {
	uchar out = 0;
	sbi(PORTB, 1); //DEBUG
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
		while (PINB & (1 << 5) && !(PSX_ATT));
		if (PSX_ATT) {
			cbi(PORTB, 1); //DEBUG
			return 0;
		}
		//clock is down, move the first bit
		if (byte & 0x1)
			DATA_UP();
		else
			DATA_DOWN();
		byte >>= 1;

		while (!(PINB & (1 << 5)) && !(PSX_ATT));

		if (PSX_ATT) {
			cbi(PORTB, 1); //DEBUG
			return 0;
		}
		// read
		// SPI is LSB first, so stick it at the top and shift down
		// this might reverse values compared to AVR USI
		out |= (PINB & (1 << 3)) << 4;
	}
	cbi(PORTB, 1); //DEBUG
	return out;
}

void psx_init() {
	//similar to the USB version
	// D0-D2, D5-D7
	// 11100111
	// maybe faster transitions?
	// stop trying to disable pull-up. The buttons stop working!
//	MCUCR |= (1 << PUD);
	PORTD = 0xE7;

	// C0-C5
	// 00111111
	PORTC = 0x3F;

	// B0, B1, B2
	PORTB = 0x7;
	
	//D3 = ACK output
//	sbi(DDRD, 3);
//	sbi(PORTD, 3);

	//B1 = debug output
	sbi(DDRB, 1); //DEBUG
}

//this function probably needs to be timed, or at least time-counted... or just
//run a timer over it.
//seems to be fairly slow?
void update_input() {
	PSX0 = PSX1 = 0xff;

#define MAP(key, byte, bit) if (key)\
	byte &= ~(1 << bit);

//	MAP(IN_SQ, PSX1, 7);
	MAP(IN_X,  PSX1, 6);
	MAP(IN_CI, PSX1, 5);
//	MAP(IN_TR, PSX1, 4);
//	MAP(IN_R1, PSX1, 3); //DEBUG because it's being used as a dbg output
//	MAP(IN_L1, PSX1, 2);
//	MAP(IN_R2, PSX1, 1);
//	MAP(IN_L2, PSX1, 0);

	MAP(IN_LT, PSX0, 7);
	MAP(IN_DN, PSX0, 6);
	MAP(IN_RT, PSX0, 5);
	MAP(IN_UP, PSX0, 4);
//	MAP(IN_ST, PSX0, 3);
//	MAP(IN_SE, PSX0, 0);
	// for DC stick set up macros for stuff
}

inline void enable_data() {
	DATA_UP();
	sbi(DDRB, 4);

}

inline void enable_ack() {
//	ACK_UP();
//	sbi(DDRD, 3);
}

inline void disable_data() {
	cbi(DDRB, 4);
	DATA_DOWN();
}

inline void disable_ack() {
//	cbi(DDRD, 3);
//	ACK_DOWN();
}


void psx_main() {
	psx_init();
	//wait for the start of the next packet
	
	uchar holdoff = 0;

	while(1) {
		//both output pins hi-Z
		//during the transition they're pulled up (mostly safe)

		disable_data();
		disable_ack();

		if (holdoff) {
			sbi(PORTB, 1);
			_delay_ms(0.8);
			holdoff = 0;
			cbi(PORTB, 1);
		}


		wait_attH();

		wait_attL();
		//there _should_ be enough time for this here. probably.

		update_input();

		//first byte is sent as 0xFF, so don't activate the output yet
		if (recv_byte() != 0x1) {
			//could be wrong value because PSX sent something wrong,
			//or because ATT went high
			if (PSX_ATT)
				continue;

#if 0
			enable_data();
			enable_ack();
			//in an abort, still respond properly

			goto abort;
#endif
		}

		if (PSX_ATT)
			continue;
		//good so far, enable the ACK line


		enable_ack();

		do_ack();
		if (PSX_ATT)
			continue;

		//all clear, enable the DATA line
		enable_data();

		if (xfer_byte(BYTE1) != 0x42) { //palindrome
			if (PSX_ATT)
				continue;
			// all lines are already active
		//	goto abort;
		}
	
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

		//continue here works
		xfer_byte(PSX0);
/*		disable_data();
		disable_ack();
		recv_byte();
*/		if (PSX_ATT)
			continue;

//		continue; //DEBUG
		//continue here doesn't work
		
		do_ack();


		if (PSX_ATT)
			continue;

		
		xfer_byte(PSX1);
		holdoff = 1;

		delay_ack();
		continue;
//		delay_wait(); //don't change MISO too quickly
//		about 6-7ms between last clock and ATT going high
//		_delay_us(3);

//		if (PSX_ATT)
			continue; //done
		//disable the output line
		
		disable_data();

		//something to try: stop the transmission here (i.e. outright
		//abort)
		//if it hasn't gone low by here ACK it
		DATA_UP();
		ACK_DOWN();
		delay_ack();
		ACK_UP();

		delay_ack();
abort:
		while (!PSX_ATT) {
	//		xfer_byte(0);
			recv_byte();
			if (PSX_ATT)
				break;
			do_ack();
		}

	}

	return;
}

