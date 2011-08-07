Pop'n Convertible ASC (Arcade Style Controller)
==============================================
中文版README請往下看。

[Pop'n Music][popnwiki] is a music simulation game produced by KONAMI.

This project is about making an arcade like, portable controller for the Pop'n Music game. The controller is designed to be connected to PC through USB. The buon push emulates keyboard key hits. (How to obtain the game to run on PC is out of topic)

Visit the [album][] to see some action shots.

Contents
--------
The files here are contains all of the design - hardware and software.

 * Mechanical Design
   A convertible stand design for holding the big buttons, designed to be made from acrylic (Plexiglas) and laser cut. This contains all the design source file and output.
 * Firmware
   Source code of the AVR Firmware to be loaded into any [LUFA][] supported device, to translate button pushes into keyboard keystrokes. It is a Keyboard HID and requires no driver for most OS.

Firmware Hardware
-----------------
The firmware is to translate button pushes as USB keyboard key press for PC. If you want to hook this up to Playstation, you don't need this firmware. You should deassemble a PS controller and hook it up instead.

The firmware can be compile for any [LUFA][] supported dev board, such as [minimus][]. Please refer to the LUFA website for supported models. Dev boards are usually sold at around 10 to 20 USD.

It is to be compiled with GCC + avr-libc. [WinAVR][] is such a toolchain for Windows. Upon connect, it requires no driver operate as it works as a keyboard device. 

Electronics
-----------
The buttons are to Port B pin 0 to pin 7, and Port C pin 7 from left to right.

 * On the microswitch: connect the Normal Open lead to the MCU, common lead to the GND.
 * On the button LED: connect positive lead to Vcc, negative lead to Normal Open of the microswitch. This assume the LED component has a built-in resistor inside (Those I bought does include a 300-ohm connected in series). The Vcc could be powered by 9V to 12V, as well as from the USB 5V directly except that it might not be bright enough.

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

Pop'n 可分拆式街機控制盤
========================
[Pop'n Music][popnwiki]是一套由KONAMI發行的音樂模擬遊戲。

這個專案是關於制作一個可與街機大小相比，但相對方便收納的控制器。控制器是以USB方式與電腦連接，按鍵會模擬成鍵盤按鈕一樣。(至於怎樣取得PC上運行的Pop'n Music版本則是題外話)

成品照片請看[相冊][album]。

內容
----
這裏提供了整件成品的所有文檔 - 硬件和軟件。

 * Mechanical Design - 硬件機械設計
   一個可分拆的腳架，當然是用來固定按鈕用的。設計上是以亞克力 (又名亞加力、有機玻璃)為材，以激光切割製成。這有了所有的設計原文件和輸出圖檔。
 * Firmware
   AVR固件的原代碼，適用於所有支持[LUFA][]架構的AVR開發版，用以將按鍵訊號轉成USB鍵盤訊號給電腦。電腦將會把它看成一般鍵盤硬件一樣，無需安裝任何驅動即可使用。

固件所用之硬件
--------------
這裏的固件是用來把按鍵訊號轉成USB鍵盤訊號給電腦。如果你要將這些按鈕接到Playstation的話，你不需要固件，也不需要AVR開發版，取而代之是你要肢解一個Playstation手掣並把線接上去。

本固件適用於所有支持[LUFA]的AVR硬件，例如[minimus]。有關硬件支持請參考LUFA的網頁。一般來說價格在於10到20美元之間。

本固件需要用GCC和avr-libc編譯才可使用。Windows用家可考慮使用[WivAVR][]來解決。硬件連上USB後，由於會模擬成鍵盤，所以電腦不需要配備任何驅動。

電子
----
那9個鍵由左至右應接到AVR B0到B7以及C7腳。

 * 微動開關方面: 常開(Normal Open - NO)接到AVR的腳。COM接地 (GND)。
 * LED方面:正極接Vcc，負極接到微動的常開腳。這假設你的LED模組中包含一個相應的電阻。(我買的有一個300歐電阻串連著) Vcc可以是其他9V-12V電源，USB的也行不過有點暗。

每按下按鈕就等同按了鍵盤的1-9一樣。

成品
----
看[圖][album]

製作成本
--------
亞克力材料及激光切割手工: 180人民幣
Minimus USB dev board: 60人民幣
特大按鈕，連微動和LED: 170人民幣
在家尤如在機舖中玩Pop'n: 此刻無賈

電線、焊接工具等不計算在內。

[popnwiki]: http://en.wikipedia.org/wiki/Pop'n_Music
[album]: https://picasaweb.google.com/103994691685219285374/PopnConvertibleAsc
[LUFA]: http://www.fourwalledcubicle.com/LUFA.php "Lightweight USB Framework for AVRs"
[minimus]: http://minimususb.com/
[WinAvr]: http://winavr.sourceforge.net/
