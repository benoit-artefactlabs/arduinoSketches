int leds[] = {2,3,4,5,6,7};

void testLeds(boolean state) {
  int kl = sizeof(leds);
  for (int k = 0; k < kl; k++) {
    if (state) {
      digitalWrite(k, HIGH);
    } else {
      digitalWrite(k, LOW);
    }
  }
}

void setup() {
  int kl = sizeof(leds);
  for (int k = 0; k < kl; k++) {
    pinMode(k, OUTPUT);
  }
  testLeds(false);
}

void loop() {
  testLeds(true);
  delay(1000);
  testLeds(false);
  delay(1000);
}
