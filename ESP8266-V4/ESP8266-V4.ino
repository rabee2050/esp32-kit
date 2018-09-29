/*
  Done by TATCO Inc.
  Contacts:
  info@tatco.cc

  Release Notes:
  - V1 Created 14 Jan 2017
  - V2 Updated 24 Mar 2017
  - V3 Updated 20 Oct 2017
  - V4 Updated 01 Oct 2018


  Tested on:
  1- NodeMCU v3.
  2- Adafruit feather Huzzah ESP8266.

*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Servo.h>
#include <ArduinoOTA.h>


const char* ssid = "HUAWEI Mi";//WIFI SSID Name
const char* password = "1231231234";//WIFI Password
String protectionPassword = ""; //This will not allow anyone to add or control your board by knowing the IP address.
#define lcd_size 3 //this will define number of LCD display on the phone LCD tab.


#define host_name "node2"//this will be the host name and the Esp8266 access point SSID.
#define  wifi_available true // If you dont have access point in your place then make it false to make ESP8266 as access point.
WiFiServer server(80);// specify the port to listen on as an argument

char mode_action[54];
int mode_val[54];
Servo myServo[53];
String lcd[lcd_size];

String http_ok = "HTTP/1.1 200 OK\r\n content-type:application/json \r\n\r\n";
String http_ok_text = "HTTP/1.1 200 OK\r\n content-type:text/plain \r\n\r\n";
unsigned long last_ip = millis();

void setup() {
  Serial.begin(115200);
  if (wifi_available) {
    WiFi.mode(WIFI_AP_STA);//WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF
    WiFi.softAP(host_name, "1231231234"); //(APname, password)
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.print("Connection Failed! to ");
      Serial.println(ssid);
      //    delay(5000);
      //    ESP.restart();
    } else {
      Serial.println();
      Serial.println("WiFi connected and the IP address is:");
      Serial.println(WiFi.localIP());
      Arduino_OTA_Start();
    }
  } else {
    Serial.println();
    Serial.print(" ESP8266 Access Point only is activated, you can connect to a this on SSID: ");
    Serial.println(host_name);
    WiFi.mode(WIFI_AP);//WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF
    WiFi.softAP(host_name, "1231231234");
  }
  server.begin();
  boardInit();
}

void loop() {
  if (wifi_available)ArduinoOTA.handle();

  lcd[0] = "Test 1 LCD";// you can send any data to the App inside the LCD tab.
  lcd[1] = analogRead(0);// you can send analog value of A0
  lcd[2] = random(1, 100);// you can send any data to the App inside the LCD tab.

  WiFiClient client = server.available();
  if (client) {
    process(client);
    client.flush();
    client.stop();
    delay(50);
  }
  update_input();
  print_wifiStatus();
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
  Serial.println(data);
  client.print(http_ok_text);
  client.print("Ok from Arduino " + random(1, 100));
}

void digitalCommand(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    mode_val[pin] = value;
    client.print(http_ok + value);
  }
}

void pwmCommand(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;
    client.print(http_ok + value);
  }
}

void servoCommand(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    myServo[pin].write(value);
    mode_val[pin] = value;
    client.print(http_ok + value);
  }
}

void modeCommand(WiFiClient client) {
  String  pinString = client.readStringUntil('/');
  int pin = pinString.toInt();
  String mode = client.readStringUntil('/');
  if (mode != "servo") {
    myServo[pin].detach();
  };

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    mode_action[pin] = 'o';
    mode_val[pin] = 0;
    allstatus(client);
  }
  if (mode == "push") {
    mode_action[pin] = 'm';
    mode_val[pin] = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    allstatus(client);
  }
  if (mode == "schedule") {
    mode_action[pin] = 'c';
    mode_val[pin] = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    allstatus(client);
  }

  if (mode == "input") {
    mode_action[pin] = 'i';
    mode_val[pin] = 0;
    pinMode(pin, INPUT);
    allstatus(client);
  }

  if (mode == "pwm") {
    mode_action[pin] = 'p';
    mode_val[pin] = 0;
    pinMode(pin, OUTPUT);
    analogWrite(pin, 0);
    allstatus(client);
  }

  if (mode == "servo") {
    mode_action[pin] = 's';
    mode_val[pin] = 0;
    myServo[pin].attach(pin);
    myServo[pin].write(0);
    allstatus(client);
  }

}



void allonoff(WiFiClient client) {
  int pin, value;
  value = client.parseInt();
  Serial.print(value );
  for (byte i = 0; i <= 16; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
  client.print(http_ok + value);
}

void changePassword(WiFiClient client) {
  String data = client.readStringUntil('/');
  protectionPassword = data;
  client.print(http_ok_text);
}

void update_input() {
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);
    }
  }
}

void boardInit() {
  for (byte i = 0; i <= 16; i++) {
    if (i == 1 || i == 3 || i == 6 || i == 7 || i == 8 || i == 9 || i == 10 || i == 11) {
      mode_action[i] = 'x';
      mode_val[i] = 0;
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
  pinMode(A0, INPUT);

}

void Arduino_OTA_Start() {
  //  ArduinoOTA.setPort(uint16_t 80);
  //  ArduinoOTA.setPassword((const char *)"123");
  ArduinoOTA.setHostname((const char *)host_name);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("mDNS responder started at:");
  Serial.println("http://"host_name".local");
}

void allstatus(WiFiClient client) {
  String data_status;
  data_status += F("HTTP/1.1 200 OK \r\n");
  data_status += F("content-type:application/json \r\n\r\n");
  data_status += "{";

  data_status += "\"m\":[";
  for (byte i = 0; i <= 16; i++) {
    data_status += "\"";
    data_status += mode_action[i];
    data_status += "\"";
    if (i != 16)data_status += ",";
  }
  data_status += "],";

  data_status += "\"v\":[";
  for (byte i = 0; i <= 16; i++) {
    data_status += mode_val[i];
    if (i != 16)data_status += ",";
  }
  data_status += "],";

  data_status += "\"a\":[";
  for (byte i = A0; i <= A0; i++) {
    data_status += analogRead(i);
    if (i != A0)data_status += ",";
  }
  data_status += "],";

  data_status += "\"l\":[";
  for (byte i = 0; i <= lcd_size - 1; i++) {
    data_status += "\"";
    data_status += lcd[i];
    data_status += "\"";
    if (i != lcd_size - 1)data_status += ",";
  }
  data_status += "],";

  data_status += "\"p\":\""; // f for Feedback.
  data_status += protectionPassword;
  data_status += "\"";
  data_status += "}";
  client.print(data_status);
}

void print_wifiStatus() {
  if (Serial.read() > 0) {
    if (millis() - last_ip > 2000) {
      Serial.println();
      Serial.println("WiFi connected and the IP address is:");
      Serial.println(WiFi.localIP());
      Serial.println("mDNS responder started at:");
      Serial.println(host_name".local");
      Serial.println();
    }
    last_ip = millis();
  }

}
