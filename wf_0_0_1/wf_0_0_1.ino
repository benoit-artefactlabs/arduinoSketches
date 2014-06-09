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
IPAddress NTPServer(129, 6, 15, 28); // time.nist.gov NTP server
byte packetBuffer[NTP_PACKET_SIZE];
WiFiUDP Udp;
unsigned long ntpLastUpdate = 0;
time_t prevDisplay = 0;

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
    Serial.println("WiFi shield not present");
    while (true) {
      toggleWfStatus(1);
      delay(1000);
      toggleWfStatus(0);
      delay(1000);
    }
  } 
  while (status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
  toggleWfStatus(1);
}

void toggleWfStatus(int toggle) {
  //Serial.println("Toggle WiFi status leds "+String(toggle));
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
  Serial.println("\nStarting connection to server...");
  Udp.begin(NTPLocalPort);
}

int getTimeAndDate() {
  int flag=0;
  //Udp.begin(localNTPPort);
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
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
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
  return String(String(year())+"-"+getDigitsFromByte(month())+"-"+getDigitsFromByte(day())+"T"+getDigitsFromByte(hour())+":"+getDigitsFromByte(minute())+":"+getDigitsFromByte(second())+".000Z");
}

String getHomeId() {
  return homeId;
}

String getTemperature() {
  char TEM[6];
  float tem = (((analogRead(temPin)/1024.0)*5.0)-.5)*100;
  return getDigitsFromFloat(tem, 5, 2);
}

String getLuminosity() {
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

byte getCt0Status() {
  return digitalRead(ct0Pin);
}

String getJsonMessage() {
  return String("{\"type\":\"arduino\",\"homeId\":\""+getHomeId()+"\",\"ct0\":\""+String(getCt0Status())+"\",\"temperature\":\""+getTemperature()+"\",\"luminosity\":\""+getLuminosity()+"\",\"timestamp\":\""+getTimeAndDateString()+"\"}");
}

void printFreeMemory() {
  Serial.print("freeMemory : ");
  Serial.print(freeMemory());
  Serial.println(" B");
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
      Serial.println(getJsonMessage()); // todo, send this to elasticsearch
      //printFreeMemory();
    }
  }
  /*
  Serial.println(getJsonMessage());
  printFreeMemory();
  delay(1000);
  */
}

// main program loop end ----------------------------------

