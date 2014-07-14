// include libraries begin --------------------------------
#include "parameters.h"
#include <SPI.h>
#include <Ethernet.h>
#include <TextFinder.h>
#include <MemoryFree.h>
// include libraries end ----------------------------------

// configuration begin ------------------------------------
// 90.A2.DA.0F.15.0E // wired
// 90.A2.DA.0E.B5.A1 // wifi
byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x15, 0x0E};
EthernetClient client;
//byte ip[] = {192, 168, 1, 44}; // set the IP address to an unused address on your network
char ESServer[] = paramESServer;
int ESServerPort = paramESServerPort;
int DHCPStatus = 0;
int ESClientWait = paramESClientWait;
boolean ESClientLastConnected = false;
int currentJobIndex = 0;
TextFinder finder(client);
int leds[] = {paramLeds};
char* jobs[] = {paramJobs};
unsigned long pM = 0;
boolean httpResponseSearchStatus = false; 
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
  } else {
    Serial.println("Configured Ethernet using DHCP");
    DHCPStatus = 1;
  }
}

void setupHardware() {
  int kl = sizeof(leds);
  for (int k = 0; k < kl; k++) {
    pinMode(k, OUTPUT);
    digitalWrite(k, LOW);
  }
}

void printFreeMemory() {
  Serial.print("freeMemory : ");
  Serial.print(freeMemory());
  Serial.println(" B");
}

void jobAction() {
  sendHttpRequest(String(jobs[currentJobsIndex]));
  if (currentJobsIndex+1 < paramJobsLength) {
    currentJobsIndex++;
  } else {
    currentJobsIndex = 0;
  }
}

void sendHttpRequest(String jobname) {
  if (client.connected()) {
    return;
  }
  String ESClientPath = paramESClientPath;
  ESClientPath.replace(String("JOBNAME"), jobname);
  /*
  Serial.println(ESServer+String(":")+ESServerPort);
  Serial.print("GET ");
  Serial.print(ESClientPath);
  Serial.println(" HTTP/1.1");
  Serial.print("Host: ");
  Serial.println(ESServer);
  Serial.print("Authorization: Basic ");
  Serial.println(paramESClientAuthRealm);
  Serial.print("User-Agent: ");
  Serial.println(paramESClientUA);
  Serial.println("Connection: close");
  Serial.println();
  */
  Serial.println(F("Sending http request to server"));
  if (client.connect(ESServer, ESServerPort)) {
    client.print("GET ");
    client.print(ESClientPath);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(ESServer);
    client.print("Authorization: Basic ");
    client.println(paramESClientAuthRealm);
    client.print("User-Agent: ");
    client.println(paramESClientUA);
    client.println("Connection: close");
    client.println();
    Serial.println("connection success");
  } else {
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.flush();
    client.stop();
  }
  
}

void resetHttpResponse() {
  Serial.println("disconnecting.");
  client.stop();
  client.flush();
  httpResponseSearchStatus = false;
}

void readHttpResponse() {
  if (client.available() && !httpResponseSearchStatus) {
    httpResponseSearchStatus = true;
    int jobNameBufferLength = 32;
    char jobName[jobNameBufferLength];
    char jobNamePre[] = paramJobNamePre;
    char jobNamePost[] = paramJobNamePost;
    int jobNameLength = 0;
    int jobStatusBufferLength = 10;
    char jobStatus[jobStatusBufferLength];
    char jobStatusPre[] = paramJobStatusPre;
    char jobStatusPost[] = paramJobStatusPost;
    int jobStatusLength = 0;
    if( finder.find(paramHttpResponseOk) ) {
      Serial.println("http response OK");
      jobNameLength = finder.getString(jobNamePre, jobNamePost, jobName, jobNameBufferLength);
      jobStatusLength = finder.getString(jobStatusPre, jobStatusPost, jobStatus, jobStatusBufferLength);
      if (jobNameLength && jobStatusLength) {
        Serial.println("http response format OK");
        actionLedGroupToggle(decodeJobStatus(String(jobStatus)), decodeJobName(String(jobName)));
      } else {
        Serial.println("http response format KO");
      }
    } else {
      Serial.println("http response KO");
    }
    resetHttpResponse();
  }
  if (!client.connected() && ESClientLastConnected) {
    resetHttpResponse();
  }
}

void actionLedGroupToggle(int state, int pin) {
  Serial.println(String("state: ")+String(state)+String(", pin: ")+String(pin));
  boolean authorizedPin = false;
  for (int k = 0; k < paramLedsGroupsLength; k++) {
    if (pin == leds[k]) {
      authorizedPin = true;
      break;
    }
  }
  if (authorizedPin) {
    if (state == 0) {
      digitalWrite(pin, HIGH);
      digitalWrite(pin+paramLedsGroupsLength, LOW);
    } else {
      digitalWrite(pin, LOW);
      digitalWrite(pin+paramLedsGroupsLength, HIGH);
    }
  }
}

int decodeJobName(String jobName) {
  Serial.println(String("jobName: [")+jobName+String("]"));
  for (int k = 0; k < paramJobsLength; k++) {
    String _jobName = jobs[k];
    if (jobName.startsWith(_jobName)) {
      return leds[k];
    }
  }
}

int decodeJobStatus(String jobStatus) {
  Serial.println(String("jobStatus: [")+jobStatus+String("]"));
  if (jobStatus == "SUCCESS") {
    return 1;
  }
  return 0;
}
// helper functions end -----------------------------------

// main program setup begin -------------------------------
void setup() 
{
  setupSerial();
  setupHardware();
  setupEthernet();
  delay(1000);
}
// main program setup end ---------------------------------

// main program loop begin --------------------------------
void loop()
{
  if (DHCPStatus > 0) {
    unsigned long cM = millis();
    readHttpResponse();
    if (cM-pM > ESClientWait*1000) {
      pM = cM;
      //String jobname = "artefactlabs";
      //sendHttpRequest(jobname);
      jobAction();
    } else {
      // wait
      //Serial.print("Wait");
      //Serial.println(cM-pM);
    }
    ESClientLastConnected = client.connected();
  }
}
// main program loop end ----------------------------------

