#include <avr/io.h>
#include <avr/interrupt.h>
#include "usbdrv.h"

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
         0x40, 0x00,    //max packet size : 64 bytes
         0x0A,          //interval: 10ms 

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

#define REPORTBUFFER_SIZE 0x13

static uchar reportbuffer[REPORTBUFFER_SIZE];
static uchar response[8] = {33, 38, 0, 0, 0, 0, 0, 0};
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

void doReport() {
	// NYI
	
	//B0, pin 12 on a tn2313
/*	if (PINB & 0x1) {
		reportbuffer[0] = 0xff;
	} else {
		reportbuffer[0] = 0;
	}*/

	reportbuffer[2] = dpad[PINB & 0xF];

	if (PIND & 0x10)
		reportbuffer[1] = 0x10;
	else
		reportbuffer[1] = 0;

	if (PIND & 0x20) {
		reportbuffer[0] = 0x4;
		reportbuffer[12] = 0xFF;
	} else {
		reportbuffer[0] = 0;
		reportbuffer[12] = 0;
	}

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
//			usbMsgPtr = reportbuffer;
		/*	if (len < REPORTBUFFER_SIZE)
				return len;
		*///	return REPORTBUFFER_SIZE;
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

int main() {
	//blah blah init
	
	initReport();
	usbInit();
	sei();
	
	// to send the data buffer
	// can't send with usbSetInterrupt because it's too long
//	usbSetInterrupt(reportbuffer, REPORTBUFFER_SIZE);
	while(1) {

	//	while (!usbInterruptIsReady()) {usbPoll();};

		usbPoll();
		doReport();
		usbSetInterrupt(reportbuffer, 8); //will it continue...?
		repbuffer_idx = 8;
		// erm... ?
	}

	return 0; //or not
}


