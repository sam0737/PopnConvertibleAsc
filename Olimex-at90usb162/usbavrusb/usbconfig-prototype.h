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

Version: $Id: usbconfig-prototype.h 3 2008-04-12 17:58:30Z sam $

*/

/*==========================================================================
 This file is intended to be copied and modified to suit your project needs.
 All user tunable parameters are listed in this file, please go through
 each item below.
==========================================================================*/

#ifndef __USBCONFIG_H__
#define __USBCONFIG_H__


/*--------------------------------------------------------------------------
   AT90USB Hardware
--------------------------------------------------------------------------*/
// Enable D Pads regulator. (See Chapter 7 of the Atmel Specification)
// 1: MCU in 5V operation and need Internal regulator
// 0: MCU in 3-3.3V operation and do not need Internal regulator
#define usbEnableRegulator 1

/*--------------------------------------------------------------------------
   Handler Interface 
   
   To hook into the following feature, you are expected to uncomment the 
   define line and implement the corresponding own function.
   
   The functions will be invoked upon the condition mentioned below.
   
   See usbdrv.h for the functions signature and parameters.
--------------------------------------------------------------------------*/

/*.................................
 * Hook into the SETUP Packet processing. This is invoked upon the presence 
 * of the SETUP Packet, before the driver handling. Unhandled packet will be 
 * handled by the driver if possible.
 *
 * If a particular SETUP Packet is unhandled by both here and driver, the 
 * driver will signal a STALL condition. Also a good place to override 
 * particular descriptor.
 *
 * In practice, you should handle all the the CLASS and VENDOR requests here.
 *
 * You should also handle CLASS's GET_REPORT(Feature) and SET_REPORT(Feature) 
 * request here if you are implementing HID.
 *
 * Note that Some OSes, notably original Windows 98, will use 
 * SET_REPORT(OUTPUT) mode even if you have Interrupt-OUT EndPoint.
 *
 * If the SETUP is a Control-IN Transfer, you should use usbSetupDataIn or 
 * usbSetupDataPGM to complete the data transfer.
 * If the SETUP is a Control-OUT Transfer, you should complete it by using
 * usbSetupDataOut
 */
// #define usbUserFunctionSetup_Handler usbUserFunctionSetup

/*.................................
 * Invoked when Start of Frame event is noticed.
 *
 * Start of Frame is sent by the Host for every 1ms. This is another good 
 * timer source for syncing with the host.
 *
 * Note: This is to be executed within the Interrupt Service Routine context.
 */
// #define usbUserSOFEvent_Handler usbUserSOFEvent

/*.................................
 * Invoked when USB macro has just entered the Suspend state.
 *
 * This usually happen due to the shutdown or sleep of the Host.
 *
 * The USB spec says that bus-powered device should enter the 500uA low power 
 * state within 7ms.
 *
 * Upon invoke, the driver has already done the job to shutdown the PLL Clock
 * of the USB.  Use this function to further reducing the power by shutting
 * down other components, turning to internal RC clock, etc.
 *
 * Note: This is to be executed within the Interrupt Service Routine context.
 */
// #define usbUserSuspendEvent_Handler usbUserSuspendEvent

/*.................................
 * Invoked when traffic is detect on the USB in the Suspend state, before 
 * waking up the USB macro.
 *
 * This happens when the host comes back alive.
 *
 * If you have enabled usbUserSuspendEvent_Handler, you probably want to 
 * implement this to undo the changes in Suspend.
 *
 * Upon return, the driver will start the USB.
 *
 * Note: This is to be executed within the Interrupt Service Routine context.
 */
// #define usbUserWakeUpEvent_Handler usbUserWakeUpEvent

/*.................................
 * Invoked when Host comes back and resume, after remote wakeup is done.
 *
 * This happens when the host comes back alive after a remote wakeup call.
 *
 * Note: This is to be executed within the Interrupt Service Routine context.
 */
// #define usbUserResumeEvent_Handler usbUserResumeEvent

/*.................................
 * Invoked when the USB is reset (Prolonged SE0 Condition). After the 
 * driver has just re-initialized the control Endpoint.
 *
 * This could be see at anytime, like during Startup. And is the strongest
 * evidence that the device is being noticed by the host.
 *
 * Note: This is to be executed within the Interrupt Service Routine context.
 */
// #define usbUserResetEvent_Handler usbUserResetEvent

/*.................................
 * Invoked when SET_CONFIGURATION is received and the device is enumerated.
 *
 * You may do the EndPoint initialization here, if the auto one is disabled.
 * See USB_SIMPLE_CONFIGURATION_DESCRIPTOR
 */
// #define usbUserSetConfigurationEvent_Handler usbUserSetConfigurationEvent


/*--------------------------------------------------------------------------
   Driver Behavior
--------------------------------------------------------------------------*/
/* Enable USB descriptor generated by the driver
 *
 * By answering a few parameters below, the driver can handle the 
 * GET_DESCRIPTOR, as well as Endpoint configuration.
 *
 * If you need more flexibility, you have to implemnet usbUserFunctionSetup to
 * manage the GET_DESCRIPTOR(Configuration) request, and 
 * usbUserSetConfigurationEvent to setup the Endpoint.
 */
#define USB_SIMPLE_CONFIGURATION_DESCRIPTOR 1

/*--------------------------------------------------------------------------
   USB Simple Configuration Descriptor Parameters and Settings
--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
   Ignore the following Unless using USB_SIMPLE_CONFIGURATION_DESCRIPTOR.
   However, you must then handle usbUserFunctionSetup to handle all the
   GET_DESCRIPTOR by yourself.
--------------------------------------------------------------------------*/
/*----- Endpoints ------
 * The size of each enabled endpoint * bank
 */
#define USB_CFG_CONTROL_ENDPOINT_SIZE    16

/* Interrupt Endpoint polling interval in millisecond */
#define USB_CFG_INTR_POLL_INTERVAL  20

/*----- Endpoint In Configuration -----
/* 0 = No In-Interrupt Endpoint
 * 1 = Enable In-Interrupt Endpoint at Endpoint 3
 */
#define USB_CFG_IN_INTR_ENDPOINT    1

/* Endpoint Number.  Only 3 and 4 can use double bank */
#define USB_CFG_IN_ENDPOINT_NO		3

/* The size of the endpoint */
#define USB_CFG_IN_INTR_ENDPOINT_SIZE 32

/* The bank mode: 1 = Single Bank, 2 = Double Bank */
#define USB_CFG_IN_INTR_ENDPOINT_BANK 2

/*----- Endpoint Out Configuration -----
/* 0 = No Out-Interrupt Endpoint
 * 1 = Enable Out-Interrupt Endpoint at Endpoint 4
 */
#define USB_CFG_OUT_INTR_ENDPOINT   1

/* Endpoint Number.  Only 3 and 4 can use double bank */
#define USB_CFG_OUT_ENDPOINT_NO		4

/* The size of the endpoint */
#define USB_CFG_OUT_INTR_ENDPOINT_SIZE 32

/* The bank mode: 1 = Single Bank, 2 = Double Bank */
#define USB_CFG_OUT_INTR_ENDPOINT_BANK 2

/*----- Interface Class ------
 * Refer to USB specification for existing interface class.
 * 
 * This template is about HID class. If you implement a vendor class device,
 * set USB_CFG_INTERFACE_CLASS to 0 and USB_CFG_DEVICE_CLASS to 0xff.
 */
/* 3 = HID Class */
#define USB_CFG_INTERFACE_CLASS     3
/* In HID Class, 
 * 0 = No subclass. 1 = Boot Interface Subclass */
#define USB_CFG_INTERFACE_SUBCLASS  0   
/* In HID Class, and Boot Interface Subclass
 * 0 = None, 1 = Keyboard, 2 = Mouse */
#define USB_CFG_INTERFACE_PROTOCOL  0

/* HID Report Descriptor Length (Only meaningful if Interface Class = 3)
 *
 * You must also implement usbUserFunctionSetup and handle the HID_REPROT 
 * request, response to the request with the data of the same length.
 */
#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH    42


/*--------------------------------------------------------------------------
   USB Device Descriptor Parameters
--------------------------------------------------------------------------*/

/* Device Class
 * Refer to USB specification for existing device class.
 *
 * This template is about HID class. If you implement a vendor class device,
 * set USB_CFG_INTERFACE_CLASS to 0 and USB_CFG_DEVICE_CLASS to 0xff.
 */
#define USB_CFG_DEVICE_CLASS      0
#define USB_CFG_DEVICE_SUBCLASS   0
#define USB_CFG_DEVICE_PROTOCOL   0

/* USB vendor ID for the device in little endian (low byte first)
 */
//#define  USB_CFG_VENDOR_ID        0x2A, 0x00

/* USB Product ID for the device in little endian
 */
//#define  USB_CFG_PRODUCT_ID       0x00, 0x01

/* Version number of the device. In little endian, in 0xJJMN format - Major, Minor, SubMinor
 */
#define USB_CFG_DEVICE_VERSION    0x00, 0x01 // So this is really 0x0100

/* 0 = Bus Power, 1 = Self Power */
#define USB_CFG_SELF_POWERED        0

/* 0 = No remote wakeup feature, 1 = Capable to do remote wakeup */
#define USB_CFG_REMOTE_WAKEUP       1

/* Max power consumed from the bus in the unit of 2mA */
#define USB_CFG_MAX_POWER           50

// Note: 

/* Vendor Name in UTF-16 and its Character length
 * Comment out the following two lines if 
 *   1) you don't want one, or 
 *   2) you will use USB_CFG_VENDOR_NAME_INDEX.
 */
#define USB_CFG_VENDOR_NAME       'H','e','l','l','o','S','a','m','.','N','e','t',' ','H','a','r','d','w','a','r','e'
#define USB_CFG_VENDOR_NAME_LEN   21
/* If you want to return a dynamically generated name in usbUserFunctionSetup
 * Uncomment the following line, and implement usbUserFunctionSetup such that
 * it will handle the corresponding GET_DESCRIPTOR(STRING) request.
 */
//#define USB_CFG_VENDOR_NAME_INDEX 1

/* Product Name in UTF-16 and its Character length
 * Comment out the following two lines if 
 *   1) you don't want one, or 
 *   2) you will use USB_CFG_PRODUCT_NAME_INDEX.
 */
#define USB_CFG_PRODUCT_NAME      'A','n','s','w','e','r'
#define USB_CFG_PRODUCT_NAME_LEN  6
/* Dynamically generated product name string */
//#define USB_CFG_PRODUCT_NAME_INDEX 2

/* Serial Number in UTF-16 and its Character length
 * Comment out the following two lines if 
 *   1) you don't want one, or 
 *   2) you will use USB_CFG_SERIAL_NUMBER_INDEX.
 */
#define USB_CFG_SERIAL_NUMBER     '4','6','7','0','9','3','9','4'
#define USB_CFG_SERIAL_NUMBER_LEN 8
/* Dynamically generated serial number string */
//#define USB_CFG_SERIAL_NUMBER_INDEX 3


#endif /* __usbconfig_h_included__ */
