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

Version: $Id: usbdrv.h 9 2008-04-20 15:44:53Z sam $

*/

#ifndef __USBDRV_H__
#define __USBDRV_H__
#include "usbconfig.h"

/*==========================================================================
 This file is served as the driver public interface.
 Modification is not recommended.
==========================================================================*/

#include <avr/io.h>
#include <avr/pgmspace.h>

/*--------------------------------------------------------------------------
   Public Interface 
--------------------------------------------------------------------------*/

/* Version identifier in YYYYMMDD */
#define USBAVRUSB_VERSION  20080320

struct usbSetupPacket; /* forward declaration */

extern unsigned char _usb_configuration_index;
extern unsigned char _usb_device_feature;

/**
 * Initialize and start the USB Interface.
 *
 * After initializing, one must
 * 1. call sei() to enable global interrupt
 * 2. call usbPoll() periodically.
 *
 * This better be done before global interrupt is enabled to prevent noise.
 *
 * Depends on your MCU and fuse bit, you might want to turn off clock prescalar
 * with the following code so MCU is running at the full clock speed
 * 	// Turn off clock Prescaler
 *	CLKPR = (1<<CLKPCE);
 *	CLKPR = 0;
 *
 */
void usbInit(void);

/**
 * Shutdown the USB interface. Same as unplugging from the Host prospective.
 * The USB clock will also be stopped.
 */
void usbShutdown(void);

/**
 * USB Polling. Must be call every 50ms in the worst case to handle the SETUP
 * request.
 */
void usbPoll(void);

/**
 * In DATA Phase of SETUP Transaction, push the data in the pointer to the 
 * USB host through the EndPoint 0.
 *
 * SETUP Interrupt (RXSTPI) will be cleared in the function. Upon returned, 
 * the whole SETUP Transaction is completed.
 *
 * @param ptr The pointer to the data
 * @param length The length of the data to be sent
 */
void usbSetupDataIn(unsigned char *ptr, unsigned int maxlength, unsigned int length);

/**
 * In DATA Phase of SETUP Transaction, push the data in the pointer to the 
 * USB host through the EndPoint 0. The data is read from the Program Space.
 *
 * SETUP Interrupt (RXSTPI) will be cleared in the function. Upon returned, 
 * the whole SETUP Transaction is completed.
 *
 * @param ptr The pointer to the data in Program Space
 * @param length The length of the data to be sent
 */
void usbSetupDataInPGM(const unsigned char* PROGMEM ptr, unsigned int maxlength, unsigned int length);

/**
 * In DATA Phase of SETUP Transaction, read the data to the pointer from the 
 * USB host through the EndPoint 0.
 *
 * SETUP Interrupt (RXSTPI) will be cleared in the function. Upon returned, 
 * the whole SETUP Transaction is completed.
 *
 * If there are more data sent from Host than expected, this function will
 * return a STALL handshake.
 *
 * @param ptr The pointer to the empty memory so that data can be stored to.
 * @param length The length of the data to be received
 * @return Actual length of data received. 0xFFFF if too much data is received.
 */
unsigned int usbSetupDataOut(unsigned char *ptr, unsigned int length);

/**
 * STALL an Setup Request. Used when a requset is not recognized
 */
void usbSetupStall(void);

/**
 * Check if the USB is up nad ready and not in a reset state
 *
 * @return 1 if ready. 0 if not.
 */
static inline unsigned char usbIsUSBReady(void);

/**
 * Check if the USB has be enumerated and non-Control endpoints are Setup.
 * Also check if USB is available (Not suspending, still attached and such)
 *
 * @return 0 for not Ready. Non-zero donates the current active configuration
 *         selected.
 */
static inline unsigned char usbCurrentConfiguration(void);

/**
 * Select the EndPoint and also check if there is something to Read, and clear
 * the RXOUTI if needed.
 *
 * This won't tell you how many spaces we have. The firmware should check the
 * BYCT register by itself.
 *
 * @param endpoint The endpoint number. Can only be >0
 * @return 0 for not Ready. Non-zero for Ready.
 */
static inline unsigned char usbIsEndPointReadable(unsigned char endpoint);

/**
 * Select the EndPoint and also check if there is space to Write something,
 * and clear the TXINI if needed.
 *
 * This won't tell you how many spaces we have. The firmware should know the 
 * size of an EndPoint and act accordingly.
 *
 * @param endpoint The endpoint number. Can only be >0
 * @return 0 for not Ready. Non-zero for Ready.
 */
static inline unsigned char usbIsEndPointWritable(unsigned char endpoint);

/**
 * Select the EndPoint
 *
 * @param endpoint The endpoint number.
 */
static inline void usbSelectEndpoint(unsigned char endpoint);

/**
 * Return the number of bytes available for reading
 *
 * @return Number of available bytes for reading
 */
static inline unsigned char usbByteAvailable(void);

/**
 * Write a byte to the EndPoint buffer
 *
 * @param data The byte to be written
 */
static inline void usbWriteData(unsigned char data);

/**
 * Read a byte from the EndPoint buffer
 *
 * @return the byte read
 */
static inline unsigned char usbReadData(void);

/**
 * End the writing to a buffer. Signal the MCU to transmit it.
 */
static inline void usbEndWrite(void);

/**
 * End the reading from a buffer. Signal the CPU to free it.
 */
static inline void usbEndRead(void);

/**
 * Check if the device is allowed to do Remote Wakeup. Host uses 
 * DEVICE[SET_FEATURE] to give device this ability.
 *
 * @return 0 for No. Non-zero for Yes. 
 * @see USB_CFG_REMOTE_WAKEUP in usbconfig.h
 */
static inline unsigned char usbIsRemoteWakeupAllowed(void);

/**
 * Remote Wakeup a host.
 *
 * Firmware should check usbIsRemoteWakeupAllowed() and
 * usbIsInSuspend(), strictly speaking, WakeUp is allowed only when both are
 * true.
 *
 * @see usbUserResumeEvent 
 * @see USB_CFG_REMOTE_WAKEUP in usbconfig.h
 */
static inline void usbRemoteWakeup(void);

/**
 * Configure a EndPoint
 *
 * @param ep   Endpoint Index
 * @param type Endpoint Type: 0 = Control, 1 = Iso, 2 = Bulk, 3 = Interrupt
 * @param dir  Direction of the Endpoint: 0 = Out, 1 = In
 * @param size Size of one bank of the Endpoint in Bytes
 * @param bank Single (1) or Double (2) bank
 * @note For Control endpoint, Dir should be Out, Bank should be Single.
 *       It is acutally handled by the driver. Implementation should not care.
 */
void usbConfigureEndPoint(
	unsigned char endpoint, unsigned char type, unsigned char dir, 
	unsigned char size,	unsigned char bank);

#define USB_ENDPOINT_TYPE_CONTROL 0
#define USB_ENDPOINT_TYPE_ISO     1
#define USB_ENDPOINT_TYPE_BULK    2
#define USB_ENDPOINT_TYPE_INTR    3

#define USB_ENDPOINT_DIR_OUT      0
#define USB_ENDPOINT_DIR_IN       1

#define USB_ENDPOINT_SIZE_8       0
#define USB_ENDPOINT_SIZE_16      1
#define USB_ENDPOINT_SIZE_32      2
#define USB_ENDPOINT_SIZE_64      3

#define USB_ENDPOINT_BANK_ONE     0
#define USB_ENDPOINT_BANK_TWO     1
   
/*--------------------------------------------------------------------------
   Event Handler Signature
   
   This handler can be hooked optionally.   
   usb-config.h is used to configure which one should be hooked, while
   the functions signature is defined here.
--------------------------------------------------------------------------*/

/**
 * Hook into the SETUP Packet processing. This is invoked upon the presence 
 * of the SETUP Packet, before the driver handling. Unhandled packet will be 
 * handled by the driver if possible.
 *
 * This function will be invoked during the usbPoll(). 
 *
 * @param The 8-byte usbSetupPacket of the SETUP packet
 * @return 1 if the SETUP Transaciton is handled completely.
 *         0 means the packet is not handled by the handler, the driver will 
 *         take care of it.  Or eventually will return a STALL handshake if
 *         no one knows how to handle it.
 * @see usbUserFunctionSetup_Handler in the usbconfig.h 
 */
extern unsigned char usbUserFunctionSetup(struct usbSetupPacket *packet);

/**
 * Invoked when SET_CONFIGURATION is received and the device is enumerated.
 *
 * This function will be invoked during the usbPoll(). 
 * 
 * @see usbUserSetConfigurationEvent_Handler in the usbconfig.h
 */
extern void usbUserSetConfigurationEvent(unsigned char index);

/**
 * Invoked when Start of Frame event is noticed.
 *
 * This will be execute under the USB Interrupt Service Routine, please
 * minimize the time.
 *
 * @see usbUserSOFEvent_Handler in the usbconfig.h
 */
extern void usbUserSOFEvent(void);

/**
 * Invoked when USB macro has just entered the Suspend state.
 *
 * This will be execute under the USB Interrupt Service Routine, please
 * minimize the time.
 *
 * @see usbUserSuspendEvent_Handler in the usbconfig.h
 */
extern void usbUserSuspendEvent(void);

/**
 * Invoked when traffic is detect on the USB in the Suspend state, before 
 * waking up the USB macro.
 *
 * This will be execute under the USB Interrupt Service Routine, please
 * minimize the time.
 * 
 * @note This will only be called when it were in Suspend, but not for every
 *       bus traffic
 * @see usbUserWakeUpEvent_Handler in the usbconfig.h
 */
extern void usbUserWakeUpEvent(void);

/**
 * Invoked when Host comes back and resume, after remote wakeup is done.
 *
 * This will be execute under the USB Interrupt Service Routine, please
 * minimize the time.
 * 
 * @see usbUserResumeEvent_Handler in usbconfig.h
 * @see usbRemoteWakeup
 */
extern void usbUserResumeEvent(void);

/**
 * Invoked when the USB is reset (Prolonged SE0 Condition). After the 
 * driver has just re-initialized the control Endpoint.
 *
 * This will be execute under the USB Interrupt Service Routine, please
 * minimize the time.
 * 
 * @see usbUserResetEvent_Handler in the usbconfig.h
 */
extern void usbUserResetEvent(void);

/*--------------------------------------------------------------------------
   Constant and Verification
--------------------------------------------------------------------------*/

#if (!defined USB_CFG_VENDOR_ID || !defined USB_CFG_DEVICE_ID)
#warning "You should really define your own USB_CFG_VENDOR_ID and USB_CFG_DEVICE_ID in usbconfig.h"
#endif

/* We still need an VID and PID defined in little endian order */
/* lowbyte, highbyte */
#ifndef USB_CFG_VENDOR_ID
#   define  USB_CFG_VENDOR_ID   0x2A, 0x00 /* 42. Answer to everything */
#endif

#ifndef USB_CFG_PRODUCT_ID
#   if USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH
#       define USB_CFG_PRODUCT_ID    0x00, 0x01  /* 256, for HIDs */
#   elif USB_CFG_INTERFACE_CLASS == 2
#       define USB_CFG_PRODUCT_ID    0x00, 0x02  /* 512, for CDC Modems */
#   else
#       define USB_CFG_PRODUCT_ID    0x00, 0x03  /* 768 for anything */
#   endif
#endif

#if USB_SIMPLE_CONFIGURATION_DESCRIPTOR
	#if !(USB_CFG_SELF_POWERED == 0 || USB_CFG_SELF_POWERED == 1)
		#error "USB_CFG_SELF_POWERED is out of range"
	#endif
	#if !(USB_CFG_REMOTE_WAKEUP == 0 || USB_CFG_REMOTE_WAKEUP == 1)
		#error "USB_CFG_REMOTE_WAKEUP is out of range"
	#endif
	#if !(USB_CFG_IN_INTR_ENDPOINT == 0 || USB_CFG_IN_INTR_ENDPOINT == 1)
		#error "USB_CFG_IN_INTR_ENDPOINT is out of range"
	#endif
	#if !(USB_CFG_OUT_INTR_ENDPOINT == 0 || USB_CFG_OUT_INTR_ENDPOINT == 1)
		#error "USB_CFG_OUT_INTR_ENDPOINT is out of range"
	#endif
#else
	#ifndef usbUserFunctionSetup_Handler
		#error "You must implement usbUserFunctionSetup if not using USB_SIMPLE_DESCRIPTOR_GENERATION"
	#endif
	#ifndef usbUserSetConfigurationEvent_Handler
		#error "You must handle usbUserSetConfigurationEvent if not using USB_SIMPLE_DESCRIPTOR_GENERATION"
	#endif
#endif

#if USB_CFG_INTERFACE_CLASS == 3
	#ifndef usbUserFunctionSetup_Handler
		#error "You must implement usbUserFunctionSetup for USD HID"
	#endif
#endif

#ifndef USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH
	#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH 0
#endif

#if !(F_CPU == 8000000 || F_CPU == 16000000)
	#error "Unsupported CPU speed"
#endif


/*--------------------------------------------------------------------------
   USB Specification Constant
--------------------------------------------------------------------------*/

typedef union usbWord {
    unsigned int word;
    unsigned char bytes[2];
} usbWord_t;

/* USB Setup Packet Structure and Definition */
typedef struct usbSetupPacket {
    unsigned char bmRequestType;
    unsigned char bRequest;
	union
	{
		unsigned int  wValue_i;
		unsigned char wValue[2];
	};
	union
	{
		unsigned int  wIndex_i;
		unsigned char wIndex[2];
	};
    unsigned int wLength;
} usbSetupPacket_t;

/* bmRequestType
 * D T T R R R R R
 * D ..... Direction: 0 = Host -> Device, 1 = Device -> Host
 * T ..... Type: 0 = Standard, 1 = Class, 2 = Vendor, 3 = Reserved
 * R ..... Recipient: 0 = Device, 1 = Interface, 2 = Endpoint, 3 = Other
 */

/* USB setup Packet - bmRequestType - Direction */
#define USBSETUP_DIR_MASK              0x80
#define USBSETUP_DIR_HOST_TO_DEVICE    (0<<7)
#define USBSETUP_DIR_DEVICE_TO_HOST    (1<<7)

/* USB setup Packet - bmRequestType - Recipient */
#define USBSETUP_RCPT_MASK         0x1F
#define USBSETUP_RCPT_DEVICE       0
#define USBSETUP_RCPT_INTERFACE    1
#define USBSETUP_RCPT_ENDPOINT     2

/* USB setup Packet - bmRequestType - Type */
#define USBSETUP_TYPE_MASK      0x60
#define USBSETUP_TYPE_STANDARD  (0<<5)
#define USBSETUP_TYPE_CLASS     (1<<5)
#define USBSETUP_TYPE_VENDOR    (2<<5)

/* The mask of EndPoint for wIndex */
#define USBSETUP_ENDPOINT_MASK  0x0F

/* USB setup Packet - bRequest */
// Standard
#define USBSETUP_REQ_GET_STATUS        0x00
#define USBSETUP_REQ_CLEAR_FEATURE     0x01
#define USBSETUP_REQ_SET_FEATURE       0x03
#define USBSETUP_REQ_SET_ADDRESS       0x05
#define USBSETUP_REQ_GET_DESCRIPTOR    0x06
#define USBSETUP_REQ_SET_DESCRIPTOR    0x07
#define USBSETUP_REQ_GET_CONFIGURATION 0x08
#define USBSETUP_REQ_SET_CONFIGURATION 0x09
#define USBSETUP_REQ_GET_INTERFACE     0x0A
#define USBSETUP_REQ_SET_INTERFACE     0X0B
#define USBSETUP_REQ_SYNCH_FRAME       0X0C
// Class 3 - HID (Section 7.2 of USB HID 1.11)
// USBSETUP_REQ_HID_GET_REPORT must be implemented, and also
// USBSETUP_REQ_HID_SET_REPORT if output/feature report is supported
#define USBSETUP_REQ_HID_GET_REPORT    0x01 // bmRequestType = 0xA1.
#define USBSETUP_REQ_HID_GET_IDLE      0x02 // bmRequestType = 0xA1. 
#define USBSETUP_REQ_HID_GET_PROTOCOL  0x03 // bmRequestType = 0xA1. 
#define USBSETUP_REQ_HID_SET_REPORT    0x09 // bmRequestType = 0x21.
#define USBSETUP_REQ_HID_SET_IDLE      0x0A // bmRequestType = 0x21. 
#define USBSETUP_REQ_HID_SET_PROTOCOL  0x0B // bmRequestType = 0x21. 

/* USB setup Packet - wValue - Features */
#define USBFEATURE_DEVICE_ENDPONIT_HALT 0x00
#define USBFEATURE_DEVICE_REMOTE_WAKEUP 0x01

/* USB Setup Packet - wValue - Address */
#define USBADDRESS_ADDRESS_MASK 0x7F

/* USB setup Packet - wValue - Descriptors */
// Standard (Section 7.1 of USB HID 1.11)
#define USBDESCR_DEVICE         0x01
#define USBDESCR_CONFIG         0x02
#define USBDESCR_STRING         0x03
#define USBDESCR_INTERFACE      0x04
#define USBDESCR_ENDPOINT       0x05
// Class 3 - HID (Always with bmRequestType = 0x81)
#define USBDESCR_HID            0x21
#define USBDESCR_HID_REPORT     0x22
#define USBDESCR_HID_PHYSICAL   0x23

/* USB setup Packet - wValue - Class HID_*_REPORT */
// This is the High byte of the wValue, Low byte is Report ID
#define USBHID_REPORT_INPUT     0x01
#define USBHID_REPORT_OUTPUT    0x02
#define USBHID_REPORT_FEATURE   0x03

/* USB Descriptor - bmAttributes - USBDESCR_CONFIG */
#define USBDESCR_CONFIG_ATTR_BASE         0x80
#define USBDESCR_CONFIG_ATTR_SELFPOWER    0x40
#define USBDESCR_CONFIG_ATTR_REMOTEWAKE   0x20

/*--------------------------------------------------------------------------
   Inline Functions
--------------------------------------------------------------------------*/
#include "usbdrv_inline.c"

#endif
