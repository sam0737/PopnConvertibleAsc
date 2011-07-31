/*

hColorLed - Controlling tricolor LED over USB

Copyright (c) 2008 Sam Wong http://hellosam.net/

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the Free 
Software Foundation, either version 2 of the License, or (at your option) any 
later version.

This software is provided "as is" and any express or implied warranties, 
including, but not limited to, the implied warranties of merchantibility and 
fitness for a particular purpose are disclaimed. In no event shall the 
copyright owner or contributors be liable for any direct, indirect, 
incidental, special, exemplary, or consequential damages (including, but not 
limited to, procurement of substitute goods or services; loss of use, data, 
or profits; or business interruption) however caused and on any theory of 
liability, whether in contract, strict liability, or tort (including 
negligence or otherwise) arising in any way out of the use of this software, 
even if advised of the possibility of such damage. See the GNU General Public 
License for more details.

For full license details see license.txt

Version: $Id: hcolorled.c 7 2008-04-20 15:38:00Z sam $

*/

#include "usbconfig.h"
#include "usbavrusb/usbdrv.h"
#include "button.h"

#include <avr/interrupt.h>
#include <avr/wdt.h>

unsigned char debug[8];

int main(void)
{
	// Turn on Watchdog
	wdt_enable(WDTO_500MS);
	
	// Turn off clock Prescaler
	CLKPR = (1<<CLKPCE);
	CLKPR = 0;
	
	buttonInit();
	usbInit();
	
	// Enable Interrupt
	sei();
	while (1)
	{
		wdt_reset();
		usbPoll();
		
		if (usbIsUSBReady())
		{
			buttonLoop();
			
		}
	}
	return 0;
}

// Remember to update USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH in usbconfig.h if this is changed
PROGMEM unsigned char usbHidReportDescriptor[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x1E,                    //   USAGE_MINIMUM (Keyboard 1 and !)
    0x29, 0x26,                    //   USAGE_MAXIMUM (Keyboard 9 and ()
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x09,                    //   REPORT_COUNT (9)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x75, 0x07,                    //   REPORT_SIZE (7)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0xc0                           // END_COLLECTION
	
	// There is one IN-report:
	// Byte 1: Bit status of key 1 to 8
	// Byte 2: Bit status of key 0. Followed by 7 reserved bits.
};

// Answering HID Report descriptor request
unsigned char usbUserFunctionSetup(struct usbSetupPacket *packet)
{
	if (packet->bmRequestType == 0x81)
	{
		if (packet->bRequest == USBSETUP_REQ_GET_DESCRIPTOR)
		{
			if (packet->wValue[1] == USBDESCR_HID_REPORT) 
			{
				usbSetupDataInPGM(usbHidReportDescriptor, packet->wLength, USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH);
				return 1;
			}
		}
	}

	return 0;
}

void usbUserSuspendEvent()
{
	wdt_disable();
}

void usbUserWakeUpEvent()
{
	wdt_enable(WDTO_500MS);
}
