#include <TheThingsNetwork.h>
                                  //includes libraries and functions
#include "voltSense.h"


///////////////////////////////////////////////////////////////////
//////////////////////////      INIT     //////////////////////////
///////////////////////////////////////////////////////////////////

/* FOR LORA COMMUNICATION */

#define loraSerial Serial1
#define debugSerial Serial

#define freqPlan TTN_FP_EU868 // frequency of european LoRa network

// "username/password" for our device to connect to network. To be found in the ttn console
const char *appEui = "70B3D57ED0010011";
const char *appKey = "BBABB7D4579913C4EC8407A8E3024FCB";

// parameters for sending messages over LoRa
const unsigned int intervMess = 60; // interval for sending "
unsigned long lastMess = 0;

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);
byte payload[3];


// identifier for the line and phase where our sensor is connected to
const unsigned int lineIdent = 1;
const unsigned int phaseIdent = 1;

const unsigned int input = A0; //analog input for measurment

// parameters for measurement
const unsigned long sampleTime = 100000UL;
const unsigned long numSamples = 250UL;
const unsigned long sampleInterval = sampleTime/numSamples;

float voltEff = 0; // actual measured voltage value
float storVal = 0; // stored previous voltage value
bool pwlOn = false; // power on/off




///////////////////////////////////////////////////////////////////
//////////////////////////     SETUP     //////////////////////////
///////////////////////////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:

  // initializing serial connections
  loraSerial.begin(57600);
  debugSerial.begin(9600);
  Serial.begin(115200);
  
  while (!debugSerial && millis() < 10000)
    ;

  // connecting to LoRa
  debugSerial.println("-- STATUS");
  ttn.showStatus();
  
  debugSerial.println("-- JOIN");
  ttn.join(appEui, appKey);

  // reference for analog input. DEFAULT is 5V internal voltage. Problem --> not constant
  analogReference(DEFAULT);

}



///////////////////////////////////////////////////////////////////
//////////////////////////     LOOP      //////////////////////////
///////////////////////////////////////////////////////////////////

void loop() {
  // put your main code here, to run repeatedly:
    
  voltEff = voltSense(input, sampleInterval, numSamples); // taking measurements
  Serial.println(voltEff); // printing measurements on serial monitor

//  byte w = (i - 4);
//  byte x = (i - 5);
//  byte y = (i - 6);




  if ((storVal * 0.01) > voltEff) {

    pwlOn = false;
    payload[0] = lineIdent;
    payload[1] = phaseIdent;
    payload[2] = pwlOn;
    debugSerial.println("-- SEND");
    ttn.sendBytes(payload, sizeof(payload));
    
  }


  if ((storVal / 0.01) < voltEff) {

    pwlOn = true;
    payload[0] = lineIdent;
    payload[1] = phaseIdent;
    payload[2] = pwlOn;
    debugSerial.println("-- SEND");
    ttn.sendBytes(payload, sizeof(payload));
    
  }

  
  storVal = voltEff;
  
  delay(500);

  
  if (millis() >= lastMess + (intervMess * 1000)) {

    payload[0] = lineIdent;
    payload[1] = phaseIdent;
    payload[2] = pwlOn;
    debugSerial.println("-- SEND");
    ttn.sendBytes(payload, sizeof(payload));
    lastMess = millis();
    
  }

}

