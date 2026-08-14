Smart Display GUI Builder<P>

v0.8.4 - 2026/03/24<BR>
New Features:
* Support QR Code widget for SmartDisplay 5" UART
* Supported up to 255 widgets per page for SmartDisplay CustomCAN.
* Host can check the GPIO status when GPIO high/low changed(Custom CAN, not include 32 MB version)

Firmware Bugfixes:
* Fix when refresh Text Buffer, it will send repeat Event message then make Queue overflow. (SmartDisplay RS485, not include 32 MB version)
* Remove default can 250k initial setting and check can rate config as first. (SmartDisplay CANopen)

v0.8.3 - 2025/12/22<BR>
New Features:
* Supported up to 255 widgets per page for SmartDisplay UART.

Firmware Bugfixes:
* Fix property type limit as 20 issue. (SmartDisplay RS485 / CANopen 3.5”, 3.9”, 4.3” and 5”)
* Update share code to latest master git as fix touchgfx 4.16.0 graph drag issue. (SmartDisplay RS485 / CANopen 3.9” and 5”)

v0.8.2 - 2025/11/25<BR>
New Features:
* Supported 7” V2 UART + GPIO (800 x 480)
* Supported Duplicate Widget function
Enhancement:
* Clear the receive buffer when entering Boot mode to prevent the device from sending abnormal data during the boot process, which could cause the upgrade to fail.
Bugfixes:
* Fixed the issue where the originally collapsed Widget Group re-expands when adding a new Widget.
* Set Enter Boot wait time to 2 seconds to prevent update issues.
* Fixed the issue where Modbus failed to export the upgrade file.
* Fixed the issue where abnormal WMI data causes program crash.
* Fixed the issue where the Test device did not update the page index variable, resulting in sending incorrect commands.

v0.8.1 - 2025/09/23<BR>
New Features:
* GUI Builder UI supported multiple languages – English, Traditional Chinese, Japanese
* Supported 7” 32 MB Modbus + GPIO
* Supported 10.1” 32 MB Modbus, UART and Custom CAN + GPIO
* Supported DBC Cycle Time function for 3.5", 3.9", 4.3", 5", 7" and 10.1” Custom CAN + GPIO
* Supported default brightness setting for 3.5", 3.9", 4.3", 5", 7" and 10.1” Custom CAN + GPIO
* Supported delete all images function for Radio Button Editor

v0.8.0 - 2025/06/25<BR>
New Features:
* Supported 7” 32MB UART + GPIO
* Improved UI / UX

v0.7.6 - 2025/05/26<BR>
New Features:
* Supported 7” 32MB Custom CAN + GPIO
* Supported the new UI template, Industry2, for SmartDisplay 7” and 10.1”

v0.7.5 - 2025/03/26<BR>
New Features:
* Support non touch for SmartDisplay 3.5", 3.9", 4.3", 5", 7 and 10.1" UART + GPIO
Bugfixes:
* Fixed the issue where the Page cannot be dragged and adjusted properly.
* Fixed the issue where dragging the 3.5" custom CAN simulator Graph widget causes a crash.

v0.7.4 - 2025/02/25<BR>
New Features:
* Supported 3.9” Custom CAN + GPIO
* Supported 3.9” Modbus + GPIO
* Supported 3.9” UART + GPIO
* Supported clearing keypad value command for 10.1” Modbus
* Supported the function to export upgrade files
Bugfixes:
* Fixed the scroll bar of the Page info that cannot be dragged properly.
* Fixed the issue where resource images cannot be updated properly when the compressed size exceeds 1 MB.


v0.7.3 - 2024/12/25<BR>
New Features:
* Supported keyboard arrow key to move object location.
* Supported when adding a new Action, multiple objects can be selected.
* Supported import / export widget function
* Supported custom date format for Digital Clock Widget.
* Supported Radio Button widget.
* Supported Get / Set Real-time Clock (RTC) command.
* Supported SmartDisplay Custom CAN Id 3.9” device.
* Supported SmartDisplay CANopen / Modbus for 3.5", 4.3", 5" and 7" with GPIO
* Supported non-touch panel for 3.5", 4.3", 5", 7" and 10.1 Custom CAN (with and without GPIO)
Bugfixes:
*	Fixed 5" F429 image resource issue.


v0.7.2 - 2024/10/22<BR>
New Features:
* Supported SmartDisplay UART GPIO for 7”.

v0.7.1 - 2024/09/24<BR>
New Features:
* Supported SmartDisplay UART GPIO for 10.1”, 5” and 3.5”.
* Supported modifying the text widget preview text.
* Enhanced paste function: copy default value and CAN signal settings.
* Enhanced function for pasting- copy and paste to the same page.
* Enhanced function for pasting- copy and paste to different page.
* Changed the maximum value of the animation interval from 1000 ms to 4200 ms.
Bugfixes:
* Fixed the issue where the simulator crashes when the property's space usage exceeds 50%.[* 1]
                
[*1] SmartDisplay UART, CANopen and Modbus.


v0.7.0 - 2024/08/26<BR>
New Features:
* Supported SmartDisplay UART GPIO 4.3” device.
* Supported keypad function. [*1]
* Supported non-touch panel. [*2]
* Supported displaying / hiding version information during boot [*2]
* Supported modifying baud rate and slave ID for Modbus [*3]
* Supported deselect object with pressed Ctrl.
* Supported custom characters settings for fonts.
* Supported new feature for 3.9” CANopen
* Supported drag-and-drop to adjust the order of pages and objects.
Bugfixes:
* Fixed the issue that unplugging USB2CAN in connected state may cause GUI builder to hang.

[*1] SmartDisplay 10.1”, 5”, 4.3” and 3.5” CANopen / Modbus
[*2] SmartDisplay 10.1”, 7”, 5”, 4.3” and 3.5” CANopen / Modbus
[*3] SmartDisplay 10.1, 5”, 4.3” and 3.5” Modbus

v0.6.3 - 2024/06/26<BR>
New Features:
* Supported SmartDisplay Custom CAN GPIO 4.3" device

v0.6.2 - 2024/05/29<BR>
New Features:
* Supported non-touch panel for 7" CANopen / Modbus
* Updated FW file for 3.5" Custom CAN

v0.6.1 - 2024/05/06<BR>
New Features:
* Supported for upgrading the bootloader function [*1]
* Supported for updating the baud rate used by the project to be consistent with the project's settings. [*1]

[*1]: Supported for SmartDisplay Custom CAN  3.5”, 4.3”, 5”, 7” and 10.1"

v0.6.0 - 2024/03/26<BR>
New Features:
* Supported GPIO function [*1]
* Supported new feature for Custom CAN 3.5” and 5”
* Supported command comment for Modbus and Custom CAN log
* Supported new line for Text Widget
* Supported preview text for Text Widget editor.
* Added new parameters for testing Buzzer.
* The upper limit for Widget images has been increased from 64 to 200.
* Updated simulator and FW file for 3.5", 7" and 10.1" CANopen / Modbus.

Bugfixes:
* Updated simulator and FW file for 3.5”, 4.3", 5”,  7" and 10.1" Custom CAN
	- Fixed the issue where the Widget displays abnormal when there is a mixture of CAN Id 11-bit and 29-bit data in the CAN Bus.
* Fixed the issue of text cut off in the DigitalClock widget display.
* Fixed the issue where modifying the Keypad causes a resource generation problem.

[*1]: Supported for SmartDisplay Custom CAN  3.5”, 5” and 7"

v0.5.3 - 2023/12/26<BR>
Bugfixes:
* Fixed the issue of encountering an exception when opening a project that include the new widget.

v0.5.2 - 2023/12/25<BR>
New Features:
* Supported Animated widget editor [*1]
* Supported Circle Progress widget editor [*1]
* Supported Graph widget editor [*1]
* Supported Horizontal Slider widget editor [*1]
* Supported Image Progress widget editor [*1]
* Supported Vertical Slider widget editor [*1]
* Supported Multi State widget editor [*1]
* Supported Digital Clock widget editor [*1]
* Supported alignment for text and number widgets. [*1]
* Supported “Add Widget” function [*1]
* Supported max 30 pages. [*1]
* Supported widget action. [*1]
* Supported DBC multiplex setting [*1]
* Supported Keypad Widget [*1]
* Supported displaying / hiding version information during boot [*1]
* Supported horizontal/vertical space distribution for GUI layout.
* Supported the “Save As” function.
* Supported the "Export Hex File" function that can be programmed onto a SmartDisplay device using STLink.
* Supported displaying the default value of the text/number widget on the editing screen.
Bugfixes:
* Fixed the issue of modifying the background setting in 3.5" CANopen/Modbus.
* Fixed the issue where setting the display orientation to portrait on the 7" CANopen / Modbus.
* Fixed the issue of being unable to open old version projects.

[*1]: Supported for SmartDisplay Custom CAN  4.3”, 7" and 10.1"

v0.5.1 - 2023/11/06<BR>
New Features:
* Supported modifying baud rate and slave ID for 7” Modbus
* Changed the name from Custom Widget to Fixed Widget.
* Changed the maximum value of the animation interval from 100 ms to 1000 ms.
* Supported Keypad Widget for 7” CANopen / Modbus.
Bugfixes:
* Fixed 7" Custom CAN's unintended activation of the "Add background" feature.

v0.5.0 - 2023/08/30<BR>
New Features:
* Supported Animated widget editor [*1]
* Supported Circle Progress widget editor [*1]
* Supported Graph widget editor [*1]
* Supported Horizontal Slider widget editor [*1]
* Supported Image Progress widget editor [*1]
* Supported Vertical Slider widget editor [*1]
* Supported Multi State widget editor [*1]
* Supported Digital Clock widget editor [*1]
* Supported alignment for text and number widgets. [*1]
* Supported “Add Widget” function [*1]
* Supported max 30 pages. [*1]
* Supported widget action. [*1]
* Supported NAND Flash detail information.
Bugfixes:
* Fixed the issue where the Custom CAN simulator was unable to operate when modifying the channel ID settings.
* Fixed the issue where the Custom CAN simulator crashed when a widget set a message but not a signal.

[*1]: Supported for SmartDisplay CANopen / Modbus  3.5", 4.3”, 5", 7" and 10.1" 

v0.4.6 - 2023/07/28<BR>
Bugfixes<BR>
* Fixed the issue of being unable to parse DBC decimal settings when the computer language is set to a European language.

v0.4.5 - 2023/06/08<BR>
Bugfixes<BR>
* Fixed the RS485 upgrade issue.

v0.4.4 - 2023/06/01<BR>
Bugfixes<BR>
* Hid the change value action function for the 10.1 Custom CAN.

v0.4.3 - 2023/05/12<BR>
New Features<BR>
* Supported USB2CAN add-on package function
Bugfixes<BR>
* Fixed Custom CAN simulator crash issue when over 32 objects

v0.4.2 - 2023/04/10<BR>
New Features<BR>
* Supported Custom CAN id protocol for SmartDisplay 3.5", 5", 7" and 10.1"
* Supported widget action and GPIO function for 10.1" Custom CAN
* Supported import and export project function

v0.4.1 - 2022/11/22<BR>
Bugfixes<BR>
* Fixed Custom CAN changing baud rate issue
* Fixed Custom CAN simulator not displaying GIF issue

v0.4.0 - 2022/11/08<BR>
New Features<BR>
* Supported custom CAN id protocol for SmartDisplay 4.3"
* Supported multiple language font Unicode range setting

v0.3.11 - 2022/10/18<BR>
Bugfixes<BR>
* Fixed crash issue when updating location
* Hide unavailable FTDI USB2RS485 COM port
Deprecated Features<BR>
* Removed Upload Page function
* Removed upgrade firmware type: Page Info and Project Binary File

v0.3.10 - 2022/09/19<BR>
New Features<BR>
* Added UI template for SmartDisplay 5” HB (STM32F750)
* Device type sorted by screen size
* Added device information in the device page

v0.3.9 - 2022/08/12<BR>
New Features<BR>
* Supported SmartDisplay 5” (F750)

v0.3.8 - 2022/07/14<BR>
New Features<BR>
* Supported CANopen 1000 kbps for SmartDisplay 4.3”, 5” and 7”

v0.3.7 - 2022/06/01<BR>
Bugfixes<BR>
* Fixed the issue where font data could not be generated when the GUI Builder installation drive was different from the project drive.
* Fixed SmartDisplay 5” firmware hang issue in older version bootloader

v0.3.6 - 2022/05/05<BR>
New Features<BR>
* Supported Modbus for SmartDisplay 10.1”
Bugfixes<BR>
* Fixed simulator crash issue when using high resolution GIF on SmartDisplay 4.3”, 5”, 7” and 10.1”
* Fixed SmartDisplay 7” Simulator crash issue when the installation folder was too short.

v0.3.5 - 2022/04/26<BR>
New Features<BR>
* Automatically upload resources after upgrading firmware when uploading the project

v0.3.4 - 2022/04/25<BR>
New Features<BR>
* Supported 64 objects, custom fonts , custom Text / Number color , splash screen GIF , advanced Gauge property and portrait mode for SmartDisplay 5”

v0.3.3 - 2022/04/14<BR>
New Features<BR>
* Supported up to 64 objects on one page and enable / disable buzzer for SmartDisplay OLED 3.55”
* Supported 7 indicator widgets for SmartDisplay 7” and 10.1”

Bugfixes<BR>
* Fixed simulator crash issue when changing config mode for SmartDisplay 10.1"

v0.3.2 - 2022/03/31<BR>
Bugfixes<BR>
* Fixed resource image format issue for SmartDisplay OLED 3.55"
* If the device does not support advanced property settings, no need to check the properties feature.

v0.3.1 - 2022/03/29<BR>
Bugfixes<BR>
* Updated firmware file for SmartDisplay 3.5", 3.9", 4.3", 10.1" (fix buzzer issue)

v0.3.0 - 2022/03/25<BR>
New Features<BR>
* Supported up to 64 objects on one page [*1]
* Supported custom fonts [*1]
* Supported custom Text / Number color [*1]
* Supported splash screen GIF [*1]
* Supported advanced Gauge property setting [*1]
* Supported portrait mode [*2]

[*1]: Supported for SmartDisplay 3.5", 3.9", 4.3", 7" and 10.1"<BR>
[*2]: Supported for SmartDisplay 3.5", 4.3", 7" and 10.1"<BR>

Bugfixes<BR>
* Added non-ASCII code checking rule for project folder and name.

v0.2.6 - 2022/01/25<BR>
New Features<BR>
* Supported SmartDisplay OLED 3.55"

v0.2.5 - 2021/12/17<BR>
New Features<BR>
* Supported digital clock for SmartDisplay 5", 4.3", 3.9" and 3.5"
* Supported battery for SmartDisplay 3.9", 4.3" and 3.5"
* Supported white/black text color text for SmartDisplay 5", 3.5" and 4.3"
* Supported MuitlState widget for SmartDisplay 3.5", 5", 3.9", 4.3" and 7"
* Rearrange the device type list in the new project window
* Supported independent toggle button image for 5", 7" and 10.1"
* Supported SmartDisplay 10.1"
* Supported buzzer test function

Bugfixes<BR>
* Fixed the issue that the software version is not reloaded after upgrading the firmware.
* Fixed the issue that the old project is abnormal when adding a new widget type

v0.2.4 - 2021/10/27<BR>
Bugfixes<BR>
* Fixed the bug of SmartDisplay 3.9" simulator display background 0
* Fixed some typos in the description of the file filter

v0.2.3 - 2021/10/22<BR>
New Features<BR>
* Supported brightness adjustment function

Bugfixes<BR>
* Fixed the bug of SmartDisplay 3.5" changing the circular progress image

v0.2.2 - 2021/10/13<BR>
New Features<BR>
* Added clock widget for SmartDisplay 7"
* Supported Modbus simulator for SmartDisplay 7", 4.3" and 3.5"

Updated<BR>
* Removed request for administrator permission

Bugfixes<BR>
* Fixed vehicle template setting for SmartDisplay 3.9" 

v0.2.1 - 2021/08/31<BR>
New Features<BR>
* Added new widget gauge 2~6 for SmartDisplay 4.3"
* Added 3 white color number and text widget for SmartDisplay 7"

Updated<BR>
* Updated Gauge 5 range between 0 and 160 for SmartDisplay 7" and 5"

Bugfixes<BR>
* Fixed Modbus transport not sending default value when testing device screen 
* Fixed Modbus transport not reading default text buffer data

v0.2.0 - 2021/08/02<BR>
New Features<BR>
* Supported Modbus RTU with RS485 for SmartDisplay series

Bugfixes<BR>
* Fixed connection not disconnected after opening a new project

v0.1.9 - 2021/07/21<BR>
New Features<BR>
* Supported SmartDisplay 3.5"
* Supported clock setting function for SmartDisplay 4.3" simulator

v0.1.8 - 2021/07/14<BR>
New Features<BR>
* Supported SmartDisplay 4.3"

v0.1.7 - 2021/02/19<BR>
New Features<BR>
* Supported SmartDisplay 3.9"
* Supported Text widget
* Updated simulator function for SmartDisplay 3.9" , 5" and 7"

v0.1.6 - 2021/01/21<BR>
New Features<BR>
* Supported Simulator 5" for firmware v00.00.11
* Maximized the "Test Device window" screen to avoid small screen resolution, which will block the title bar

Bugfixes<BR>
* Fixed typos
* Increased waiting time to avoid simulator timeout
* Fixed the issue that the default value is not stored normally

v0.1.5 - 2021/01/06<BR>
New Features<BR>
* Added the open project function in the new project window

Bugfixes<BR>
* Fixed undo/redo commands not cleared after opening a new project
* Fixed page information binary file format problem
* Fixed the resource file cannot be generated when the project name contains space characters
* Fixed the SmartDisplay 7" simulator cannot update the background 0

v0.1.4 - 2020/12/18<BR>
Update<BR>
* Change start logo image

Fix<BR>
* Fix Windows 10 permission issue
* Fix test device windows default value issue

v0.1.3 - 2020/12/09<BR>
Update<BR>
* Support checking device type for firmware upgrade
* Change logo image
* Change SmartDisplay 7" color format form RGB888 to RGB565

Fix<BR>
* Fix the GUI crash when the size of the application bin file is less than 544

v0.1.2 - 2020/12/02<BR>
Update<BR>
* Support image dithering for background 0 and start logo image
* Add validation error message for Number String input

v0.1.1 - 2020/11/25<BR>
Update
* support CANOpen Log
* Use SDO command to change and receive values
* Use SDO command to change config mode
* Support device configuration function to modify baudrate , Node id and SDO Address
* Support UI Template

v0.0.3 - 2020/11/02<BR>
Update<BR>
* Support upload pageage info function
* support control tool tip
* Support delete context menu for page
* Support delete / Send backward / Bring forward context menu for control
* Simulator support CANopen function

Fix<BR>
* Fix DPI issue
* Fix invalid project file name issue

v0.0.2 - 2020/10/22<BR>
Update<BR>
* Support Simulator Function
* add resource image size check rule!
* add Splash log
* add version for About
* add Number function

v0.0.1
* Init Version
	
	