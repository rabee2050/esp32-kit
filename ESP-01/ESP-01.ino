/*
  WiFiEsp example: WebServer

  A simple web server that shows the value of the analog input
  pins via a web page using an ESP8266 module.
  This sketch will print the IP address of your ESP8266 module (once connected)
  to the Serial monitor. From there, you can open that address in a web browser
  to display the web page.
  The web page will be automatically refreshed each 20 seconds.

  For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html

  SendCommand("AT+RST", "Ready");
  delay(5000);
  SendCommand("AT+UART_DEF=9600,8,1,0,0", "OK");
  delay(2000);
  SendCommand("AT+CWMODE=3", "OK");
  SendCommand("AT+CWJAP=\"Mi rabee\",\"1231231234\"", "OK");
  delay(2000);
  SendCommand("AT+CIFSR", "OK");
  SendCommand("AT+CIPMUX=1", "OK");
  SendCommand("AT+CIPSERVER=1,80", "OK");
  
*/

#include "WiFiEsp.h"
// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(7, 6); // RX, TX
#endif

char ssid[] = "Mi rabee";            // your network SSID (name)
char pass[] = "1231231234";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
int reqCount = 0;                // number of requests received

#include <Servo.h>
#define lcd_size 3 //this will define number of LCD on the phone app
char mode_action[54];
int mode_val[54];
String lcd[lcd_size];
String api, channel, notification;
Servo myServo[53];
unsigned long last = millis();
#define TIMEOUT 100 // mS

WiFiEspServer server(80);

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println(F("You're connected to the network"));
  printWifiStatus();

  // start the web server on port 80
  server.begin();
  

  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == 18 || i == 19) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
  for (byte i = A0; i <= A15; i++) {
    pinMode(i, INPUT);
  }

#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1 || i == 6 || i == 7) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
  for (byte i = A0; i <= A5; i++) {
    pinMode(i, INPUT);
  }
#endif
  

  
}

void update_input() {
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);
    }
  }
}

void loop()
{

  lcd[0] = "Test 1 LCD";// you can send any data to your mobile phone.
  lcd[1] = analogRead(0);// you send analog value of A1
  lcd[2] = "Test 2 LCD";// here you send the battery status if you are using Adafruit BLE


  WiFiEspClient client = server.available();
  if (client) {
    process(client);
    client.stop();
    client.flush();
  }
  delay(50);
  update_input();
  // listen for incoming clients

  if (Serial.available()) {
    String data = Serial.readStringUntil('\r');
    SendCommand(data, "OK");
  }



}

void process(WiFiEspClient client) {
  String _get = client.readStringUntil('/');
  String command = client.readStringUntil('/');
  if (command == "digital") {
    digitalCommand(client);
  }

  if (command == "analog") {
    analogCommand(client);
  }

  if (command == "servo") {
    servo(client);
  }

  if (command == "mode") {
    modeCommand(client);
  }

  if (command == "allonoff") {
    allonoff(client);
  }

  if (command == "allstatus") {
    allstatus(client);
  }

}


void digitalCommand(WiFiEspClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    mode_val[pin] = value;
    client.print(F("HTTP/1.1 200 OK\r\n"));
    client.print("Content-Type: text/html \r\n\r\n");
    client.println(value);
  }
}

void analogCommand(WiFiEspClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;
    //    client.print(F("HTTP/1.1 200 OK \r\n"));
    //    client.print("Content-Type: text/plain \r\n\r\n");
    //    client.println(value);
  }

}

void servo(WiFiEspClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    myServo[pin].write(value);
    mode_val[pin] = value;
    //    client.print(F("HTTP/1.1 200 OK\r\n"));
    //    client.print("Content-Type: text/plain\r\n\r\n");
    //    client.println(value);
  }
}

void modeCommand(WiFiEspClient client) {
  int pin;
  pin = client.parseInt();

  if (client.read() == '/') {
    String mode = client.readStringUntil('/');
    client.print(F("HTTP/1.1 200 OK\r\n"));
    client.print("Content-Type: text/plain\r\n\r\n");
    if (mode == "input") {
      mode_action[pin] = 'i';
      pinMode(pin, INPUT);
      client.print("Pin D");
      client.print(pin);
      client.println(F(" configured as INPUT!"));
      return;
    }

    if (mode == "output") {
      pinMode(pin, OUTPUT);
      mode_action[pin] = 'o';
      client.print(F("Pin D"));
      client.print(pin);
      client.println(F(" configured as OUTPUT!"));
      return;
    }

    if (mode == "pwm") {
      pinMode(pin, OUTPUT);
      mode_action[pin] = 'p';
      client.print(F("Pin D"));
      client.print(pin);
      client.println(F(" configured as PWM!"));
      return;
    }

    if (mode == "servo") {
      myServo[pin].attach(pin);
      mode_action[pin] = 's';
      client.print(F("Pin D"));
      client.print(pin);
      client.println(F(" configured as SERVO!"));
      return;
    }
  }
}



void allonoff(WiFiEspClient client) {
  int pin, value;
  value = client.parseInt();

  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
  client.print(F("HTTP/1.1 200 OK\r\n"));
  client.print("Content-Type: text/plain\r\n\r\n");
  client.println(value);
#endif
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
  client.print(F("HTTP/1.1 200 OK\r\n"));
  client.print("Content-Type: text/plain\r\n\r\n");
  client.println(value);
#endif
  
}



void allstatus(WiFiEspClient client) {
  client.print(F("HTTP/1.1 200 OK \r\n"));
  client.print("content-type:application/json \r\n\r\n");
  String data_status;
  data_status += "{";
  
  data_status += "\"mode\":[";
  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    data_status += "\"";
    data_status += mode_action[i];
    data_status += "\"";
    if (i != 53)data_status += ",";
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    data_status += "\"";
    data_status += mode_action[i];
    data_status += "\"";
    if (i != 13)data_status += ",";
  }
#endif
  
  data_status += "],";

  data_status += "\"mode_val\":[";
  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    data_status += mode_val[i];
    if (i != 53)data_status += ",";
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    data_status += mode_val[i];
    if (i != 13)data_status += ",";
  }
#endif
  
  data_status += "],";
  client.print(data_status);
  data_status = "";
  data_status += "\"analog\":[";
  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 15; i++) {
    data_status += analogRead(i);
    if (i != 15)data_status += ",";
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 5; i++) {
    data_status += analogRead(i);
    if (i != 5)data_status += ",";
  }
#endif
  
  data_status += "],";

  data_status += "\"lcd\":[";
  for (byte i = 0; i <= lcd_size - 1; i++) {
    data_status += "\"";
    data_status += lcd[i];
    data_status += "\"";
    if (i != lcd_size - 1)data_status += ",";
  }
  data_status += "],";

  data_status += "\"boardtype\":\"";
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  data_status += "kit_mega\",";
#endif
#if defined(__AVR_ATmega32U4__)//Leo
  data_status += "kit_leo\",";
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega16U4__)//UNO
  data_status += "kit_uno\",";
#endif

  data_status += "\"boardname\":\"ESP8266\",";
  data_status += "\"boardstatus\":1";
  data_status += "}";
  client.println(data_status);
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print(F("To see this page in action, open a browser to http://"));
  Serial.println(ip);
  Serial.println();
}

boolean SendCommand(String cmd, String ack) {
  Serial1.println(cmd); // Send "AT+" command to module
  if (!echoFind(ack)) // timed out waiting for ack string
    return true; // ack blank or ack found
}

boolean echoFind(String keyword) {
  byte current_char = 0;
  byte keyword_length = keyword.length();
  long deadline = millis() + TIMEOUT;
  while (millis() < deadline) {
    if (Serial1.available()) {
      char ch = Serial1.read();
      Serial.write(ch);
      if (ch == keyword[current_char])
        if (++current_char == keyword_length) {
          Serial.println();
          return true;
        }
    }
  }
  return false; // Timed out
}
