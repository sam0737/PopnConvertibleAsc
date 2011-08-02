Popn Convertible ASC (Arcade Style Controller)
==============================================

[Pop'n Music][popnwiki] is a music simulation game produced by KONAMI.

This project is about making a portable game controller for such game. The controller is then connected to USB and emits keystrokes according to the button pushes.

Contents
--------
 * Mechanical Design
   A convertible stand design for holding the big buttons, designed to be made from acrylic (Plexiglas) and laser cut
 * Firmware
   An AVR Firmware to be loaded into any [LUFA][] supported device, to translate button pushes into keyboard keystrokes. It is a Keyboard HID and requires no driver for most OS.

[popnwiki]: http://en.wikipedia.org/wiki/Pop'n_Music
[LUFA]: http://www.fourwalledcubicle.com/LUFA.php "Lightweight USB Framework for AVRs"

