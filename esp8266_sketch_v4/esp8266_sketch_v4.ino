/*
  Done by TATCO Inc.
  Contacts:
  info@tatco.cc

  Release Notes:
  - V1 Created 14 Jan 2017
  - V2 Updated 24 Mar 2017
  - V3 Updated 20 Oct 2017
  - V4 Updated 07 Oct 2018


  Tested on:
  1- NodeMCU v3.
  2- Adafruit feather Huzzah ESP8266.

*/

#include <ESP8266WiFi.h>
#include <Servo.h>

const char* ssid = "HUAWEI";//WIFI Network Name.
const char* password = "1231231234";//WIFI Password

#define lcdSize 3 //this will define number of LCD display on the LCD Dashboard tab.
String protectionPassword = ""; //This will not allow anyone to add or control your board.


char pinsMode[54];
int pinsValue[54];
Servo servoArray[53];
String lcd[lcdSize];

WiFiServer server(80);// Create server and specify the port.

String httpAppJsonOk = "HTTP/1.1 200 OK\r\n content-type:application/json \r\n\r\n";
String httpTextPlainOk = "HTTP/1.1 200 OK\r\n content-type:text/plain \r\n\r\n";

unsigned long serialTimer = millis();

void setup() {
  Serial.begin(115200);

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);//WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected successfully, the IP address: ");
  server.begin();
  Serial.println(WiFi.localIP());
  boardInit();
}

void loop() {

  serialPrintIpAddress();

  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while (!client.available()) {
    delay(1);
  }

  updateInputValues();

  lcd[0] = "Test 1 LCD";// Send any data to the LCD dashboard in the App.
  lcd[1] = analogRead(A0);// Send analog value of A0 to the LCD dashboard in the App.
  lcd[2] = random(1, 100);// Send any data to the LCD dashboard in the App.

  client.flush();
  process(client);
  client.stop();
  return;
}

void process(WiFiClient client) {
  String getString = client.readStringUntil('/');
  String arduinoString = client.readStringUntil('/');
  String command = client.readStringUntil('/');

  if (command == "digital") {
    digitalCommand(client);
  }

  if (command == "pwm") {
    pwmCommand(client);
  }

  if (command == "servo") {
    servoCommand(client);
  }

  if (command == "terminal") {
    terminalCommand(client);
  }

  if (command == "mode") {
    modeCommand(client);
  }

  if (command == "allonoff") {
    allonoff(client);
  }

  if (command == "password") {
    changePassword(client);
  }

  if (command == "allstatus") {
    allstatus(client);
  }

}

void terminalCommand(WiFiClient client) {//Here you recieve data form app terminal
  String data = client.readStringUntil('/');
  client.print(httpAppJsonOk + "Ok from Arduino " + String(random(1, 100)));
  Serial.println(data);
}

void digitalCommand(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    pinsValue[pin] = value;
    client.print(httpAppJsonOk + value);
  }
}

void pwmCommand(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    pinsValue[pin] = value;
    client.print(httpAppJsonOk + value);
  }
}

void servoCommand(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    servoArray[pin].write(value);
    pinsValue[pin] = value;
    client.print(httpAppJsonOk + value);
  }
}

void modeCommand(WiFiClient client) {
  String  pinString = client.readStringUntil('/');
  int pin = pinString.toInt();
  String mode = client.readStringUntil('/');
  if (mode != "servo") {
    servoArray[pin].detach();
  };

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    pinsMode[pin] = 'o';
    pinsValue[pin] = 0;
    allstatus(client);
  }
  if (mode == "push") {
    pinsMode[pin] = 'm';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    allstatus(client);
  }
  if (mode == "schedule") {
    pinsMode[pin] = 'c';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    allstatus(client);
  }

  if (mode == "input") {
    pinsMode[pin] = 'i';
    pinsValue[pin] = 0;
    pinMode(pin, INPUT);
    allstatus(client);
  }

  if (mode == "pwm") {
    pinsMode[pin] = 'p';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    analogWrite(pin, 0);
    allstatus(client);
  }

  if (mode == "servo") {
    pinsMode[pin] = 's';
    pinsValue[pin] = 0;
    servoArray[pin].attach(pin);
    servoArray[pin].write(0);
    allstatus(client);
  }

}

void allonoff(WiFiClient client) {
  int pin, value;
  value = client.parseInt();
  for (byte i = 0; i <= 16; i++) {
    if (pinsMode[i] == 'o') {
      digitalWrite(i, value);
      pinsValue[i] = value;
    }
  }
  client.print(httpTextPlainOk + value);
}

void changePassword(WiFiClient client) {
  String data = client.readStringUntil('/');
  protectionPassword = data;
  client.print(httpAppJsonOk);
}

void allstatus(WiFiClient client) {
  String dataResponse;
  dataResponse += F("HTTP/1.1 200 OK \r\n");
  dataResponse += F("content-type:application/json \r\n\r\n");
  dataResponse += "{";

  dataResponse += "\"m\":[";//m for mode
  for (byte i = 0; i <= 16; i++) {
    dataResponse += "\"";
    dataResponse += pinsMode[i];
    dataResponse += "\"";
    if (i != 16)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"v\":[";//v for value
  for (byte i = 0; i <= 16; i++) {
    dataResponse += pinsValue[i];
    if (i != 16)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"a\":[";//a for analog value
  for (byte i = A0; i <= A0; i++) {
    dataResponse += analogRead(i);
    if (i != A0)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"l\":[";//l for LCD value
  for (byte i = 0; i <= lcdSize - 1; i++) {
    dataResponse += "\"";
    dataResponse += lcd[i];
    dataResponse += "\"";
    if (i != lcdSize - 1)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"p\":\""; // p for Password.
  dataResponse += protectionPassword;
  dataResponse += "\"";
  dataResponse += "}";
  client.print(dataResponse);
}

void serialPrintIpAddress() {
  if (Serial.read() > 0) {
    if (millis() - serialTimer > 2000) {
      Serial.println();
      Serial.println("IP address is:");
      Serial.println(WiFi.localIP());
    }
    serialTimer = millis();
  }

}

void updateInputValues() {
  for (byte i = 0; i < sizeof(pinsMode); i++) {
    if (pinsMode[i] == 'i') {
      pinsValue[i] = digitalRead(i);
    }
  }
}

void boardInit() {
  for (byte i = 0; i <= 16; i++) {
    if (i == 1 || i == 3 || i == 6 || i == 7 || i == 8 || i == 9 || i == 10 || i == 11) {
      pinsMode[i] = 'x';
      pinsValue[i] = 0;
    }
    else {
      pinsMode[i] = 'o';
      pinsValue[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
  pinMode(A0, INPUT);
}
