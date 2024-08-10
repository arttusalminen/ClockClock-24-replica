#include <Wire.h>

#define receiver 31


void setup() {
  // put your setup code here, to run once:
  Wire.begin();
}

int toSend = 1;
void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
  String stringToSend = "a:" + String(toSend);
  toSend += 1;
  Wire.beginTransmission(receiver);
  for (int i = 0; i < stringToSend.length(); i++){
    Wire.write(stringToSend[i]);
  }
  Wire.endTransmission();
}
