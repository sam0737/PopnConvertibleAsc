/*

Button - Button change detection and report

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

Version: $Id: leds.c 6 2008-04-12 18:08:14Z sam $

*/

#include "button.h"
#include "usbconfig.h"
#include "usbavrusb/usbdrv.h"

#include <avr/io.h>
#include <avr/interrupt.h>

unsigned char timeMark;

unsigned char lastTime[9];
unsigned short buttonState;
unsigned short buttonDebounce;

void buttonInit()
{
	// Set PD7 for Button Polling
	DDRD &= ~(1<<7);
	PORTD |= (1<<7);
}

void usbUserSOFEvent()
{
	unsigned char i;	
	unsigned short currentState;
	short change;
	
	timeMark++;
	
	if (PIND & (1<<7))
	{
		currentState = (unsigned short) 0x0000;
	} else {
		currentState = (unsigned short) 0x01FF;
	}
	
	buttonState = (buttonState & buttonDebounce) | (~buttonDebounce & currentState);
	change = buttonState ^ currentState;
	
	for (i = 0; i < 9; i++)
	{
		if (buttonDebounce & (1 << i))
		{
			unsigned char diff = timeMark - lastTime[i];
			if (diff > 10)
			{
				buttonDebounce &= ~(1 << i);
			}
		} else {
			if (change & (1 << i)) {
				lastTime[i] = timeMark;
				buttonDebounce |= (1 << i);
			}
		}
	}
}

void buttonLoop()
{
	if (usbIsEndPointWritable(USB_CFG_IN_ENDPOINT_NO))
	{
		usbWriteData(buttonState & 0xFF);
		usbWriteData((buttonState >> 8) & 0xFF);
		usbEndWrite();
	}
}
