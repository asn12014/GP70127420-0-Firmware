About: GridPulser70127420-0-Firmware (formerly known as test_sketch_20_08_04) is an Arduino sketch written for the firmware on the DNDO gun pulser (701-274-20) which utilizes a ATMEGA168 microcontroller. It handles communication over UART to control the grid pulser and filament power supply output voltages, and ON/OFF states. 

Note: The off-state on the grid pulser does not disable pulse capabilities, but instead merely limits the output voltage to a ~5V minimum.

Original author: M. Kemp

Changelog 
2023-01-11 - R1-v0.1.0 - A. S. Nguyen
	VERSION NO LONGER COMPATIBLE WITH R0 BOARDS!

	Watchdog timer re-implementation
	- Outputs are disabled and set points are reset to minimums if input data is not received for 10s by checking the new resetIfIdle() function in the loop routine. This code was present in the 22-02-08 test sketch loaded onto r0 hardware, but was never ported into git controlled code due to oversight.

	Start-up load & start-up load bypass implemented 
	- A 2 ohm, 10W equivalent resistor has been placed on the filament supply output to limit in-rush current to the filament at start-up. 
	- Digital pin 5 (PD5) now connects to a solid-state relay. A HIGH input to this relay will bypass the slow-start load resistor and allow for un-impeded current flow.
		- Bypass state is controlled by input from MATLAB program. Additional flag is required on serial input string.

	- CS_2 (chip select for grid voltage control rheostat) has been moved to digital pin 6 (PD6) now that start-up bypass is on digital pin 5 (PD5).

2022-03-14 - R0-v1.0.0 - A. S. Nguyen
	- Renamed program from test_sketch_20_08_04.ino to GridPulser70127420-0-FirmwareV1.0.ino for clarity.

2021-08-25 - A. S. Nguyen
	- Added a line to explicitly initialize the RX pin (0) as an input to stop this pin from sourcing current for whatever reason... 