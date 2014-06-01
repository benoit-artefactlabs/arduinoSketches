const int ledPin = 9;
const int frameLength = 24;
int readValue = 16;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}

void loop() {
  if (Serial.available() == frameLength) {
    byte parts[frameLength];
    int c = 0;
    while(Serial.available()) {
      parts[c] = Serial.read();
      Serial.print(parts[c]);
      //Serial.print(Serial.read(), HEX);
      Serial.print(", ");
      c++;
    }
    if (c == frameLength) {
      readValue = parts[frameLength-2];
      Serial.print("ACCEPT");
      if (readValue == 0) {
        digitalWrite(ledPin, LOW);
      } else {
        digitalWrite(ledPin, HIGH);
      }
    } else {
      Serial.print("DROP");
    }
    Serial.println();
    Serial.flush();
  }
}
