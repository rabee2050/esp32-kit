/*
  Done by TATCO Inc.
  Contacts:
  info@tatco.cc
*/

//d0 ,d3,d4,d8---nodemcu
//d16,d0,d2,d15--gpio


#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <Servo.h>
#include <ArduinoOTA.h>
#include <YalerESP8266WiFiServer.h>
//#include <ESP8266HTTPClient.h>
//#include <ESP8266Ping.h>
#define lcd_size 3 //this will define number of LCD on the phone app
#define host_name "node3" //this will be the host name and the AP ssid
const char* ssid = "Mi rabee";
const char* password = "1231231234";
char mode_action[54];
int mode_val[54];
String lcd[lcd_size];
String api, channel, notification;
Servo myServo[53];
unsigned long last = millis();
int counter = 1;
// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
//YalerESP8266WiFiServer server("try.yaler.io", 80, "gsiot-ddrx-wgph");

void setup() {
  Serial.begin(115200);
  delay(10);

  for (byte i = 0; i <= 16; i++) {
    if (i == 1 || i == 3 || i == 6 || i == 7 || i == 8 || i == 9 || i == 10 || i == 11) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }

  pinMode(A0, INPUT);

  // Connect to WiFi network
  WiFi.mode(WIFI_AP_STA);//WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF
  WiFi.softAP(host_name);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    //    delay(5000);
    //    ESP.restart();
  } else {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());
    Serial.println("mDNS responder started:");
    Serial.println("http://"host_name".local");

  }
  //  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //    Serial.println("Connection Failed! Rebooting...");
  //    delay(5000);
  //    ESP.restart();
  //  }
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(500);
  //    Serial.print(".");
  //  }

  server.begin();
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

}

void update_input() {
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);
    }
  }
}

void loop() {
  ArduinoOTA.handle();
  lcd[0] = "Test 1 LCD";// you can send any data to your mobile phone.
  lcd[1] = analogRead(0);// you send analog value of A1
  lcd[2] = "Test 2 LCD";// here you send the battery status if you are using Adafruit BLE

  WiFiClient client = server.available();
  if (client) {
    process(client);
    client.flush();
    client.stop();
  }
  delay(50);
  update_input();
}

void process(WiFiClient client) {
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


void digitalCommand(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    mode_val[pin] = value;
    String data = "";
    data += F("HTTP/1.1 200 OK\r\n");
    data += F("Content-Type: text/plain \r\n\r\n");
    data += value;
    client.print(data);
  }
}

void analogCommand(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;
    String data = "";
    data += F("HTTP/1.1 200 OK\r\n");
    data += F("Content-Type: text/plain \r\n\r\n");
    data += value;
    client.print(data);
  }

}

void servo(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    myServo[pin].write(value);
    mode_val[pin] = value;
    String data = "";
    data += F("HTTP/1.1 200 OK\r\n");
    data += F("Content-Type: text/plain \r\n\r\n");
    data += value;
    client.print(data);
  }
}

void modeCommand(WiFiClient client) {
  String data = "";
  int pin;
  pin = client.parseInt();

  if (client.read() == '/') {
    String mode = client.readStringUntil('/');
    data += F("HTTP/1.1 200 OK\r\n");
    data += F("Content-Type: text/plain \r\n\r\n");
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



void allonoff(WiFiClient client) {
  int pin, value;
  value = client.parseInt();

  for (byte i = 0; i <= 16; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
  String data = "";
  data += F("HTTP/1.1 200 OK\r\n");
  data += F("Content-Type: text/plain \r\n\r\n");
  data += value;
  client.print(data);
}



void allstatus(WiFiClient client) {
  String data_status;
  data_status += F("HTTP/1.1 200 OK \r\n");
  data_status += F("content-type:application/json \r\n\r\n");

  data_status += "{";

  data_status += "\"mode\":[";

  for (byte i = 0; i <= 16; i++) {
    data_status += "\"";
    data_status += mode_action[i];
    data_status += "\"";
    if (i != 16)data_status += ",";
  }
  data_status += "],";

  data_status += "\"mode_val\":[";
  for (byte i = 0; i <= 16; i++) {
    data_status += mode_val[i];
    if (i != 16)data_status += ",";
  }
  data_status += "],";

  data_status += "\"analog\":[";

  for (byte i = 0; i <= 0; i++) {
    data_status += analogRead(i);
    if (i != 0)data_status += ",";
  }
  data_status += "],";

  data_status += "\"lcd\":[";
  for (byte i = 0; i <= lcd_size - 1; i++) {
    data_status += "\"";
    data_status += lcd[i];
    data_status += "\"";
    if (i != lcd_size - 1)data_status += ",";
  }
  data_status += "],";

  data_status += "\"boardtype\":\"feather_esp8266\",";

  data_status += "\"boardname\":\"";
  data_status += host_name;
  data_status += "\",\"boardstatus\":1";
  data_status += "}";
  client.print(data_status);
}

