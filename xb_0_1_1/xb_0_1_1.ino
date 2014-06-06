#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Time.h>

// configuration begin ------------------------------------

// ethernet
// 90.A2.DA.0F.15.0E
byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x15, 0x0E}; 
byte ip[] = {192, 168, 1, 44}; // set the IP address to an unused address on your network

// time offset GMT +2
const long timeZoneOffset = +7200L;

// ntp settings
IPAddress NTPServer(129, 6, 15, 28); // time.nist.gov
int localNTPPort = 2390;
const int NTP_PACKET_SIZE= 48;
byte packetBuffer[NTP_PACKET_SIZE];
EthernetUDP Udp;
int ntpSyncTime = 15;
unsigned long ntpLastUpdate = 0;

// digital out
const int guardPin = 9;
const int tempPinHigh = 6;
const int tempPinMedium = 5;
const int tempPinLow = 3;

// xbee API frame length
const int frameLength = 22;

// guard sensor
int guardValue = 16;
int guardValueIndex = 19;
int guardValueLow = 0;

// temp sensor
float tempValue = 0.0;
int tempValueIndex[] = {20, 21};
float tempVRef = 2.766;
int tempScale = 1024;
float tempCMax = 26.0;
float tempCMin = 19.0;

// configuration end --------------------------------------

// helper functions begin ---------------------------------

void setupEthernet() {
  int i = 0;
   int DHCP = 0;
   DHCP = Ethernet.begin(mac);
   while( DHCP == 0 && i < 2){
     delay(1000);
     DHCP = Ethernet.begin(mac);
     i++;
   }
   if(!DHCP){
    Serial.println("DHCP OK");
     for(;;);
   }
   Serial.println("DHCP KO");
}

void setupGuardAction() {
  pinMode(guardPin, OUTPUT);
  digitalWrite(guardPin, HIGH);
}

int getGuardValueFromByte(byte parts[], int frameLength) {
  return parts[guardValueIndex-1];
}

int getGuardValue(byte parts[], int frameLength) {
  unsigned int guardValueFromByte;
  guardValueFromByte = getGuardValueFromByte(parts, frameLength);
  return guardValueFromByte;
}

void guardAction(int guardValue) {
  if (guardValue == guardValueLow) {
      digitalWrite(guardPin, LOW);
    } else {
      digitalWrite(guardPin, HIGH);
    }
}

void setupTempAction() {
  pinMode(tempPinHigh, OUTPUT);
  digitalWrite(tempPinHigh, HIGH);
  pinMode(tempPinMedium, OUTPUT);
  digitalWrite(tempPinMedium, HIGH);
  pinMode(tempPinLow, OUTPUT);
  digitalWrite(tempPinLow, HIGH);
}

int getTempValueFromByte(byte parts[], int frameLength) {
  return (parts[tempValueIndex[0]-1]*256)+parts[tempValueIndex[1]-1];
}

float getTempVoltageFromValue(int tempValue) {
 return ((float)tempValue/(float)tempScale)*tempVRef;
}

float getTempCelciusFromVoltage(float tempV) {
  return (tempV-0.5)*100;
}

float getTempValue(byte parts[], int frameLength) {
  unsigned int tempValueFromByte;
  float tempV;
  float tempC;
  tempValueFromByte = getTempValueFromByte(parts, frameLength);
  tempV = getTempVoltageFromValue(tempValueFromByte);
  tempC = getTempCelciusFromVoltage(tempV);
  return tempC;
}

void tempAction(float tempC) {
  // high
  if (tempC > tempCMax) {
    digitalWrite(tempPinHigh, HIGH);
    digitalWrite(tempPinMedium, LOW);
    digitalWrite(tempPinLow, LOW);
  }
  // medium
  else if (tempC <= tempCMax && tempC > tempCMin) {
    digitalWrite(tempPinHigh, LOW);
    digitalWrite(tempPinMedium, HIGH);
    digitalWrite(tempPinLow, LOW);
  }
  // low
  else {
    digitalWrite(tempPinHigh, LOW);
    digitalWrite(tempPinMedium, LOW);
    digitalWrite(tempPinLow, HIGH);
  }
}

int getTimeAndDate() {
   int flag=0;
   Udp.begin(localNTPPort);
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

unsigned long sendNTPpacket(IPAddress& address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;                  
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}

// helper functions end -----------------------------------

void setup() {
  Serial.begin(9600);
  setupGuardAction();
  setupTempAction();
}

void loop() {
  if (Serial.available() == frameLength) {
    byte parts[frameLength];
    int partsIndex = 0;
    while(Serial.available()) {
      parts[partsIndex] = Serial.read();
      Serial.print(parts[partsIndex], HEX);
      Serial.print(", ");
      partsIndex++;
    }
    if (partsIndex == frameLength) {
      Serial.print("XBEE FRAME OK");
      // guard sensor
      //int guardValue;
      guardValue = getGuardValue(parts, frameLength);
      guardAction(guardValue);
      // temp sensor
      //float tempValue;
      tempValue = getTempValue(parts, frameLength);
      tempAction(tempValue);
    } else {
      Serial.print("XBEE FRAME KO");
      setupGuardAction();
      setupTempAction();
    }
    Serial.println();
    Serial.flush();
  } else {
    setupGuardAction();
    setupTempAction();
  }
}
