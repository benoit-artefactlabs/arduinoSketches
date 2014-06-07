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
int tempValue = 0;
int tempValueIndex[] = {20, 21};
float tempVRef = 2.766;
int tempScale = 1024;
float tempCMax = 26.0;
float tempCMin = 19.0;

void setup() {
  Serial.begin(9600);
  pinMode(guardPin, OUTPUT);
  digitalWrite(guardPin, HIGH);
  pinMode(tempPinHigh, OUTPUT);
  digitalWrite(tempPinHigh, HIGH);
  pinMode(tempPinMedium, OUTPUT);
  digitalWrite(tempPinMedium, HIGH);
  pinMode(tempPinLow, OUTPUT);
  digitalWrite(tempPinLow, HIGH);
}

void loop() {
  if (Serial.available() == frameLength) {
    byte parts[frameLength];
    int c = 0;
    while(Serial.available()) {
      parts[c] = Serial.read();
      Serial.print(parts[c], HEX);
      //Serial.print(Serial.read(), HEX);
      Serial.print(", ");
      c++;
    }
    if (c == frameLength) {
      Serial.print(F("ACCEPT"));
      // guard sensor
      guardValue = parts[guardValueIndex-1];
      Serial.print(" ["); Serial.print(guardValue); Serial.print("]");
      if (guardValue == guardValueLow) {
        digitalWrite(guardPin, LOW);
      } else {
        digitalWrite(guardPin, HIGH);
      }
      // temp sensor
      int tempValue = (parts[tempValueIndex[0]-1]*256)+parts[tempValueIndex[1]-1];
      Serial.print(" ["); Serial.print(tempValue);
      float tempV = ((float)tempValue/(float)tempScale)*tempVRef;
      Serial.print(","); Serial.print(tempV);
      float tempC = (tempV-0.5)*100;
      Serial.print(","); Serial.print(tempC); Serial.print("]");
      if (tempC > tempCMax) {
        digitalWrite(tempPinHigh, HIGH);
        digitalWrite(tempPinMedium, LOW);
        digitalWrite(tempPinLow, LOW);
      } else if (tempC <= tempCMax && tempC > tempCMin) {
        digitalWrite(tempPinHigh, LOW);
        digitalWrite(tempPinMedium, HIGH);
        digitalWrite(tempPinLow, LOW);
      } else if (tempC <= tempCMin) {
        digitalWrite(tempPinHigh, LOW);
        digitalWrite(tempPinMedium, LOW);
        digitalWrite(tempPinLow, HIGH);
      } else {
        for (int li = 0; li < 2; li++) {
          digitalWrite(tempPinHigh, HIGH);
          digitalWrite(tempPinMedium, HIGH);
          digitalWrite(tempPinLow, HIGH);
          delay(250);
          digitalWrite(tempPinHigh, LOW);
          digitalWrite(tempPinMedium, LOW);
          digitalWrite(tempPinLow, LOW);
          delay(250);
        }
      }
    } else {
      Serial.print(F("DROP"));
    }
    Serial.println();
    Serial.flush();
  }
}
