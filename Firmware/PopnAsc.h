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

#ifndef _POPNASC_H_
#define _POPNASC_H_

#include "Descriptors.h"

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);
void EVENT_USB_Device_StartOfFrame(void);
void EVENT_USB_Device_Suspend(void);
void EVENT_USB_Device_WakeUp(void);

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize);
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize);

#endif

