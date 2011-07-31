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

Version: $Id: leds.h 6 2008-04-12 18:08:14Z sam $

*/

#ifndef __BUTTON_H__
#define __BUTTON_H__

/**
 * Initialize the Button functionalities
 */
void buttonInit(void);

/**
 * Check the buttons and send report if necessary
 * Must be called periodically when USB is ready
 */
void buttonLoop(void);

#endif
