#include <IRremote.h>

int LED_PIN = 3;
//int RECV_PIN = 11;

IRsend irsend;
//IRrecv irrecv(RECV_PIN);

//decode_results results;

void setup()
{
  Serial.begin(9600);
  //irrecv.enableIRIn(); // Start the receiver
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  /*if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }*/
  
  // ON
  Serial.println("sending ON F7C03F");
  irsend.sendNEC(0xF7C03F, 32);
  delay(5000);
  
  // WHITE
  Serial.println("sending WHITE F7E01F");
  irsend.sendNEC(0xF7E01F, 32);
  delay(5000);
  
  // RED
  Serial.println("sending RED F720DF");
  irsend.sendNEC(0xF720DF, 32);
  delay(5000);
  
  // GREEN
  Serial.println("sending GREEN F7A05F");
  irsend.sendNEC(0xF7A05F, 32);
  delay(5000);
  
  // BLUE
  Serial.println("sending BLUE F7609F");
  irsend.sendNEC(0xF7609F, 32);
  delay(5000);
  
  // OFF
  Serial.println("sending OFF F740BF");
  irsend.sendNEC(0xF740BF, 32);
  irsend.sendNEC(0xF740BF, 32);
  delay(5000);
  
}
