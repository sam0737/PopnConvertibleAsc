/*

USBAVRUSB - USB device driver for AVR USB's controller

Copyright (c) 2008 Sam Wong http://hellosam.net

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

Version: $Id: usbdrv_inline.c 3 2008-04-12 17:58:30Z sam $

*/


#ifndef __USBDRV_INLINE_C__
#define __USBDRV_INLINE_C__

static inline unsigned char usbIsUSBReady(void)
{
	return _usb_configuration_index != 0 && (USBCON & (1<<USBE));
}

static inline unsigned char usbCurrentConfiguration(void)
{
	return _usb_configuration_index;
}

static inline unsigned char usbIsEndPointReadable(unsigned char endpoint)
{
	UENUM = endpoint;
	return (UEINTX & (1<<FIFOCON));
}

static inline unsigned char usbIsEndPointWritable(unsigned char endpoint)
{
	UENUM = endpoint;
	return (UEINTX & (1<<FIFOCON));
}

static inline void usbSelectEndpoint(unsigned char endpoint)
{
	UENUM = endpoint;
}

static inline unsigned char usbByteAvailable(void)
{
	return UEBCLX;
}

static inline void usbWriteData(unsigned char data)
{
	UEDATX = data;
}

static inline unsigned char usbReadData(void)
{
	return UEDATX;
}

static inline void usbEndWrite(void)
{
	UEINTX &= ~(1<<TXINI);
	UEINTX &= ~(1<<FIFOCON);
}

static inline void usbEndRead(void)
{
	UEINTX &= ~(1<<RXOUTI);
	UEINTX &= ~(1<<FIFOCON);
}

static inline unsigned char usbIsInSuspend(void)
{
	return UDINT & (1<<SUSPI);
}

static inline unsigned char usbIsRemoteWakeupAllowed(void)
{
	return _usb_device_feature & USBFEATURE_DEVICE_REMOTE_WAKEUP;
}

static inline void usbRemoteWakeup(void)
{
	UDCON |= (1<<RMWKUP);
}

#endif
