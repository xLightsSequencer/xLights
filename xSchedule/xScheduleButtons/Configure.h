// un-comment out this line if you want to see debug messages over the serial connection
//#define DEBUG 1
//#define DEBUGRESPONSE 1

// The serial speed used for debugging messages
#define SERIALRATE 115200

// This is where you define the IP address of the arduino
#define IP_BYTE_1 192
#define IP_BYTE_2 168
#define IP_BYTE_3 0
#define IP_BYTE_4 201

// This pin will be held high when the program is running ... you can connect an LED through a 330 Ohm resistor to show the power is on
#define POWERPIN 11

// This pin will be held high when any button is pressed ... you can connect an LED to provide button press feedback to the user
#define PRESSPIN 13

// This is the IP address of the machine running xSchedule
#define SERVER_IP "192.168.0.103"

// This is the xSchedule port
#define WEBPORT 80

// This is the number of buttons connected
#define BUTTONS 8

// Define the pins to use ... each pin maps to a button named ArduinoButton_n where n is the one based index in the table below
// This must be at least as large as the number of buttons above. Excess pins are ignored
short pins[] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41};

// NOTE: When wiring up the buttons the buttons should short the pin to ground when pressed. When not pressed the pin should be free floating
