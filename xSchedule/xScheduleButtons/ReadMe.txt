To use this arduino code you need an arduino. I built it with a Mega but it could be adapted to work with others by changing the pin numbers.

I also used an ENC28J60 and installed the UIPEthernet library which can be found here: https://github.com/ntruchsess/arduino_uip

Minimum wiring for 8 buttons using Configure.h as supplied is as follows.

Arduino Mega, ENC28J60.

Mega Pin	to

GND		each button
GND		each LED
GND		ENC28J60 GND

+5V		ENC28J60 5V
	or
+3.3V		ENC28J60 3.3V

50		ENC28J60 SO
51		ENC28J60 ST (SI?)
52		ENC28J60 SCK
53		ENC28J60 CS

11		through 330 Ohm resistor to LED
13		through 330 Ohm resistor to LED

22		button 1 -> triggers xSchedule button HIDE_ArduinoButton_1
23		button 2 -> triggers xSchedule button HIDE_ArduinoButton_2
24		button 3 -> triggers xSchedule button HIDE_ArduinoButton_3
25		button 4 -> triggers xSchedule button HIDE_ArduinoButton_4
26		button 5 -> triggers xSchedule button HIDE_ArduinoButton_5
27		button 6 -> triggers xSchedule button HIDE_ArduinoButton_6
28		button 7 -> triggers xSchedule button HIDE_ArduinoButton_7
29		button 8 -> triggers xSchedule button HIDE_ArduinoButton_8

Define the required buttons to decide what each does in the xSchedule options.
