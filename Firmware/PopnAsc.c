/*
  Pop'n Convertible Arcade Style Controller Project
 
  Copyright 2011 Sam Wong (Sam /@/ hellosam /./ net)

  This work is licensed under the Creative Commons Attribution 3.0 
  Hong Kong License. 

  To view a copy of this license, visit 
  http://creativecommons.org/licenses/by/3.0/hk/ or send a letter to 
  Creative Commons, 444 Castro Street, Suite 900, 
  Mountain View, California, 94041, USA.

  --
  Code is based on the following LUFA Demo project
    [Demos\Device\ClassDriver\Keyboard]:

  Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.

*/

#include "PopnAsc.h"

#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/Board/Buttons.h>
#include <LUFA/Drivers/USB/USB.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

/// Button edge changes are ignored for how many milliseconds?
#define DEBOUNCE_TIME 35

/// Current timestamp (millisecond)
unsigned char timeMark;

/// Button status management: When was the button pushed?
unsigned char lastTime[9];
/// Button status management: [Bitmap] The current button states (active high)
unsigned short buttonState;
/// Button status management: [Bitmap] If a button is in debounce delay phase, during which button state changes will be ignored
unsigned short buttonDebounce;

/** LUFA HID Class driver interface configuration and state information. */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
 	{
		.Config =
			{
				.InterfaceNumber              = 0,

				.ReportINEndpointNumber       = DEVICE_ENDPOINT_NUM,
				.ReportINEndpointSize         = DEVICE_ENDPOINT_SIZE,
				.ReportINEndpointDoubleBank   = false,

				.PrevReportINBuffer           = NULL,
				.PrevReportINBufferSize       = DEVICE_ENDPOINT_SIZE,
			},
    };


void init_hardware(void);
void Popn_Buttons_Init(void);
void CalculateButtonState(void);

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	init_hardware();

	sei();

	for (;;)
	{
        wdt_reset();
		HID_Device_USBTask(&Keyboard_HID_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void init_hardware(void)
{
  	// Turn on Watchdog
	wdt_enable(WDTO_500MS);

	// Disable clock prescaler
	clock_prescale_set(clock_div_1);

	// Initialize other driver
	LEDs_Init();
	Buttons_Init();
    Popn_Buttons_Init();
	USB_Init();
}

/** Initialize Pop'n buttons */
void Popn_Buttons_Init(void)
{
    // Buttons are connected to PB0 to PB7 and PC7
	DDRB  = 0;
	PORTB = 0xFF;
	
	DDRC  &= ~_BV(7);
	PORTC |=  _BV(7);
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_TurnOffLEDs(LEDS_LED1);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_TurnOnLEDs(LEDS_LED1);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);

	USB_Device_EnableSOFEvents();

	LEDs_TurnOnLEDs(LEDS_LED1);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
    CalculateButtonState();
}

void EVENT_USB_Device_Suspend()
{
	wdt_disable();
}

void EVENT_USB_Device_WakeUp()
{
	wdt_enable(WDTO_500MS);
}

void CalculateButtonState(void)
{
	unsigned char i;	
	unsigned short currentState;
	short change;
	
	timeMark++;
	
    // If any debug button is presed
	if (Buttons_GetStatus() != 0) 
	{
		// Assume all key down
		currentState = (unsigned short) 0x01FF;
	} else {
		// Read actual push button 
        // (Push button are active low, so remember to flip them)
		currentState = (unsigned short) ~(((PINC & _BV(7)) << 1) | PINB);
	}
	
    // Use old state if still in debounce interval, or else new state
	buttonState = (buttonState & buttonDebounce) | (~buttonDebounce & currentState);

    // Was the state changed?
	change = buttonState ^ currentState;
	
	for (i = 0; i < 9; i++)
	{
		if (buttonDebounce & _BV(i))
		{
            // Release debounce state if interval has passed
			unsigned char diff = timeMark - lastTime[i];
			if (diff > DEBOUNCE_TIME)
			{
				buttonDebounce &= ~_BV(i);
			}
		} else {
            // If change, kick into debounce state
			if (change & _BV(i)) {
				lastTime[i] = timeMark;
				buttonDebounce |= _BV(i);
			}
		}
	}
}

/** HID IN report */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo, uint8_t* const ReportID,
                                         const uint8_t ReportType, void* ReportData, uint16_t* const ReportSize)
{
    uint8_t* data = (uint8_t*) ReportData;

    data[0] = buttonState & 0xFF;
    data[1] = (buttonState >> 8) & 0xFF;

    *ReportSize = 2;

	return true;
}

/** HID OUT report */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
    // No-op
}

