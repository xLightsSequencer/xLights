// You should never need to touch this code ... all configuration should be done in Configure.h

#include "Configure.h"
#include <UIPEthernet.h>

byte mac[6] = {0x00, 0x01, 0x02, 0x03, 0x05, IP_BYTE_4}; // the arduinos MAC address
IPAddress ip(IP_BYTE_1,IP_BYTE_2,IP_BYTE_3,IP_BYTE_4); // the arduinos IP address
short pressed[100]; // an array keeping state of the buttons between loops
short buttons; // number of buttons

void setup() 
{
  // Prepare to send debug messages
  #ifdef DEBUG
    Serial.begin(SERIALRATE);
    Serial.println("Starting");
  #endif

  // work out the number of buttons
  buttons = BUTTONS;
  if (buttons > sizeof(pressed) / sizeof(short))
  {
    buttons = sizeof(pressed) / sizeof(short);
    #ifdef DEBUG
      Serial.print("Buttons limited to ");
      Serial.println(buttons);
    #endif
  }

  // setup the power pin
  pinMode(POWERPIN, OUTPUT);
  digitalWrite(POWERPIN, HIGH);

  // set up the button pressed pin
  pinMode(PRESSPIN, OUTPUT);

  // start ethernet
  Ethernet.begin(mac, ip);

  // give it time to connect
  delay(200);

  #ifdef DEBUG
  // check everything looks ok
  if (Ethernet.localIP() != ip)
  {
    Serial.println("ERROR: IP address did not take.");
  }

  Serial.println(Ethernet.localIP());
  #endif

  // setup our input pins
  for (int i = 0; i < buttons; i++)
  {
    pinMode(pins[i], INPUT_PULLUP);
    pressed[i] = HIGH;
  }
}

// Send the request to xSchedule
void WebRequest(short button)
{
  #ifdef DEBUG
    Serial.print("Connecting to "); 
    Serial.print(SERVER_IP); 
    Serial.print(":"); 
    Serial.print(WEBPORT); 
  #endif
  
  EthernetClient client; // the ethernet connection
  client.setTimeout(200);
  if (client.connect(SERVER_IP, WEBPORT)) 
  {
    char inChar;
    char bs[20];
    memset(bs, 0x00, sizeof(bs));
    sprintf(bs, "%d", button);

    #ifdef DEBUG
      Serial.println("-> Connected"); 
      Serial.print( "GET /xScheduleCommand?Command=PressButton&Parameters=HIDE_ArduinoButton_");
      Serial.print(bs);
      Serial.println( " HTTP/1.1");
    #endif

    client.print( "GET /xScheduleCommand?Command=PressButton&Parameters=HIDE_ArduinoButton_");
    client.print(bs);
    client.println( " HTTP/1.1");
    client.print( "Host: " );
    client.println(SERVER_IP);
    client.println( "Connection: close\r\n" );

    #ifdef DEBUG
      Serial.println("Sent"); 
    #endif
    
    int connectLoop = 0;
    while (client.connected() && connectLoop < 1000)
    {
       while (client.available())
       {
          inChar = client.read();
          #ifdef DEBUGRESPONSE
             Serial.write(inChar);
          #endif
          client.stop();
       }

       if (client.connected())
       {
           connectLoop++;
           // this is a delay for the connectLoop timing
           delay(1);
       }
    }
    
    #ifdef DEBUGRESPONSE
       Serial.println();
    #endif
    #ifdef DEBUG
      Serial.println("Response read"); 
    #endif
    
    client.stop();
  }
  else 
  {
    // you didn't get a connection to the server:
    #ifdef DEBUG
       Serial.println("--> connection failed/n");
    #endif
  }
}

void loop() 
{
  bool ispressed = false; // keep track if any button is pressed so i know if to light the pressed pin

  // check each button
  for (int i = 0; i < BUTTONS; i++)
  {
    // read it
    short v = digitalRead(pins[i]);

    // if it is pressed immediately light the led
    if (v == LOW)
    {
      digitalWrite(PRESSPIN, HIGH);
      ispressed = true;
    }

    // if the button has changed state
    if (pressed[i] != v)
    {
      #ifdef DEBUG
        Serial.print("Pin changed state: ");
        Serial.print(pins[i]);
        Serial.print(" : ");
        Serial.println(v);
      #endif
      
      if (v == LOW)
      {
        // button is newly pressed
        WebRequest(i+1);
      }

      pressed[i] = v;
    }
  }

  // if no button is pressed clear the pressed LED
  if (!ispressed)
  {
    digitalWrite(PRESSPIN, LOW);
  }

  // a small delay helps deal with key bounce
  delay(10);
}