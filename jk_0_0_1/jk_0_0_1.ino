// include libraries begin --------------------------------

#include "parameters.h"
#include <SPI.h>
#include <Ethernet.h>
#include <MemoryFree.h>

// include libraries end ----------------------------------

// configuration begin ------------------------------------

// ethernet
// 90.A2.DA.0F.15.0E // wired
// 90.A2.DA.0E.B5.A1 // wifi
byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x15, 0x0E};
EthernetClient client;
//byte ip[] = {192, 168, 1, 44}; // set the IP address to an unused address on your network
// ES settings
char ESServer[] = paramESServer;
int ESServerPort = paramESServerPort;
int DHCPStatus = 0;
int ESClientWait = paramESClientWait;

int leds[] = {paramLeds};
int ononon[] = {1, 1, 1};
int onoffoff[] = {1, 0, 0};
int offonoff[] = {0, 1, 0};
int offoffon[] = {0, 0, 1};
int offoffoff[] = {0, 0, 0};

long pM = 0;

// configuration end --------------------------------------

// helper functions begin ---------------------------------

void setupSerial() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  delay(1000);
}

void setupEthernet() {
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    //Ethernet.begin(mac, ip);
    DHCPStatus = 0;
    return;
  }
  delay(1000);
  DHCPStatus = 1;
}

void setupHardware() {
  int kl = sizeof(leds);
  for (int k = 0; k < kl; k++) {
    pinMode(k, OUTPUT);
    digitalWrite(k, LOW);
  }
  delay(1000);
}

void printFreeMemory() {
  Serial.print("freeMemory : ");
  Serial.print(freeMemory());
  Serial.println(" B");
}

void sendHttpRequest(jobname) {
  String ESClientPath = paramESClientPath;
  ESClientPath.replace("JOBNAME", "</");
  Serial.println(F("Sending http request to server"));
  if (client.connect(ESServer, ESServerPort)) {
    client.print("GET ");
    client.print(paramESClientPath);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(ESServer);
    client.print("Authorization: Basic ");
    client.println(paramESClientAuthRealm);
    client.print("User-Agent: ");
    client.println(paramESClientUA);
    client.println("Connection: close");
    client.println();
    //delay(2000);
    //client.flush();
    //client.stop();
  } else {
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    //client.flush();
    //client.stop();
  }
}

void getJsonMessage() {
  
}

void actionLeds(int groupStates[], int groupMax) {
  //for (int k = 0; k < groupMax; k++) {
    for (int k = 0; k < groupMax; k++) {
      actionLedGroupToggle(groupStates[k], leds[k], groupMax);
    }
  //}
}

void actionLedGroupToggle(int state, int pin, int offset) {
  //Serial.println(String("state: ")+String(state)+String(", pin: ")+String(pin)+String(", offset: ")+String(offset));
  if (state == 0) {
    digitalWrite(pin, HIGH);
    digitalWrite(pin+offset, LOW);
  } else {
    digitalWrite(pin, LOW);
    digitalWrite(pin+offset, HIGH);
  }
}

// helper functions end -----------------------------------

// main program setup begin -------------------------------

void setup() 
{
  setupSerial();
  setupHardware();
  setupEthernet();
}

// main program setup end ---------------------------------

// main program loop begin --------------------------------

void loop()
{
  unsigned long cM = millis();
  if (cM-Pm > ESClientWait*1000) {
    Pm = cM;
    String jobname = "artefactlabs";
    sendHttpRequest(jobname);
  } else {
    // wait
  }
  
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    client.flush();
    for(;;)
      ;
  }
  
  /*
  actionLeds(ononon, 3); delay(1000);
  actionLeds(offoffoff, 3); delay(1000);
  actionLeds(onoffoff, 3); delay(1000);
  actionLeds(offonoff, 3); delay(1000);
  actionLeds(offoffon, 3); delay(1000);
  */
  /*
  Serial.println(getJsonMessage());
  printFreeMemory();
  delay(1000);
  */
}

// main program loop end ----------------------------------

