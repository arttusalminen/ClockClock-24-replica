#include <Wire.h>
#define ADDRESS            42

String clocktime = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(ADDRESS);
  Wire.onReceive(receiveEvent);
}


void receiveEvent(int howMany) {

  String data = "";
  while( Wire.available()){
    data += (char)Wire.read();
  }

  int clockNumber = ((String)data[0]).toInt();
  int clockPos = ((String)data[2]+(String)data[3]+(String)data[4]+(String)data[5]).toInt();

  Serial.println("Kello numero: " + String(clockNumber) + ".");
  Serial.println("Kellon asento: " + String(clockPos) + ".");
  Serial.println(data);
  clocktime = data;
}

void loop() {
  // put your main code here, to run repeatedly:
  
  delay(300);
}
