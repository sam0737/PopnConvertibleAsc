/*

USBAVRUSB - USB device driver for AVR USB's controller

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

Version: $Id: usbdrv.c 3 2008-04-12 17:58:30Z sam $

*/

#include "usbdrv.h"

/*==========================================================================
 This is the implementation of the driver.
 Please see the "usbdrv.h" for public interface.
==========================================================================*/

#include <avr/interrupt.h>

#define USB_MAX_HARDWARE_ENDPOINT		4
#define USB_CONTROL_ENDPOINT_SIZE		USB_ENDPOINT_SIZE_32
#define USB_CONTROL_ENDPOINT_SIZE_VALUE	32

unsigned char _usb_configuration_index;
/* Used to backup the real configuration index during suspend */
unsigned char _usb_configuration_index_backup;

unsigned char _usb_device_feature = (USB_CFG_SELF_POWERED<<0);


/*--------------------------------------------------------------------------
   Descriptors
--------------------------------------------------------------------------*/

#define USB_DESCR_DEVICE_LENGTH 18
PROGMEM unsigned char usbDescriptorDevice[] =
{
	// Device Descriptor
    18,						// Descriptor Size: Always 18
    USBDESCR_DEVICE,        // Descriptor Type: Device
    0x00, 0x02,             // Little Endian format of USB Version in BCD in 0xJJMN (Major, Minor, Subminor)
	USB_CFG_DEVICE_CLASS,
	USB_CFG_DEVICE_SUBCLASS,
	USB_CFG_DEVICE_PROTOCOL,	
    USB_CONTROL_ENDPOINT_SIZE_VALUE,	// EndPoint 0 Size
	USB_CFG_VENDOR_ID,
	USB_CFG_PRODUCT_ID,
	USB_CFG_DEVICE_VERSION,
	
	#ifdef USB_CFG_VENDOR_NAME_INDEX
	USB_CFG_VENDOR_NAME_INDEX,
	#else
		#ifdef USB_CFG_VENDOR_NAME_LEN	
		1,
		#else
		0,
		#endif
	#endif
	
	#ifdef USB_CFG_PRODUCT_NAME_INDEX
	USB_CFG_PRODUCT_NAME_INDEX,
	#else
		#ifdef USB_CFG_PRODUCT_NAME_LEN	
		2,
		#else
		0,
		#endif
	#endif
	
	#ifdef USB_CFG_SERIAL_NUMBER_INDEX
	USB_CFG_SERIAL_NUMBER_INDEX,
	#else
		#ifdef USB_CFG_SERIAL_NUMBER_LEN	
		3,
		#else
		0,
		#endif
	#endif
	
    1,						// Number of Configuration
};

#if USB_SIMPLE_CONFIGURATION_DESCRIPTOR
#define USB_DESCR_CONFIG_LENGTH (9 * (USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH > 0 ? 3 : 2) + 7 * (USB_CFG_IN_INTR_ENDPOINT + USB_CFG_OUT_INTR_ENDPOINT))
PROGMEM unsigned char usbDescriptorConfiguration[] =
{
	// Configuration Descriptor
    9,						// Descriptor Size: Always 9
    USBDESCR_CONFIG,		// Descriptor Type: Configuration
	USB_DESCR_CONFIG_LENGTH & 0xFF,			// Total Length of the whole thing
	(USB_DESCR_CONFIG_LENGTH>>8) & 0xFF,	// including Config, Interfaces, Endpoints, Class/Vendor Descriptors
    1,						// Number of Configuration
    1,						// ID of This Configuration
    0,						// String of This Configuration
	(1<<7) | (USB_CFG_SELF_POWERED<<6) | (USB_CFG_REMOTE_WAKEUP<<5),	// Attributes	
    USB_CFG_MAX_POWER,		// Power in 2mA per Unit
	
	// Interface Descriptor	
    9,						// Descriptor Size: Always 9
    USBDESCR_INTERFACE,		// Descriptor Type: Interface
    0,						// ID of This Interface
	0,						// Alternate Setting ID
	USB_CFG_IN_INTR_ENDPOINT + USB_CFG_OUT_INTR_ENDPOINT,				// Number of Endpoints (Excluding Control)
	USB_CFG_INTERFACE_CLASS,
    USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    0,						// String of This Interface
	
	#if USB_CFG_INTERFACE_CLASS == 3    /* HID descriptor */
	// HID Descriptor
    9,						// Descriptor Size: 9 for our case
    USBDESCR_HID,			// Descriptor Type: HID
	0x01, 0x01,				// Little Endian format of HID Version in BCD in 0xJJMN (Major, Minor, Subminor)		
    0x00,					// Country Code
    0x01,					// Number of HID Report Descirptors
    0x22,					// Descriptor Type of the 1st and the only Descriptor
    USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH & 0xFF,
	(USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH>>8) & 0xFF,					// Length of that HID Report Descriptors
	#endif
	
	#if USB_CFG_IN_INTR_ENDPOINT
	// Endpoint 3 Descriptor
    7,						// Descriptor Size: Always 7
    USBDESCR_ENDPOINT,		// Descriptor Type: Endpoint
    0x80 | USB_CFG_IN_ENDPOINT_NO,			// IN Endpoint
    0x03,					// Attribute: Interrupt
    USB_CFG_IN_INTR_ENDPOINT_SIZE, 0,		// Packet Size
    USB_CFG_INTR_POLL_INTERVAL,				// Polling Interval in ms
	#endif
	
	#if USB_CFG_OUT_INTR_ENDPOINT
	// Endpoint 4 Descriptor
    7,						// Descriptor Size: Always 7
    USBDESCR_ENDPOINT,		// Descriptor Type: Endpoint
    0x00 | USB_CFG_OUT_ENDPOINT_NO,			// OUT Endpoint
    0x03,					// Attribute: Interrupt
    USB_CFG_OUT_INTR_ENDPOINT_SIZE, 0,		// Packet Size
    USB_CFG_INTR_POLL_INTERVAL,				// Polling Interval in ms
	#endif
};
#endif

PROGMEM unsigned char usbDescriptorString0[] =
{
	// Language Descriptor: 0
    4,						// Descriptor Size: 4 in our case
    USBDESCR_STRING,		// Descriptor Type: String
    0x09, 0x04,				// Always report 0x0409: US-English
};

#if USB_CFG_VENDOR_NAME_LEN
PROGMEM unsigned int usbDescriptorString1[] =
{
	// Language Descriptor: 1, Vendor Name
    (USB_CFG_VENDOR_NAME_LEN * 2 + 2) | (USBDESCR_STRING<<8), // Size and Descriptor Type
    USB_CFG_VENDOR_NAME
};
#endif

#if USB_CFG_PRODUCT_NAME_LEN
PROGMEM unsigned int usbDescriptorString2[] =
{
	// Language Descriptor: 2, Product Name
    (USB_CFG_PRODUCT_NAME_LEN * 2 + 2) | (USBDESCR_STRING<<8), // Size and Descriptor Type
    USB_CFG_PRODUCT_NAME
};
#endif

#if USB_CFG_SERIAL_NUMBER_LEN
PROGMEM unsigned int usbDescriptorString3[] =
{
	// Language Descriptor: 3, Serial Number
    (USB_CFG_SERIAL_NUMBER_LEN * 2 + 2) | (USBDESCR_STRING<<8), // Size and Descriptor Type
    USB_CFG_SERIAL_NUMBER
};
#endif


/*--------------------------------------------------------------------------
   Code
--------------------------------------------------------------------------*/

// Forward Declaration
void usbInitToDefault(void);

ISR(USB_GEN_vect)
{
	// Start Of Frame
	if (UDINT & (1<<SOFI))
	{
		#ifdef usbUserSOFEvent_Handler
		usbUserSOFEvent_Handler();
		#endif
		UDINT &= ~(1<<SOFI);
	}
	// Suspend
	if (0 && (UDINT & (1<<SUSPI)))
	{
		// Enable WakeUp Interrupt
		UDINT &= ~(1<<WAKEUPI);
		UDIEN |= (1<<WAKEUPE);
	
		// Disable PLL
		USBCON |= (1<<FRZCLK);
		PLLCSR &= ~(1<<PLLE);
		
		#ifdef usbUserSuspendEvent_Handler
		usbUserSuspendEvent_Handler();
		#endif
		UDINT &= ~(1<<SUSPI);
	}
	// Wakeup
	if (0 && (UDINT & (1<<WAKEUPI)))
	{
		#ifdef usbUserWakeUpEvent_Handler
		usbUserWakeUpEvent_Handler();
		#endif
		
		// Enable PLL
		PLLCSR |= (1<<PLLE);
		
		// Disable WakeUp Interrupt
		UDINT &= ~(1<<WAKEUPI);
		UDIEN &= ~(1<<WAKEUPE);
				
		// Check and Wait PLL
		while (!(PLLCSR & (1<<PLOCK)));
		USBCON &= ~(1<<FRZCLK);
	}
	// USB Reset
	if (UDINT & (1<<EORSTI))
	{
		usbInitToDefault();
		#ifdef usbUserResetEvent_Handler
		usbUserResetEvent_Handler();
		#endif
		UDINT &= ~(1<<EORSTI);
	}
}

void usbInit(void)
{
	// USB Reset will reset the system
	UDCON |= (1<<RSTCPU);
	
	// Shutdown and Reset USB for sure
	USBCON &= ~(1<<USBE);
	
	// D-Pad Regulator
	#if usbEnableRegulator
		REGCR &= ~(1<<REGDIS);
	#else
		REGCR |= (1<<REGDIS);
	#endif
	
		
	// Configure and Enable PLL
	#if F_CPU == 8000000
		PLLCSR = (0<<PLLP2) | (0<<PLLP1) | (0<<PLLP0) | (1<<PLLE);
	#elif F_CPU == 16000000
		PLLCSR = (0<<PLLP2) | (0<<PLLP1) | (1<<PLLP0) | (1<<PLLE);
	#else
		#error "Unknown F_CPU speed"
	#endif
		
	// Clear USB Flags and Interrupt
	_usb_configuration_index = 0;
	UDINT = 0;
	
	// Reset all USB Endpoints
	UERST = (unsigned char) (2 << USB_MAX_HARDWARE_ENDPOINT) - 1;
			
	// Check and Wait PLL
	while (!(PLLCSR & (1<<PLOCK)));
	USBCON &= ~(1<<FRZCLK);
	
	// Enable USB (USBE)
	USBCON |= (1<<USBE);
	
	// Configure USB Interrupt
	UDIEN = (1<<EORSTE) | (1<<SUSPE)
		#ifdef usbUserResumeEvent_Handler
		| (1<<EORSME)
		#endif
		#ifdef usbUserSOFEvent_Handler
		| (1<<SOFE)
		#endif
		;
		
	// Attach USB
	UDCON &= ~(1<<DETACH);
	
	// Configure the EndPoint 0
	usbInitToDefault();
}

// Initialize to the Default state
void usbInitToDefault(void)
{
	_usb_configuration_index = 0;
	_usb_device_feature = (USB_CFG_SELF_POWERED<<0);
	
	// Unfreeze clock and Enable USB if not the case (We might be coming from Reset)
	USBCON &= ~(1<<FRZCLK);
	USBCON |= (1<<USBE);
	
	// Configure Endpoint 0
	usbConfigureEndPoint(0, USB_ENDPOINT_TYPE_CONTROL, USB_ENDPOINT_DIR_OUT, USB_CONTROL_ENDPOINT_SIZE, USB_ENDPOINT_BANK_ONE);
}

// Initialize to the Configured state (or Address State if it's 0)
void usbToConfigured(unsigned char index)
{
	UERST = (unsigned char) (2 << USB_MAX_HARDWARE_ENDPOINT) - 1 - 1;
	UERST = 0;
	for (unsigned char i = USB_MAX_HARDWARE_ENDPOINT; i != 0; i--)
	{
		UENUM = i;
		UECONX &= ~(1<<EPEN);
		UECONX |= (1<<RSTDT);
		UECFG1X &= ~(1<<ALLOC);
	}
	
	if (index != 0)
	{
		#if USB_CFG_IN_INTR_ENDPOINT
		// Configure Endpoint 3
		usbConfigureEndPoint(USB_CFG_IN_ENDPOINT_NO, USB_ENDPOINT_TYPE_INTR, USB_ENDPOINT_DIR_IN, 
			#if USB_CFG_IN_INTR_ENDPOINT_SIZE == 8
				USB_ENDPOINT_SIZE_8
			#elif USB_CFG_IN_INTR_ENDPOINT_SIZE == 16
				USB_ENDPOINT_SIZE_16
			#elif USB_CFG_IN_INTR_ENDPOINT_SIZE == 32
				USB_ENDPOINT_SIZE_32
			#elif USB_CFG_IN_INTR_ENDPOINT_SIZE == 64
				USB_ENDPOINT_SIZE_64
			#else
				#error "Unknown IN_INTR EndPoint size"
			#endif
		,
			#if USB_CFG_IN_INTR_ENDPOINT_BANK == 1
				USB_ENDPOINT_BANK_ONE
			#elif USB_CFG_IN_INTR_ENDPOINT_BANK == 2
				USB_ENDPOINT_BANK_TWO
			#else
				#error "Unknown IN_INTR EndPoint Bank"
			#endif
		);
		#endif
		#if USB_CFG_OUT_INTR_ENDPOINT
		// Configure Endpoint 4
		usbConfigureEndPoint(USB_CFG_OUT_ENDPOINT_NO, USB_ENDPOINT_TYPE_INTR, USB_ENDPOINT_DIR_OUT, 
			#if USB_CFG_OUT_INTR_ENDPOINT_SIZE == 8
				USB_ENDPOINT_SIZE_8
			#elif USB_CFG_OUT_INTR_ENDPOINT_SIZE == 16
				USB_ENDPOINT_SIZE_16
			#elif USB_CFG_OUT_INTR_ENDPOINT_SIZE == 32
				USB_ENDPOINT_SIZE_32
			#elif USB_CFG_OUT_INTR_ENDPOINT_SIZE == 64
				USB_ENDPOINT_SIZE_64
			#else
				#error "Unknown OUT_INTR EndPoint size"
			#endif
		,
			#if USB_CFG_OUT_INTR_ENDPOINT_BANK == 1
				USB_ENDPOINT_BANK_ONE
			#elif USB_CFG_OUT_INTR_ENDPOINT_BANK == 2
				USB_ENDPOINT_BANK_TWO
			#else
				#error "Unknown OUT_INTR EndPoint Bank"
			#endif
		);
		#endif
	}
	UENUM = 0;
}

void usbShutdown(void)
{
	_usb_configuration_index = 0;
	_usb_device_feature = (USB_CFG_SELF_POWERED<<0);
	
	// Detach
	UDCON |= (1<<DETACH);
	
	// Disable USB
	USBCON &= ~(1<<USBE);
	
	// Disable PLL
	USBCON |= (1<<FRZCLK);
	PLLCSR &= ~(1<<PLLE);
}

void usbPoll(void)
{
	// Check if Setup packet is received
	UENUM = 0;
	if ((USBCON & (1<<USBE)) && (UEINTX & (1<<RXSTPI)))
	{
		// SETUP are always 8 bytes long
		if (UEBCLX == 8)
		{
			// Reading the SETUP packet into the struct
			struct usbSetupPacket packet;
			unsigned char *ptr, *reply;
			ptr = reply =  (unsigned char*) &packet;
			
			for (unsigned char i = 0; i < 8; i++)
			{
				*ptr++ = UEDATX;
			}
			
			#ifdef usbUserFunctionSetup_Handler
			// If firmware has handled it, we are fine
			if (usbUserFunctionSetup_Handler(&packet) != 0) return;
			#endif
			
			unsigned char type = packet.bmRequestType & USBSETUP_TYPE_MASK;
			if (type == USBSETUP_TYPE_STANDARD)
			{
				if (packet.bRequest == USBSETUP_REQ_GET_STATUS)
				{
					if (packet.bmRequestType == 0x80) // Device
					{
						reply[0] = _usb_device_feature;
					}
					else if (packet.bmRequestType == 0x81 && packet.wIndex == 0) // Interface
					{
						reply[0] = 0;
					}
					else if (packet.bmRequestType == 0x82) // Endpoint
					{
						unsigned char ep = packet.wIndex[0] & USBSETUP_ENDPOINT_MASK;
						if (ep <= USB_MAX_HARDWARE_ENDPOINT)
						{
							UENUM = ep;
							if (UECONX & (1<<EPEN)) {
								if (UECONX & (1<<STALLRQ)) {
									reply[0] = 1;
								}
							} else {
								usbSetupStall();
								UENUM = 0;
								return;
							}
							UENUM = 0;
						}
						else
						{						
							usbSetupStall();
							return;
						}
					}
					else
					{
						usbSetupStall();
						return;
					}
					reply[1] = 0;
					usbSetupDataIn(reply, packet.wLength, 2);
				}
				else if (packet.bRequest == USBSETUP_REQ_CLEAR_FEATURE)
				{
					if (packet.bmRequestType == 0x00 && packet.wValue_i == USBFEATURE_DEVICE_REMOTE_WAKEUP) // Device (Wakeup)
					{
						_usb_device_feature &= ~(1<<USBFEATURE_DEVICE_REMOTE_WAKEUP);
					}
					else if (packet.bmRequestType == 0x02 && packet.wValue_i == USBFEATURE_DEVICE_ENDPONIT_HALT) // Endpoint (Halt)
					{
						unsigned char ep = packet.wIndex[0] & USBSETUP_ENDPOINT_MASK;
						if (ep <= USB_MAX_HARDWARE_ENDPOINT)
						{
							UENUM = ep;
							if (UECONX & (1<<EPEN)) {
								UECONX |= (1<<STALLRQC) | (1<<RSTDT);								
							} else {
								usbSetupStall();
								UENUM = 0;
								return;
							}
							UENUM = 0;
						}
						else
						{						
							usbSetupStall();
							return;
						}
					}
					else
					{
						usbSetupStall();
						return;
					}
					usbSetupDataOut(0, 0);
				}
				else if (packet.bRequest == USBSETUP_REQ_SET_FEATURE)
				{
					if (packet.bmRequestType == 0x00 && packet.wValue_i == USBFEATURE_DEVICE_REMOTE_WAKEUP) // Device
					{
						_usb_device_feature |= (1<<USBFEATURE_DEVICE_REMOTE_WAKEUP);
					}
					else if (packet.bmRequestType == 0x02 && packet.wValue_i == USBFEATURE_DEVICE_ENDPONIT_HALT) // Endpoint
					{
						unsigned char ep = packet.wIndex[0] & USBSETUP_ENDPOINT_MASK;
						if (ep <= USB_MAX_HARDWARE_ENDPOINT)
						{
							UENUM = ep;
							if (UECONX & (1<<EPEN)) {
								UECONX |= (1<<STALLRQ);
							} else {
								usbSetupStall();
								UENUM = 0;
								return;
							}
							UENUM = 0;
						}
						else
						{						
							usbSetupStall();
							return;
						}
					}
					else
					{
						usbSetupStall();
						return;
					}
					usbSetupDataOut(0, 0);
				}
				else if (packet.bRequest == USBSETUP_REQ_SET_ADDRESS)
				{
					if (packet.bmRequestType == 0x00) // Device
					{
						usbSetupDataOut(0, 0);
						UDADDR = packet.wValue_i & USBADDRESS_ADDRESS_MASK;
						UDADDR |= (1<<ADDEN);
					}
					else
					{
						usbSetupStall();
						return;
					}
				}
				else if (packet.bRequest == USBSETUP_REQ_GET_DESCRIPTOR)
				{
					if (packet.bmRequestType == 0x80) // Device
					{
						if (packet.wValue[1] == USBDESCR_DEVICE && packet.wValue[0] == 0) 
						{
							usbSetupDataInPGM(usbDescriptorDevice, packet.wLength, USB_DESCR_DEVICE_LENGTH);
						}
						#if USB_SIMPLE_CONFIGURATION_DESCRIPTOR
						else if (packet.wValue[1] == USBDESCR_CONFIG && packet.wValue[0] == 0) 
						{
							usbSetupDataInPGM(usbDescriptorConfiguration, packet.wLength, USB_DESCR_CONFIG_LENGTH);
						}
						#endif
						// String Descriptor: We are not focusing on Language ID
						else if (packet.wValue[1] == USBDESCR_STRING && packet.wValue[0] == 0) 
						{
							usbSetupDataInPGM((unsigned char* PROGMEM) usbDescriptorString0, packet.wLength, 4);
						}
						#if USB_CFG_VENDOR_NAME_LEN
						if (packet.wValue[1] == USBDESCR_STRING && packet.wValue[0] == 1) 
						{
							usbSetupDataInPGM((unsigned char* PROGMEM) usbDescriptorString1, packet.wLength, USB_CFG_VENDOR_NAME_LEN * 2 + 2);
						}
						#endif
						#if USB_CFG_PRODUCT_NAME_LEN
						if (packet.wValue[1] == USBDESCR_STRING && packet.wValue[0] == 2) 
						{
							usbSetupDataInPGM((unsigned char* PROGMEM) usbDescriptorString2, packet.wLength, USB_CFG_PRODUCT_NAME_LEN * 2 + 2);
						}
						#endif
						#if USB_CFG_SERIAL_NUMBER_LEN
						if (packet.wValue[1] == USBDESCR_STRING && packet.wValue[0] == 3) 
						{
							usbSetupDataInPGM((unsigned char* PROGMEM) usbDescriptorString3, packet.wLength, USB_CFG_SERIAL_NUMBER_LEN * 2 + 2);
						}
						#endif
						else
						{
							usbSetupStall();
							return;
						}
					}
					#if USB_CFG_INTERFACE_CLASS == 3 && USB_SIMPLE_CONFIGURATION_DESCRIPTOR
					else if (packet.bmRequestType == 0x81) // Interface
					{
						if (packet.wValue[1] == USBDESCR_HID) 
						{
							usbSetupDataInPGM(usbDescriptorConfiguration + 18, packet.wLength, 9);
						}
						else
						{						
							usbSetupStall();
							return;
						}
					}
					#endif
					else
					{
						usbSetupStall();
						return;
					}
				}
				// USBSETUP_REQ_GET_DESCRIPTOR is not supported
				else if (packet.bRequest == USBSETUP_REQ_GET_CONFIGURATION)
				{
					if (packet.bmRequestType == 0x80) // Device
					{
						reply[0] = _usb_configuration_index;
						usbSetupDataIn(reply, packet.wLength, 1);
					}
					else
					{
						usbSetupStall();
						return;
					}
				}
				else if (packet.bRequest == USBSETUP_REQ_SET_CONFIGURATION)
				{
					if (packet.bmRequestType == 0x00) // Device
					{
						// We don't really care Race Condition here. (with Suspend/Reset USB interrupt)
						// If the USB were enabled on the top of the function, it probably can
						// reach here within 1 ms, and that Suspend/Reset shouldn't happen yet.
						_usb_configuration_index = packet.wValue[0];
						#if USB_SIMPLE_CONFIGURATION_DESCRIPTOR
						usbToConfigured(packet.wValue[0]);
						#else
							#ifdef usbUserSetConfigurationEvent_Handler
								usbUserSetConfigurationEvent_Handler(packet.wValue[0]);
							#else
								#error "usbUserSetConfigurationEvent_Handler or USB_SIMPLE_CONFIGURATION_DESCRIPTOR must be defined"
							#endif
						#endif
						usbSetupDataOut(0, 0);
					}
					else
					{
						usbSetupStall();
						return;
					}
				}
				else if (packet.bRequest == USBSETUP_REQ_GET_INTERFACE)
				{
					if (packet.bmRequestType == 0x81) // Interface
					{
						reply[0] = 1;
						usbSetupDataIn(reply, packet.wLength, 1);
					}
					else
					{
						usbSetupStall();
						return;
					}
				}
				// USBSETUP_REQ_SET_INTERFACE is not supported
				// USBSETUP_REQ_SYNCH_FRAME is not supported
				else
				{
					usbSetupStall();
					return;
				}
			}
			else
			{
				usbSetupStall();
				return;
			}
		}
		else
		{
			// Stall if it's not 8 bytes long
			usbSetupStall();
		}
	}
}


void usbSetupDataIn(unsigned char *ptr, unsigned int maxlength, unsigned int length)
{
	unsigned char zlp = 0;
	unsigned char count;
	
	// Clear Receive Setup flag
	UEINTX &= ~(1<<RXSTPI);
	
	// Clear Out NAK flag
	// The first status OUT packet by host will always be NAK
	// We will monitor this flag to see if the host has aborted or not
	UEINTX &= ~(1<<NAKOUTI);
	
	// Max Length limitation
	if (maxlength < length) length = maxlength;
	
	// Do we need to send a ZLP to signal the end of phase?
	// Only the case when we can't use short packet.
	if ((length % USB_CONTROL_ENDPOINT_SIZE_VALUE) == 0)
		zlp = 1;
	
	while (length != 0 && (UEINTX & (1<<NAKOUTI)) == 0)
	{
		// Fill the USB buffer
		count = 0;
		while (length != 0)
		{
			if (count++ == USB_CONTROL_ENDPOINT_SIZE_VALUE)
				break;
			UEDATX = *ptr++;
			length--;	
		}
		
		// Send!
		UEINTX &= ~(1<<TXINI);
		
		// Wait until we have finished sending.
		// While we are waiting, let's check for the abort conditions
		do
		{
			// Host has abort / acknowledge 
			if (UEINTX & (1<<NAKOUTI))
				goto IN_STATUS_STAGE_ABORT;
			// USB is reset
			if (!(USBCON & (1<<USBE)))
				goto IN_STATUS_STAGE_ABORT;
		} while ((UEINTX&(1<<TXINI)) == 0);
	}

	// If we need to send an Zero Length Packet
	if (zlp)
	{
		// Send!
		UEINTX &= ~(1<<TXINI);
	}	
	
	// Wait until the host acknowledge us 
	while (!(UEINTX & (1<<NAKOUTI)));

IN_STATUS_STAGE_ABORT:
	// Clear all the flags and return
	UEINTX &= ~((1<<NAKOUTI) | (1<<RXOUTI));
}

void usbSetupDataInPGM(const unsigned char* PROGMEM ptr, unsigned int maxlength, unsigned int length)
{
	unsigned char zlp = 0;
	unsigned char count;
	
	// Clear Out RX Out flag
	UEINTX &= ~(1<<RXOUTI);
	
	// Clear Receive Setup flag
	UEINTX &= ~(1<<RXSTPI);	
	
	// Max Length limitation
	if (maxlength < length) length = maxlength;
	
	// Do we need to send a ZLP to signal the end of phase?
	// Only the case when we can't use short packet.
	if ((length % USB_CONTROL_ENDPOINT_SIZE_VALUE) == 0)
		zlp = 1;
		
	while (length != 0 && (UEINTX & (1<<NAKOUTI)) == 0)
	{
		// Fill the USB buffer
		count = 0;
		while (length != 0)
		{
			if (count++ == USB_CONTROL_ENDPOINT_SIZE_VALUE)
				break;
			UEDATX = pgm_read_byte_near(ptr++);
			length--;	
		}
		
		// Send!
		UEINTX &= ~(1<<TXINI);		

		// Wait until we have finished sending.
		// While we are waiting, let's check for the abort conditions
		do
		{
			// Host has abort / acknowledge 
			if (UEINTX & (1<<NAKOUTI))
				goto INPGM_STATUS_STAGE_ABORT;
			// USB is reset
			if (!(USBCON & (1<<USBE)))
				goto INPGM_STATUS_STAGE_ABORT;
		} while ((UEINTX&(1<<TXINI)) == 0);
	}

	// If we need to send an Zero Length Packet
	if (zlp)
	{
		// Send!
		UEINTX &= ~(1<<TXINI);
	}	
	
	// Wait until the host acknowledge us
	while (!(UEINTX & (1<<NAKOUTI)));

	// Wait until the real OUT has been received
	while (!(UEINTX & (1<<RXOUTI)));

INPGM_STATUS_STAGE_ABORT:
	// Clear all the flags and return
	UEINTX &= ~((1<<NAKOUTI)|(1<<RXOUTI));
}

unsigned int usbSetupDataOut(unsigned char *ptr, unsigned int length)
{
	unsigned int total = length;	
	
	// Clear IN NAK, RX OUT flag
	UEINTX &= ~((1<<NAKINI) | (1<<RXOUTI));
	
	// Clear Receive Setup flag
	UEINTX &= ~(1<<RXSTPI);
		
	while (length != 0 && (UEINTX & (1<<NAKINI)) == 0)
	{	
		// Wait until we have finished receiving.
		// While we are waiting, let's check for the abort conditions
		do
		{
			// Host has gone to status page
			if (UEINTX & (1<<NAKINI))
				goto OUT_STATUS_STAGE;
			// USB is reset
			if (!(USBCON & (1<<USBE)))
				goto OUT_STATUS_STAGE;
		} while ((UEINTX&(1<<RXOUTI)) == 0);
		
		
		// Fill the landing buffer
		while (UEBCLX != 0)
		{
			if (length == 0)
				goto OUT_STATUS_STAGE;
			*ptr++ = UEDATX;
			length--;	
		}
		
		// Receive!
		UEINTX &= ~(1<<RXOUTI);
	}

OUT_STATUS_STAGE:
	if (UEBCLX)
	{
		// Still have data awaiting for read? Stall!
		usbSetupStall();
		UEINTX &= ~(1<<RXOUTI);
		return 0xFFFF;
	}
	else
	{
		UEINTX &= ~(1<<TXINI);
		UEINTX &= ~(1<<RXOUTI);
		// Wait until the host enter the status stage
		while (!(UEINTX & (1<<TXINI)))
		{
			if (UEINTX&(1<<RXOUTI))
			{
				// Still have data awaiting for read? Stall!
				usbSetupStall();
				UEINTX &= ~(1<<RXOUTI);
				return 0xFFFF;
			}
		}
	}
	
	// Clear all the flags and return
	UEINTX &= ~((1<<NAKOUTI) | (1<<RXOUTI));
	return total - length;
}

void usbSetupStall(void)
{
	UECONX |= (1<<STALLRQ);
	UEINTX &= ~(1<<RXSTPI);
}

void usbConfigureEndPoint(
	unsigned char endpoint, unsigned char type, unsigned char dir, 
	unsigned char size,	unsigned char bank)
{
	// Select EndPoint
	UENUM = endpoint;
	
	// Reset and Enable EndPoint
	UECONX &= ~(1<<EPEN);
	UECONX |= (1<<EPEN);
	
	UEIENX = 0;
	UEINTX = 0;
	
	// Configuration (Preserving the unknown bits)
    UECFG0X = (UECFG0X & 0x3E) | (type<<6) | (dir<<0);
    UECFG1X = (UECFG1X & 0x81) | (size<<4) | (bank<<2);
	UECFG1X |= (1<<ALLOC) ;
}

