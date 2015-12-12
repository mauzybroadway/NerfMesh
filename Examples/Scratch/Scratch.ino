#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  pinMode(4,INPUT);
  digitalWrite(4, HIGH);
}

void loop() {
  int val;

  val = digitalRead(4);

  Serial.println(val);

  delay(10);
}
