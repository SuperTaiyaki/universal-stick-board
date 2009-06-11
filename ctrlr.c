#include <avr/io.h>
#include <avr/interrupt.h>
#include "usbdrv.h"
#include "mapping.h"

//all lifted from UPCB and modified for AVR
//commented out constants are 0 in V-USB header - probably wrong
PROGMEM char usbDescriptorDevice[] = {
        0x12, //size of this descriptor in bytes
        /*DEVICE*/1, //descriptor type = PS3USB_Device_Descriptor
        0x10, 0x01,      //USB Spec 2.0
        0x00,           //class code
        0x00,           //subclass code
        0x00,           //protocol code
        0x8,           //EPO buffer size //VSHG is 40, V-USB is 8
         0xC4, 0x10,    //vendor ID 
         0xC0, 0x82, //product ID
        0x06, 0x00, //device release number
        0x01,           //manufacturer string index
        0x02,           //product string index
        0x00,           //device serial number string index
        0x01            //# of possible configurations
};

//this shouldn't blow up, but it's a few descriptors stuck together
PROGMEM char usbDescriptorConfiguration[] = {
         0x09,                  //size of this desc in bytes
         /*CONFIGURATION*/2, //type of desc
         0x22,  0x00,           //size of total descriptor
         0x01,                  //# of interfaces
         0x01,                  //index number of this config
         0x00,                  //config string index
         0x80,                  //attributes:Default
         0x32,                  //power consumption * 2mA
        //Interface portion
         0x09,                  //size of this desc in bytes
         /*INTERFACE*/4,             //type of desc //from UPCB headers
         0x00,                  //interface number (ordinal)
         0x00,                  //alternate setting number
         0x01,                  //number of endpoints (not including EP0)
         0x03,                  //Interface class - HID
         0x00,                  //sublass 
         0x00,                  //protocol code - mouse
         0x00,                  //interface string index

         //HID portion
         0x09,                  //size of this descriptor in bytes
         /*USBHID*/0x21,                   //type of descriptor: HID
         0x11, 0x01,    //HID spec release number
         0x00,                  //country code (0x00 for not supported)
         0x01,                  //number of class descriptors
         /*USBREPORT*/0x22,                        //Report descriptor type
         0x70, 0x00,    //size of report descriptor

         //endpoint portion
         0x07,          //size of desc in bytes
         /*ENDPOINT*/5,      //type od desc
         0x81,          //endpoint address EP1 IN
         0x03,          //transfer style: interrupt
         0x08, 0x00,    //max packet size : 64 bytes
         0x04,          //interval: 4ms 

};



//from UPCB. VSHG dump.
PROGMEM char usbDescriptorHidReport[] = {
 0x05, 0x01,                                    // USAGE_PAGE (Generic Desktop)
 0x09, 0x05,                                    //Usage gamepad
 0xA1, 0x01,                                    // COLLECTION (Application)
 0x15, 0x00,                                    //   LOGICAL_MINIMUM (0)
 0x25, 0x01,                                    //   LOGICAL_MAXIMUM (1) --10
 0x35, 0x00,                                    //   PHYSICAL_MINIMUM (0)
 0x45, 0x01,                                     //     PHYSICAL_MAXIMUM (1)
 0x75, 0x01,                                    //   REPORT_SIZE (1)
 0x95, 0x0d,                                    //   REPORT_COUNT (13)
 0x05, 0x09,                                    //   USAGE_PAGE (Button) --20
 0x19, 0x01,                                    //   USAGE_MINIMUM (Button 1)
 0x29, 0x0d,                                    //   USAGE_MAXIMUM (Button 13)
 0x81, 0x02,                                    //   INPUT (Data,Var,Abs)
 0x95, 0x03,                                    //     REPORT_COUNT (3)
 0x81, 0x01,                                    //Input something or other --30
 0x05, 0x01,                                    // USAGE_PAGE (Generic Desktop)
 0x25, 0x07,                                    //   LOGICAL_MAXIMUM (7)
 0x46, 0x3b, 0x01,                              //physical maximum 315
 0x75, 0x04,                                    //Report size 4    --39
 0x95, 0x01,                                    //Report count 1
 0x65, 0x14,                                    //unit: English angular position
 0x09, 0x39,                                    //usage: hat switch
 0x81, 0x42,                                    // Input something or other
 0x65, 0x00,                                    //Unit:none    --49
 0x95, 0x01,                                    //report count 1
 0x81, 0x01,                                    //Input something or other
 0x26, 0xff, 0x00,                              // LOGICAL_MAXIMUM (255)
 0x46, 0xff, 0x00,                      // PHYSICAL_MAXIMUM (255)  --59
 0x09, 0x30,
 0x09,                  //--62
 0x31, 0x09,
 0x32, 0x09,
 0x35, 0x75,
 0x08, 0x95,
 0x04, 0x81,   //--72
 0x02, 0x06,
 0x00, 0xff,
 0x09, 0x20,
 0x09, 0x21,
 0x09, 0x22,   //82
 0x09, 0x23,
 0x09, 0x24,
 0x09, 0x25,
 0x09, 0x26,   //90
 0x09, 0x27,
 0x09, 0x28,
 0x09, 0x29,
 0x09, 0x2a,
 0x09, 0x2b,  //100
 0x95, 0x0c,
 0x81, 0x02,
 0x0a, 0x21,
 0x26, 0x95,
 0x08, 0xb1,
 0x02, 0xc0   //--112 == 0x70
 };
/*
const unsigned char PS3USB_String0[] = {
	4,	// bLength
	STRING,	// bDescriptorType
	0x09,	// wLANGID[0] (low byte)
	0x04	// wLANGID[0] (high byte)
};

//from VSHG
const unsigned char PS3USB_String1[] = {
	10, 3, 83, 0, 69, 0, 71, 0, 65, 0 
	}; 
//From VSHG
const unsigned char PS3USB_String2[] = {
	48, 3, 86, 0, 73, 0, 82, 0, 84, 0, 85, 0, //12
	65, 0, 32, 0, 83, 0, 84, 0, 73, 0, 67, 0, //24
	75, 0, 32, 0, 72, 0, 105, 0, 103, 0, 104, 0, //36
	32, 0, 71, 0, 114, 0, 97, 0, 100, 0, 101, 0 }; //48
*/
#define REPORTBUFFER_SIZE 0x13

static uchar reportbuffer[REPORTBUFFER_SIZE];
static uchar response[8] = {33, 38,0,0,0,0,0,0};
static int repbuffer_idx;

// map the 4 dpad bits into a hat switch
// map as on a standard JLF connector... UDLR will do
// 0xF shouldn't ever be hit
// ... this is all backwards because the bits are flipped. urgh.
static const char dpad[] = {
	      // UDLR
	0xF,  // 0000
	0xF,  // 0001
	0xF,  // 0010
	0xF,  // 0011
	0xF,  // 0100
	0x7,  // 0101
	0x1,  // 0110
	0x0,  // 0111
	0xF,  // 1000
	0x5,  // 1001
	0x3,  // 1010
	0x4,  // 1011
	0xF,  // 1100
	0x6,  // 1101
	0x2,  // 1110
	0x8   // 1111 (neutral)
};

void initReport() {
	int i;
	for (i = 0;i < REPORTBUFFER_SIZE;i++)
		reportbuffer[i] = 0;

	//set analogs to neutral
	reportbuffer[3] = reportbuffer[4] = reportbuffer[5] = reportbuffer[6] = 0x80;
}


#define SET(reg, bit) asm("sbr reg, (bit)")

void doReport() {
	initReport();
	// could be optimized by skipping the analog bits
	// NYI
	
	//B0, pin 12 on a tn2313
/*	if (PINB & 0x1) {
		reportbuffer[0] = 0xff;
	} else {
		reportbuffer[0] = 0;
	}*/

	// on the atmega88 the dpad is spread around a bit
	// D5 D6 D7 B0
/*	uchar dirs = PIND;
	dirs <<= 5;
	// would be faster with bst/bld
	dirs |= (PINB & 0x1) ? 8 : 0;
*/
	uchar dirs=0;
	dirs |= PINB & 0x1;
	dirs <<= 1;

	dirs |= (PIND & 0x80) ? 1 : 0;
	dirs <<= 1;
	dirs |= (PIND & 0x40) ? 1 : 0;
	dirs <<= 1;
	dirs |= (PIND & 0x20) ? 1 : 0;

	reportbuffer[2] = dpad[dirs];

#define MAP(key, bit, block) \
	if (key) { \
		reportbuffer[0] |= (1 << bit); \
		reportbuffer[block] = 0xFF; \
	}

	//so much neater than the asm alternative
	MAP(IN_SQ, 0, 14);
	MAP(IN_X,  1, 13);
	MAP(IN_CI, 2, 12);
	MAP(IN_TR, 3, 11);
	MAP(IN_L1, 4, 15);
	MAP(IN_R1, 5, 16);
	MAP(IN_L2, 6, 17);
	MAP(IN_R2, 7, 18);

#undef MAP

	if (IN_ST)
		reportbuffer[1] |= 2;
	if (IN_SE)
		reportbuffer[1] |= 1;

#ifndef PS_MACRO
	if (IN_PS)
		reportbuffer[1] |= 0x10;
#else
	//start + select
	if (reportbuffer[1] == 3)
		reportbuffer[1] = 0x10;
#endif

	return;
}

void ioInit() {
	//set pull-ups on all the input pins
	//inputs are pretty horrible (noisy) without this
	// D0-D2, D5-D7
	// 11100111
	PORTD = 0xE7;

	// C0-C5
	// 00111111
	PORTC = 0x3F;

	// B0, B1, B2
	PORTB = 0x7;

	//status leds
/*	sbi(PORTB, 1);
	sbi(PORTB, 2);
*/
	cbi(PORTB, 1);
	cbi(PORTB, 2);
	return;
}

uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (usbRequest_t*)data;
	static uchar idleRate;
	static uchar protocol;

	//only vendor and class requests should be coming in here
	//other stuff is handled in usbdrv.c
	//
	//borrowed from PS2USB. Probably needs a rewrite.

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
		if (rq->bRequest == USBRQ_HID_GET_REPORT) {

			// UPCB does it this way
			usbMsgPtr = response;
			return 8;
		} else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
			usbMsgPtr = &idleRate;
			return 1;
		} else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
			idleRate = rq->wValue.bytes[1];
		} else if (rq->bRequest == USBRQ_HID_GET_PROTOCOL) {
			//protocol doesn't matter for non-boot devices, but meh
			usbMsgPtr = &protocol;
			return 1;
		} else if (rq->bRequest == USBRQ_HID_SET_PROTOCOL) {
			protocol = rq->wValue.bytes[1];
			return 0;
		}
	}

	//umm... dunno
	//if this triggers a transfer return USB_NO_MSG
	// also set repbuffer_idx to 0 to reset the transfer
	return 0;
}

uchar usbFunctionRead(uchar *data, uchar len) {
	
	if (len + repbuffer_idx > REPORTBUFFER_SIZE)
		len = REPORTBUFFER_SIZE - repbuffer_idx;

	int i;
	for (i = 0;i < len;i++) {
		data[i] = reportbuffer[repbuffer_idx + i];
	}

	repbuffer_idx += len;

	return len;
}

void usb_main() {
	//blah blah init
	
	initReport();
	ioInit();
	usbInit();
	sei();
	
	// to send the data buffer
	// can't send with usbSetInterrupt because it's too long
//	usbSetInterrupt(reportbuffer, REPORTBUFFER_SIZE);
	while(1) {
		usbPoll();
		while (!usbInterruptIsReady()) {usbPoll();};
		doReport();
		usbSetInterrupt(reportbuffer, 8); //will it continue...?
		repbuffer_idx = 8;
		while (!usbInterruptIsReady()) {usbPoll();};
		usbSetInterrupt(reportbuffer + 8, 8);
		while (!usbInterruptIsReady()) {usbPoll();};
		usbSetInterrupt(reportbuffer + 16, 3);
	}

	return; //or not
}

//shouldn't really go here, but it's not likely to change
void psx_main();

int main() {
	// TODO: decide between usb_main and psx_main
	// maybe wait to see if PSX ATT line goes low, if it doesn't assume USB?
	// pull up CLK, wait 16ms, if it hasn't gone down then assume it's not
	// PSX
	// pull up clock
	
	//B1 is power LED
	//B2 is USB status
/*	
	sbi(PORTB, 1);
	sbi(PORTB, 2);
*/
	sbi(DDRB, 1);
	sbi(DDRB, 2);

	uchar psx = 0;
	sbi(PORTB, 5);
	//going to stall for 1 frame = 16ms = 256k clocks
	//timer1 with maximum prescaler (1024) is 250 cycles
	//TCCR0A = 11000000 (set OC0A on match)
	TCCR0A = 0xC0;
	//TCCR0B = 00000101 (system clock / 1024)
	TCCR0B = 0x05;
	OCR0A = 250;
	while(!(TIFR0 & (1 << OCF0A))) {
		if (!(PINB & (1 << 5))) {
			psx = 1;
			cbi(DDRB, 2);
			break;
		}
	}
//	psx =0 ;
	TCCR0B = 0; //stop the counter
	PORTB = 0;
	if (psx)
		psx_main();
	else
		usb_main();
	return 0; // ...
}
