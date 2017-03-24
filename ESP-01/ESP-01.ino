
/*
  Done by TATCO Inc.
  Contacts:
  info@tatco.cc

  Note:
  Make sure the baud rate of the ESP-01 is 9600.

  Connection(Arduino Uno):
  Arduino_rx_pin 10  ------->   ESP-01_Tx_Pin
  Arduino_tx_pin 11 ------->   ESP-01_Rx_Pin

  Tested with ESP-01 module on:
  1- UNO
  2- Mega
  3- Loenardo

*/

#include "WiFiEsp.h"
#include <Servo.h>
#include "SoftwareSerial.h"

#define Arduino_rx_pin 10
#define Arduino_tx_pin 11
SoftwareSerial serial(Arduino_rx_pin, Arduino_tx_pin); //SoftwareSerial serial(Arduino_rx_pin, Arduino_tx_pin);

char ssid[] = "Mi rabee";            // your network SSID (name)
char pass[] = "1231231234";        // your network password

char mode_action[54];
int mode_val[54];
Servo myServo[53];

#define lcd_size 3 //this will define number of LCD on the phone app
String lcd[lcd_size];


#define TIMEOUT 100 // mS
int status = WL_IDLE_STATUS;     // the Wifi radio's status
String http_ok = "HTTP/1.1 200 OK\r\n Content-Type: text/plain \r\n\r\n";

WiFiEspServer server(80);

void setup()
{
  Serial.begin(9600);
  serial.begin(9600);

//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for Arduino Leonardo only
//  }
  Serial.println(F("WiFi ESP-01 Initialaization.........\n"));

  WiFi.init(&serial);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi ESP-01 Module not present, Please check the wiring.\n"));
    while (true);
  }

  Serial.println(F("WIFI ESP-01 Module is Ok \n"));

  if ( status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);
    Serial.println();
    status = WiFi.begin(ssid, pass);

  }

  if ( status == WL_CONNECTED) {
    Serial.println(F("You're connected to the network\n"));
    printWifiStatus();
    server.begin();
  } else {
    Serial.println(F("Connecting failed to the network, Please check the WIFI SSID & PASSWORD\n"));
  }

  KitSetup();
  
}

void loop()
{

  lcd[0] = "Test 1 LCD";// you can send any data to your mobile phone.
  lcd[1] = analogRead(0);// you can send analog value of A0
  lcd[2] = "Test 2 LCD";// you can send any data to your mobile phone.


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
    client.print(http_ok + value);
  }
}

void analogCommand(WiFiEspClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;
    client.print(http_ok + value);
  }

}

void servo(WiFiEspClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    myServo[pin].write(value);
    mode_val[pin] = value;
    client.print(http_ok + value);
  }
}

void modeCommand(WiFiEspClient client) {
  String data = "";
  int pin;
  pin = client.parseInt();

  if (client.read() == '/') {
    String mode = client.readStringUntil('/');
    data += http_ok;
    if (mode == "input") {
      mode_action[pin] = 'i';
      pinMode(pin, INPUT);
      data += F("Pin D");
      data += pin;
      data += F(" configured as INPUT!");
      client.print(data);
      return;
    }

    if (mode == "output") {
      pinMode(pin, OUTPUT);
      mode_action[pin] = 'o';
      data += F("Pin D");
      data += pin;
      data += F(" configured as OUTPUT!");
      client.print(data);

      return;
    }

    if (mode == "pwm") {
      pinMode(pin, OUTPUT);
      mode_action[pin] = 'p';
      data += F("Pin D");
      data += pin;
      data += F(" configured as PWM!");
      client.print(data);

      return;
    }

    if (mode == "servo") {
      myServo[pin].attach(pin);
      mode_action[pin] = 's';
      data += F("Pin D");
      data += pin;
      data += F(" configured as SERVO!");
      client.print(data);

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
  client.print(http_ok + value);
#endif
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
  client.print(http_ok + value);
#endif

}

void update_input() {
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);
    }
  }
}

void KitSetup() {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == Arduino_tx_pin || i == Arduino_rx_pin) {//tx18,rx19
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
    if (i == 0 || i == 1 || i == Arduino_rx_pin || i == Arduino_tx_pin) {
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
  Serial.print(F("To see the action, open the ESP8266 Kit App then connect to http://"));
  Serial.println(ip);
  Serial.println();
}

boolean SendCommand(String cmd, String ack) {
  serial.println(cmd); // Send "AT+" command to module
  if (!echoFind(ack)) // timed out waiting for ack string
    return true; // ack blank or ack found
}

boolean echoFind(String keyword) {
  byte current_char = 0;
  byte keyword_length = keyword.length();
  long deadline = millis() + TIMEOUT;
  while (millis() < deadline) {
    if (serial.available()) {
      char ch = serial.read();
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

void allstatus(WiFiEspClient client) {
  String data_status;
  data_status += F("HTTP/1.1 200 OK \r\n");
  data_status += F("content-type:application/json \r\n\r\n");
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
  client.print(data_status);
}

