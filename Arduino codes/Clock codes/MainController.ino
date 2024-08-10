#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <Wire.h>
#include <ctype.h>

#define ATMEGAS_IN_NUMBER   3

// Wifi:
char ssid[] = SECRET_ssid;
char pass[] = SECRET_pass;
int keyIndex = 0;
int status = WL_IDLE_STATUS;

WiFiServer server(80);

/**
	The clock face looks like this:
	 1  o o  o o  o o  o o			up
	 2  o o  o o  o o  o o			mid
	 3  o o  o o  o o  o o			down
		 1    2    3    4

	The numbers define controller i2c addresses: first number corresponds to the number at the bottom row and 
	second to the number at the left column.

	Each controller controls 2 small clockfaces, so 4 indicators = steppers. So for example controller 32:
	 1  o o  o o  o o  o o			up
	|2| o o  o o  x x  o o			mid
	 3  o o  o o  o o  o o			down
		 1    2   |3|   4
*/

// I2C addresses for each controller
int hoursfirst328 [3] = {11, 12, 13};
int hourssecond328 [3] = {21, 22, 23};
int minutesfirst328 [3] = {31, 32, 33};
int minutessecond328 [3] = {41, 42, 43};

int allAddresses[12] = {11,12,13,21,22,23,31,32,33,41,42,43};


/** 
	--- TARGET POSITIONS FOR STEPPERS ---
 Order of the 4 targets: 2 clock faces per controller -> 4 indicators:
 { left min indicator, left hour indicator, right min indicator, right hour indicator }
*/

// Numbers, 0 thorugh 9.
// Top row of controllers
int positions_up[10][4] =     {{2880, 1440, 4320, 2880},
                              {3600, 3600, 2880, 2880},
                              {1440, 1440, 2880, 4320},
                              {1440, 1440, 2880, 4320},
                              {2880, 2880, 2880, 2880},
                              {2880, 1440, 4320, 4320},
                              {2880, 1440, 4320, 4320},
                              {1440, 1440, 4320, 2880},
                              {2880, 1440, 4320, 2880},
                              {2880, 1440, 4320, 2880}};
// Mid row
int positions_mid[10][4] =    {{0, 2880, 0, 2880},
                              {3600, 3600, 0, 2880},
                              {1440, 2880, 4320, 0},
                              {1440, 1440, 0, 4320},
                              {1440, 0, 0, 2880},
                              {1440, 0, 4320, 2880},
                              {0, 2880, 4320, 2880},
                              {3600, 3600, 0, 2880},
                              {1440, 0, 0, 4320},
                              {1440, 0, 0, 2880}};
// Bottom row
int positions_down[10][4] =   {{1440, 0, 4320, 0},
                              {3600, 3600, 0, 0},
                              {0, 1440, 4320, 4320},
                              {1440, 1440, 0, 4320},
                              {3600, 3600, 0, 0},
                              {1440, 1440, 0, 4320},
                              {1440, 0, 0, 4320},
                              {3600, 3600, 0, 0},
                              {0, 1440, 0, 4320},
                              {1440, 1440, 0, 4320}};

// Shapes, targets for all 12 controllers:
int pointMiddle[12][4] =   {{1695,1695,1789,1789},
                            {1440,1440,1440,1440},
                            {1185,1185,1091,1091},
                            {1979,1979,2455,2455},
                            {1440,1440,1440,1440},
                            {901,901,425,425},
                            {3305,3305,3781,3781},
                            {4320,4320,4320,4320},
                            {5335,5335,4863,4863},
                            {3971,3971,4065,4065},
                            {4320,4320,4320,4320},
                            {4669,4669,4575,4575}}; 

int waves[12][4] =          {{960,3840,4800,1920},
                            {960,3840,4800,1920},
                            {960,3840,4800,1920},
                            {960,3840,4800,1920},
                            {960,3840,4800,1920},
                            {960,3840,4800,1920},
                            {960,3840,4800,1920},
                            {960,3840,4800,1920},
                            {960,3840,4800,1920},
                            {960,3840,4800,1920},
                            {960,3840,4800,1920},
                            {960,3840,4800,1920}};

int pointDown[12][4] =      {{2880,2880,2880,2880},
                            {2880,2880,2880,2880},
                            {2880,2880,2880,2880},
                            {2880,2880,2880,2880},
                            {2880,2880,2880,2880},
                            {2880,2880,2880,2880},
                            {2880,2880,2880,2880},
                            {2880,2880,2880,2880},
                            {2880,2880,2880,2880},
                            {2880,2880,2880,2880},
                            {2880,2880,2880,2880},
                            {2880,2880,2880,2880}};
                           

struct clockinfo{
  volatile unsigned int hoursfirst_;
  volatile unsigned int hourssecond_;
  volatile unsigned int minutesfirst_;
  volatile unsigned int minutessecond_;

  volatile unsigned int hoursfirstprevious_;
  volatile unsigned int hourssecondprevious_;
  volatile unsigned int minutesfirstprevious_;
  volatile unsigned int minutessecondprevious_;
};

volatile bool receiving = false;
volatile bool staticShape = true;
volatile int gradualUpdateTimeGap = 1200;
volatile bool gradualUpdate = true;
volatile bool inSync = false;

unsigned long minute = 59905;
volatile unsigned long time_now = 0;

volatile clockinfo kello;




void sendData(int receiver, String stringToSend){
  Wire.beginTransmission(receiver);
  for (int i = 0; i < (int)stringToSend.length(); i++){
    Wire.write(stringToSend[i]);
  }
  Wire.endTransmission();
  Serial.println(String(receiver) + " -> " + stringToSend);
}

void sendReqularPosDataToEntireNumber(int numToDisplay, int addresses[3]) {
  for (int i = 0; i < ATMEGAS_IN_NUMBER; i++)
  {
    int pos[10][4];
    if (i == 0) { memcpy(pos, positions_up, sizeof(positions_up));}
    else if (i == 1) {memcpy(pos, positions_mid, sizeof(positions_mid));}
    else if (i == 2) {memcpy(pos, positions_down, sizeof(positions_down));}

    for (int c = 0; c < 4; c++) {
      sendData(addresses[i], String(c) + ":" + String(pos[numToDisplay][c]));
    }
  }
}

void sendBeginMovementCommandToDigit(int digitAddresses[3]) {
  for (int i = 0; i < ATMEGAS_IN_NUMBER; i++) {
    sendData(digitAddresses[i], "move");
  }
}

void sendCommandToAll(String commandData) {
  for (int i = 0; i < 12; i++) {
      sendData(allAddresses[i], commandData);
    }
}

void sendCalculateTimeCommands() {
  sendCommandToAll("req:m_t");
}

void sendDriveInSyncCommands() {
  sendCommandToAll("sync");
}

float getCalculatedMaxTime() {
  float maxTime = 0;

  for(int i = 0; i < 12; i++) {
    Wire.requestFrom(allAddresses[i], 15);
    String data = "";

    while (Wire.available()) {
      char c = Wire.read();
      data += c;
    }

    float f = data.toFloat();
    if (f > maxTime) { maxTime = f; }
  }

  return maxTime;
}

void sendCalculatedMaxTime(float time) {
  sendCommandToAll("t:" + String(time));
}

void sendPrepareMovementCommands() {
  sendCommandToAll("prepare");
}

void sendBeginMovementCommands() {
  sendBeginMovementCommandToDigit(minutessecond328);
  if (gradualUpdate) { delay(gradualUpdateTimeGap); }
  sendBeginMovementCommandToDigit(minutesfirst328);
  if (gradualUpdate) { delay(gradualUpdateTimeGap); }
  sendBeginMovementCommandToDigit(hourssecond328);
  if (gradualUpdate) { delay(gradualUpdateTimeGap); }
  sendBeginMovementCommandToDigit(hoursfirst328);
}



void setup() {
  Serial.begin(9600);

  // Check wifi module status:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // Create accesspoint:
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }

  // Default time = when this was compiled.
  char T[] = __TIME__;
  uint8_t oldSREG = SREG;
  cli();
  unsigned long temp = (unsigned long)((T[0]*10+T[1]-528)*60+T[3]*10+T[4]-528)*60
    +T[6]*10+T[7]-528;
  unsigned long timer0_millis = (unsigned long)temp*1000;
  SREG = oldSREG;

  kello.hoursfirst_ = T[0] - 48;
  kello.hourssecond_ = T[1] - 48;
  kello.minutesfirst_ = T[3] - 48;
  kello.minutessecond_ = T[4] - 48;

  Wire.begin();

  Serial.println(__TIME__);

  // Wait for all atmegas to start up.
  delay(4000);

  // Send times.
  //sendReqularPosDataToEntireNumber(kello.hoursfirst_, hoursfirst328);
  //sendReqularPosDataToEntireNumber(kello.hourssecond_, hourssecond328);
  //sendReqularPosDataToEntireNumber(kello.minutesfirst_, minutesfirst328);
  //sendReqularPosDataToEntireNumber(kello.minutessecond_, minutessecond328);

  //kello.hoursfirstprevious_ = kello.hoursfirst_;
  //kello.hourssecondprevious_ = kello.hourssecond_;
  //kello.minutesfirstprevious_ = kello.minutesfirst_;
  //kello.minutessecondprevious_ = kello.minutessecond_;
  
  delay(2000);
  // start the web server on port 80
  server.begin();
  // you're connected now, so print out the status
  printWiFiStatus();
}


void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}


void updateClock(){

  kello.minutessecond_ += 1;
  if (kello.minutessecond_ == 10)
  {
    kello.minutessecond_ = 0;
    kello.minutesfirst_ += 1;

    if (kello.minutesfirst_ == 6)
    {
      kello.minutesfirst_ = 0;
      kello.hourssecond_ += 1;
      //WiFi.end();
      //delay(500);
      //status = WiFi.beginAP(ssid, pass);
      //delay(500);
      //server.begin();

      if (kello.hourssecond_ == 10)
      {
        kello.hourssecond_ = 0;
        kello.hoursfirst_ += 1;
      }

      else if (kello.hoursfirst_ == 2 and kello.hourssecond_ == 4)
      {
        kello.hoursfirst_ = 0;
        kello.hourssecond_ = 0;
      }
    }
  }
}


  // Loop has 2 parts: wifi communication, and normal clock updating.
void loop(){

  bool timeUpdateRequested = false;
  bool parameterUpdateRequested = false;
  bool shapeUpdateRequested = false;

  String parameterChar = "";
  String parameterVal = "";
  String shapeName = "";


  // Wifi communication:
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();
    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    }
  }

  WiFiClient client = server.available();   // listen for incoming clients
  
  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";               // make a String to hold incoming data from the client
    bool connectionSuccessful = false;

    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
 
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            if (connectionSuccessful) {
              client.println("HTTP/1.1 202 Accepted");
              client.println("Content-type:text/html");
              client.println();

              // the content of the HTTP response follows the header:
              client.print("Hiphei<br>");

              // The HTTP response ends with another blank line:
              client.println();

              // break out of the while loop:
              break;
            }
            else {
              client.println("HTTP/1.1 400 BadRequest");
              client.println("Content-type:text/html");
              client.println();

              // the content of the HTTP response follows the header:
              client.print("Ikävä tunnelma...<br>");

              // The HTTP response ends with another blank line:
              client.println();

              // break out of the while loop:
              break;
            }
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }

        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":

        if (currentLine.endsWith("ready")) {
          int beginIndex = currentLine.lastIndexOf('/') + 1;
          int endIndex = currentLine.length() - 5;

          String data = currentLine.substring(beginIndex,endIndex);

    // Clocktime was sent:
          if (isDigit(data[0]) && data.length() == 4) {
            kello.hoursfirst_ = ((String)data[0]).toInt();
            kello.hourssecond_ = ((String)data[1]).toInt();
            kello.minutesfirst_ = ((String)data[2]).toInt();
            kello.minutessecond_ = ((String)data[3]).toInt();

            timeUpdateRequested = true;
            connectionSuccessful = true;
          }

    // New step interval value was sent:
          else if(data.indexOf("stepinterval") > -1) {
            beginIndex = data.lastIndexOf(':') + 1;
            endIndex = data.length();
            String value = data.substring(beginIndex,endIndex);

            parameterChar = "s";
            parameterVal = value;
            parameterUpdateRequested = true;
            connectionSuccessful = true;
          }

    // New acceleration value was sent:
          else if(data.indexOf("acceleration") > -1) {
            beginIndex = data.lastIndexOf(':') + 1;
            endIndex = data.length();
            String value = data.substring(beginIndex,endIndex);

            parameterChar = "a";
            parameterVal = value;
            parameterUpdateRequested = true;
            connectionSuccessful = true;
          }
    // Pattern:
          else if(data.indexOf("pattern") > -1) {
            beginIndex = data.lastIndexOf(':') + 1;
            endIndex = data.length();
            
            shapeName = data.substring(beginIndex,endIndex);
            shapeUpdateRequested = true;
            connectionSuccessful = true;
          }
    // Sync:
          else if(data.indexOf("sync") > -1) {
            if(data.indexOf("On") > -1) {
              inSync = true;
              connectionSuccessful = true;
            }
            else if(data.indexOf("Off") > -1) {
              inSync = false;
              connectionSuccessful = true;
            }
          }
    // Extra laps per movement:
          else if(data.indexOf("laps") > -1) {
            if(data.indexOf(":") > -1){
              beginIndex = data.lastIndexOf(':') + 1;
              endIndex = data.length();
              String value = data.substring(beginIndex,endIndex);
              if (value.toInt() > 0 || value == "0") {
                parameterChar = "l";
                parameterVal = value;
                parameterUpdateRequested = true;
                connectionSuccessful = true;
              }
            }
          }
    // Gradual:
          else if(data.indexOf("gradual") > -1) {
            if(data.indexOf("On") > -1) {
              gradualUpdate = true;
              connectionSuccessful = true;
            }
            else if(data.indexOf("Off") > -1) {
              gradualUpdate = false;
              connectionSuccessful = true;
            }
            else if(data.indexOf(":") > -1){
              beginIndex = data.lastIndexOf(':') + 1;
              endIndex = data.length();
              String value = data.substring(beginIndex,endIndex);
              if (value.toInt() > 0) {
                gradualUpdateTimeGap = value.toInt();
                connectionSuccessful = true;
              }
            }
          }
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }




  // Time change requested
  if (timeUpdateRequested) {
    staticShape = false;
    time_now = millis();

    // Send positions:
    sendReqularPosDataToEntireNumber(kello.minutessecond_, minutessecond328);
    sendReqularPosDataToEntireNumber(kello.minutesfirst_, minutesfirst328);
    sendReqularPosDataToEntireNumber(kello.hourssecond_, hourssecond328);
    sendReqularPosDataToEntireNumber(kello.hoursfirst_, hoursfirst328);

    // Prep movement:
    sendPrepareMovementCommands();
    delay(700);

    // Additionally, if clock hands are driven in sync:
    if (inSync) {
      float t = getCalculatedMaxTime();
      sendCalculatedMaxTime(t);
      sendDriveInSyncCommands();
      delay(80);
    }

    sendBeginMovementCommands();

    Serial.println("Clock update requested via http sent: " + String(kello.hoursfirst_) 
                                                  + String(kello.hourssecond_) + ":"
                                                  + String(kello.minutesfirst_)
                                                  + String(kello.minutessecond_));

    kello.hoursfirstprevious_ = kello.hoursfirst_;
    kello.hourssecondprevious_ = kello.hourssecond_;
    kello.minutesfirstprevious_ = kello.minutesfirst_;
    kello.minutessecondprevious_ = kello.minutessecond_;

  }

  // Shape requested
  if (shapeUpdateRequested) {
    staticShape = true;
    int pos[12][4];         // get clock hand directions from this

    // Chooce the pattern
    if (shapeName == "pointMiddle") {
      memcpy(pos, pointMiddle, sizeof(pointMiddle));
      Serial.println("Point to middle - pattern selected.");
    }

    else if (shapeName == "waves") {
      memcpy(pos, waves, sizeof(waves));
      Serial.println("Waves - pattern selected.");
    }

    else if (shapeName == "pointDown") {
      memcpy(pos, pointDown, sizeof(pointDown));
      Serial.println("Point down - pattern selected.");
    }
    

    // If wasn't a valid pattern, point middle
    else {
      memcpy(pos, pointMiddle, sizeof(pointMiddle));
      Serial.println("Pattern name not valid -> point to middle - pattern selected.");
    }

    // Send the static shape pos data
    for (int i = 0; i < 12; i++) {
      for (int c = 0; c < 4; c++) {
        String data = String(c) + ":" + String(pos[i][c]);
        sendData(allAddresses[i], data);
      }
    }

    // Prep
    sendPrepareMovementCommands();
    delay(700);

    // Additionally, if clock hands are driven in sync:
    if (inSync) {
      float t = getCalculatedMaxTime();
      sendCalculatedMaxTime(t);
      sendDriveInSyncCommands();
      delay(80);
    }

    sendBeginMovementCommands();
  }


  // Parameter change requested
  if (parameterUpdateRequested) {
    Serial.println("Parameter change requested.");
    for (int i = 0; i < 12; i++) {
      sendData(allAddresses[i], parameterChar + ":" + parameterVal);
    }
  }

  timeUpdateRequested = false;
  parameterUpdateRequested = false;
  shapeUpdateRequested = false;


  // Update the time on the clock:
  if ((unsigned long)(millis() - time_now) > minute && !staticShape) {
    time_now = time_now + minute;

    // Update what numbers to send:
    updateClock();

    // Send positions:
    sendReqularPosDataToEntireNumber(kello.minutessecond_, minutessecond328);
    sendReqularPosDataToEntireNumber(kello.minutesfirst_, minutesfirst328);
    sendReqularPosDataToEntireNumber(kello.hourssecond_, hourssecond328);
    sendReqularPosDataToEntireNumber(kello.hoursfirst_, hoursfirst328);

    // Prep movement:
    sendPrepareMovementCommands();
    delay(700);

    // Additionally, if clock hands are driven in sync:
    if (inSync) {
      float t = getCalculatedMaxTime();
      sendCalculatedMaxTime(t);
      sendDriveInSyncCommands();
      delay(80);
    }

    sendBeginMovementCommands();


    Serial.println("Regular clock updates sent: " + String(kello.hoursfirst_) 
                                                  + String(kello.hourssecond_) + ":"
                                                  + String(kello.minutesfirst_)
                                                  + String(kello.minutessecond_));

    kello.hoursfirstprevious_ = kello.hoursfirst_;
    kello.hourssecondprevious_ = kello.hourssecond_;
    kello.minutesfirstprevious_ = kello.minutesfirst_;
    kello.minutessecondprevious_ = kello.minutessecond_;
  }
  delay(300);
}
