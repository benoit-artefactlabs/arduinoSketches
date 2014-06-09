// include libraries begin --------------------------------

#include "parameters.h"
#include <SPI.h>
#include <Time.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <MemoryFree.h>

// include libraries end ----------------------------------

// configuration begin ------------------------------------

// WiFi config // 90.A2.DA.0E.B5.A1
int status = WL_IDLE_STATUS;
char ssid[] = wfSSID;
char pass[] = wfPWD;
//int keyIndex = 0; // your network key Index number (needed only for WEP)

// ntp settings
//IPAddress NTPServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress NTPServer(wfNTPServer);
byte packetBuffer[NTP_PACKET_SIZE];
WiFiUDP Udp;
unsigned long ntpLastUpdate = 0;
time_t prevDisplay = 0;

// ES settings
char ESServer[] = wfESServer;
int ESServerPort = wfESServerPort;
WiFiClient client;

// configuration end --------------------------------------

// helper functions begin ---------------------------------

void setupSerial() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
}

void setupHardware() {
  pinMode(wfOkPin, OUTPUT);
  digitalWrite(wfOkPin, LOW);
  pinMode(wfKoPin, OUTPUT);
  digitalWrite(wfKoPin, LOW);
  pinMode(ct0Pin, INPUT);
}

void setupWf() {
  toggleWfStatus(0);
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    while (true) {
      toggleWfStatus(1);
      delay(1000);
      toggleWfStatus(0);
      delay(1000);
    }
  } 
  while (status != WL_CONNECTED) { 
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  Serial.println(F("Connected to wifi"));
  printWifiStatus();
  toggleWfStatus(1);
}

void toggleWfStatus(int toggle) {
  if (toggle == 0) {
    digitalWrite(wfOkPin, LOW);
    digitalWrite(wfKoPin, HIGH);
  } else {
    digitalWrite(wfOkPin, HIGH);
    digitalWrite(wfKoPin, LOW);
  }
  delay(250);
}

void setupNtp() {
  Serial.println(F("Opening connection to NTP server"));
  Udp.begin(NTPLocalPort);
}

int getTimeAndDate() {
  int flag=0;
  sendNTPpacket(NTPServer);
  delay(1000);
  if (Udp.parsePacket()){
    Udp.read(packetBuffer,NTP_PACKET_SIZE);
    unsigned long highWord, lowWord, epoch;
    highWord = word(packetBuffer[40], packetBuffer[41]);
    lowWord = word(packetBuffer[42], packetBuffer[43]);  
    epoch = highWord << 16 | lowWord;
    epoch = epoch - 2208988800 + timeZoneOffset;
    flag=1;
    setTime(epoch);
    ntpLastUpdate = now();
  }
  return flag;
}

unsigned long sendNTPpacket(IPAddress& address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}

void printWifiStatus() {
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}

String getDigitsFromByte(byte digits){
  String _digits = "";
  if(digits < 10) {
    _digits = "0";
  }
  _digits = String(_digits+String(digits));
  return _digits;
}

String getDigitsFromFloat(float digits, int digitsLen, int digitsDec) {
  char DIGITS[digitsLen+1];
  return String(dtostrf(digits, digitsLen, digitsDec, DIGITS));
}

String getTimeAndDateString() {
  return String(String(year())+"-"+getDigitsFromByte(month())+"-"+getDigitsFromByte(day())+"T"+getDigitsFromByte(hour())+":"+getDigitsFromByte(minute())+":"+getDigitsFromByte(second())+".550Z");
}

String getHomeId() {
  return homeId;
}

String getTemperature() {
  char TEM[6];
  float tem = (((analogRead(temPin)/1024.0)*5.0)-.5)*100;
  return getDigitsFromFloat(tem, 5, 2);
}

String getLuminosityText() {
  int L = analogRead(lumPin);
  if (L < 50) {
    return String("dark");
  }
  else if (L <  200) {
    return String("dim");
  }
  else if (L < 500) {
    return String("light");
  }
  else if (L < 800) {
    return String("bright");
  }
  else {
    return String("intense");
  }
}

String getLuminosity() {
  return String(analogRead(lumPin));
}

String getCt0StatusText() {
  int cts = digitalRead(ct0Pin);
  if (cts == 1) {
    return String("opened");
  } else {
    return String("closed");
  }
}

String getCt0Status() {
  return String(digitalRead(ct0Pin));
}

String getJsonMessage() {
  return String("{\"arduino_type\":\"xb_0_0_1\",\"homeId\":\""+getHomeId()+"\",\"ct0StatusText\":\""+getCt0StatusText()+"\",\"ct0Status\":"+getCt0Status()+",\"temperature\":"+getTemperature()+",\"luminosityText\":\""+getLuminosityText()+"\",\"luminosity\":\""+getLuminosity()+"\",\"@version\":\"1\",\"@timestamp\":\""+getTimeAndDateString()+"\"}");
}

String getESServerQuery() {
  return String("/caillaux-"+String(year())+"."+getDigitsFromByte(month())+"."+getDigitsFromByte(day())+"/arduino/");
}

void printFreeMemory() {
  Serial.print("freeMemory : ");
  Serial.print(freeMemory());
  Serial.println(" B");
}

void sendHttpRequest() {
  Serial.println(F("Sending stats to server"));
  if (client.connect(ESServer, ESServerPort)) {
    String data = getJsonMessage();
    client.print("POST ");
    client.print(getESServerQuery());
    client.println("  HTTP/1.1");
    client.print("Host: ");
    client.println(wfESServer);
    client.print("User-Agent: ");
    client.println(wfESClientUA);
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);
    client.println();
  }
}

// helper functions end -----------------------------------

// main program setup begin -------------------------------

void setup() 
{
  setupSerial();
  setupHardware();
  setupWf();
  setupNtp();
}

// main program setup end ---------------------------------

// main program loop begin --------------------------------

void loop()
{
  if (now()-ntpLastUpdate > ntpSyncTime) {
    int retry = 0;
    while (!getTimeAndDate() && retry < 10) {
      retry++;
    }
    if (now() != prevDisplay){
      prevDisplay = now();
      sendHttpRequest();
      Serial.println(getJsonMessage()); // todo, send this to elasticsearch
      printFreeMemory();
    }
  }
  /*
  Serial.println(getJsonMessage());
  printFreeMemory();
  delay(1000);
  */
}

// main program loop end ----------------------------------

