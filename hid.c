#include <stdio.h>

char usbDescriptorHidReport[] = {
 0x05, 0x01,                                    // USAGE_PAGE (Generic Desktop)
 0x09, 0x05,                                    //Usage gamepad
 0xA1, 0x01,                                    // COLLECTION (Application)
 0x15, 0x00,                                    //   LOGICAL_MINIMUM (0)
 0x25, 0x01,                                    //   LOGICAL_MAXIMUM (1) --10
 0x35, 0x00,                                    //   PHYSICAL_MINIMUM (0)
 0x45, 0x01,                                    //   PHYSICAL_MAXIMUM (1)
 0x75, 0x01,                                    //   REPORT_SIZE (1)
 0x95, 0x0d,                                    //   REPORT_COUNT (13)
 0x05, 0x09,                                    //   USAGE_PAGE (Button) --20
 0x19, 0x01,                                    //   USAGE_MINIMUM (Button 1)
 0x29, 0x0d,                                    //   USAGE_MAXIMUM (Button 13)
 0x81, 0x02,                                    //   input (Data,Var,Abs) (1x13)

 0x95, 0x03,                                    //     REPORT_COUNT (3)
 0x81, 0x01,                                    //Input something or other --30
 						// Input(Constant Array Absolute
						// No_Wrap Linear) 
						// no usage included, so padding

 0x05, 0x01,                                    // USAGE_PAGE (Generic Desktop)
 0x25, 0x07,                                    //   LOGICAL_MAXIMUM (7)
 0x46, 0x3b, 0x01,                              //physical maximum 315
 0x75, 0x04,                                    //Report size 4    --39
 0x95, 0x01,                                    //Report count 1
 0x65, 0x14,                                    //unit: English angular position
 0x09, 0x39,                                    //usage: hat switch
 0x81, 0x42,                                    // Input something or other
//                             Data Variable Absolute No_Wrap Linear (4x1)

 0x65, 0x00,                                    //Unit:none    --49
 0x95, 0x01,                                    //report count 1
 0x81, 0x01,                                    //Input something or other
// Constant Array Absolute No_Wrap Linear (constant 4x1)
//padding

 0x26, 0xff, 0x00,                              // LOGICAL_MAXIMUM (255)
 0x46, 0xff, 0x00,                      // PHYSICAL_MAXIMUM (255)  --59
 0x09, 0x30, //usage direction-X
 0x09, 0x31, //usage direction-y
 0x09, 0x32, //usage direction-z
 0x09, 0x35, //usage rotate-z
  0x75, 0x08, //report size 8
 0x95, 0x04, //report count 4
 0x81, 0x02, //input Data Variable Absolute No_Wrap Linear (8x4)
// can cut from here?
 0x06, 0x00,  0xff, //usage page unknown
 0x09, 0x20, //usage unknown (in the reserved range!)
 0x09, 0x21,
 0x09, 0x22,
 0x09, 0x23,
 0x09, 0x24,
 0x09, 0x25,
 0x09, 0x26,
 0x09, 0x27,
 0x09, 0x28,
 0x09, 0x29,
 0x09, 0x2a,
 0x09, 0x2b,  //more usage up to here
 0x95, 0x0c, //report count 12
 0x81, 0x02, //input Data Variable Absolute No_Wrap Linear (8x12)
//cut to here (14*2 + 3 = 31 bytes)
 0x0a, 0x21, 0x26, //usage unknown
 0x95, 0x08, //report count 8
 0xb1, 0x02, //feature Data Variable Absolute No_Wrap Linear
 0xc0   // end collection
 };

//new size = 0x51 bytes

//cutting from report results in (19-12) = 8 reports... fits easily.

int main() {
	FILE *f = fopen("vshg.hid", "w");
	fwrite(usbDescriptorHidReport, 1, sizeof(usbDescriptorHidReport), f);
	fclose(f);
	return 0;
}
