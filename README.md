Pop'n Convertible ASC (Arcade Style Controller)
==============================================

[Pop'n Music][popnwiki] is a music simulation game produced by KONAMI.

This project is about making an arcade like, portable controller for the Pop'n Music game. The controller is designed to be connected to PC through USB. The buon push emulates keyboard key hits. (How to obtain the game to run on PC is out of topic)

Visit the [album][] to see some action shots.

Contents
--------
The files here are contains all of the design - hardware and software.

 * Mechanical Design
   A convertible stand design for holding the big buttons, designed to be made from acrylic (Plexiglas) and laser cut
 * Firmware
   An AVR Firmware to be loaded into any [LUFA][] supported device, to translate button pushes into keyboard keystrokes. It is a Keyboard HID and requires no driver for most OS.

Firmware Hardware
-----------------
The firmware is to translate button pushes as USB keyboard key press for PC. If you want to hook this up to Playstation, you don't need this firmware. You should deassemble a PS controller and hook it up instead.

The firmware can be compile for any [LUFA][] supported dev board, such as [minimus][]. Please refer to the LUFA website for supported models. Dev boards are usually sold at around 10 to 20 USD.

It is to be compiled with GCC + avr-libc. [WinAVR][] is such a toolchain for Windows. Upon connect, it requires no driver operate as it works as a keyboard device. 

Electronics
-----------
The buttons are to Port B pin 0 to pin 7, and Port C pin 7 from left to right.

On the microswitch: connect the Normal Open lead to the MCU, common lead to the GND.
On the button LED: connect positive lead to Vcc, negative lead to GND. This assume the LED component has a built-in resistor inside (Those I bought does include a 300-ohm connected in series). The Vcc could be powered by 9V to 12V, as well as from the USB 5V directly except that it might not be bright enough.

Each button push would generate a key press signal of 1 to 9 accordingly.

Action shot
-----------
See [album]

Building Cost
-------------
Acrylic parts material + laser cutting: 30 USD
Minimus USB dev board: 10 USD
Buttons (LED + Microswitch included): 25 USD
Being able to play Pop'n Music with authentic controller: Priceless

Wires, soldering tools, etc. are not included.

[popnwiki]: http://en.wikipedia.org/wiki/Pop'n_Music
[album]: https://picasaweb.google.com/103994691685219285374/PopnConvertibleAsc
[LUFA]: http://www.fourwalledcubicle.com/LUFA.php "Lightweight USB Framework for AVRs"
[minimus]: http://minimususb.com/
[WinAvr]: http://winavr.sourceforge.net/
