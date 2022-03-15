About: GridPulser70127420-0-Firmware (formerly known as test_sketch_20_08_04) is an Arduino sketch written for the firmware on the DNDO gun pulser (701-274-20 R0) which utilizes a ATMEGA168 microcontroller. It handles communication over UART to control the grid pulser and filament power supply output voltages, and ON/OFF states. 

Note: The off-state on the grid pulser does not disable pulse capabilities, but instead merely limits the output voltage to a ~5V minimum.

Original author: M. Kemp

Changelog 
2022-03-14 - V1.0 - A. S. Nguyen
	- Renamed program from test_sketch_20_08_04.ino to GridPulser70127420-0-FirmwareV1.0.ino for clarity.

2021-08-25 - A. S. Nguyen
	- Added a line to explicitly initialize the RX pin (0) as an input to stop this pin from sourcing current for whatever reason... 