// include libraries begin --------------------------------

#include "parameters.h"
#include <SPI.h>
#include <Time.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// include libraries end ----------------------------------

// configuration begin ------------------------------------

// WiFi config // 90.A2.DA.0E.B5.A1
int status = WL_IDLE_STATUS;
char ssid[] = wfSSID;
char pass[] = wfPWD;
//int keyIndex = 0; // your network key Index number (needed only for WEP)

// ntp settings
const long timeZoneOffset = +7200L;
unsigned int NTPLocalPort = 2390;
IPAddress NTPServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
WiFiUDP Udp;
int ntpSyncTime = 30;
unsigned long ntpLastUpdate = 0;
time_t prevDisplay = 0;

// Arduino config
const int wfOkPin = 6;
const int wfKoPin = 5;
const int temPin = A0;
const int lumPin = A1;
const int ct0Pin = 2;

// configuration end --------------------------------------

// helper functions begin ---------------------------------

void setupSerial() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
}

void serialPrint() {
  
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
    //Serial.println("WiFi shield not present");
    while (true) {
      toggleWfStatus(1);
      delay(1000);
      toggleWfStatus(0);
      delay(1000);
    }
  } 
  while (status != WL_CONNECTED) { 
    //Serial.print("Attempting to connect to SSID: ");
    //Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  //Serial.println("Connected to wifi");
  //printWifiStatus();
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
  //Serial.println("\nStarting connection to server...");
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

String getDigits(byte digits){
  String _digits = "";
  if(digits < 10) {
    _digits = "0";
  }
  _digits = String(_digits+String(digits));
  return _digits;
}

String getTimeAndDateString() {
  /*String y = String(year());
  String m = String(month());
  String d = String(day());
  String h = String(hour());
  String i = String(minute());
  String s = String(second());*/
  String y = String(year());
  String m = getDigits(month());
  String d = getDigits(day());
  String h = getDigits(hour());
  String i = getDigits(minute());
  String s = getDigits(second());
  return String(y+"-"+m+"-"+d+"T"+h+":"+i+":"+s+".000Z");
}

String getJsonMessage() {
  //return getTimeAndDateString(); // OK !!!
  String ts = getTimeAndDateString();
  //String ms = String('{"type": "arduino", "timestamp": "'+'ts'+'"}');
  String ms = String("{\"type\": \"arduino\", \"timestamp\": \""+ts+"\"}");
  return ms;
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
    /*if (retry < 10) {
      Serial.println("NTP OK");
    } else {
      Serial.println("NTP KO");
    }*/
    if (now() != prevDisplay){
      prevDisplay = now();
      Serial.println(getJsonMessage()); // todo, send this to elasticsearch
    }
  }
}

// main program loop end ----------------------------------

