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
const unsigned int intervMess = 5; // interval for sending
unsigned long lastMess = 0; // variable to store timestamp of last message sent

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);
byte payload[3]; // array to store message


// identifier for the line and phase where our sensor is connected to
const unsigned int lineIdent = 1;
const unsigned int phaseIdent = 1;

const unsigned int input = A0; //analog input for measurment

// parameters for measurement
const unsigned long sampleTime = 100000UL;
const unsigned long numSamples = 250UL;
const unsigned long sampleInterval = sampleTime/numSamples;

float voltEff = 0; // actual measured voltage value during normal voltage state
byte i = 0; // variable to count, assign and store values for storVal array 
float storVal[10]; // stored previous voltage values
bool pwlOn = true; // power on/off
float voltDrop = 0; // actual measured voltage during voltage drop state
float avVoltDrop = 0; // average of previous voltage values during voltage drop situation




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

  // creating and sending of first message
  payload[0] = lineIdent;
  payload[1] = phaseIdent;
  payload[2] = pwlOn;
  debugSerial.println("-- SEND");
  ttn.sendBytes(payload, sizeof(payload));

  // reference for analog input. DEFAULT is 5V internal voltage. Problem --> not constant
  analogReference(DEFAULT);

  // filling the storVal array with measurement values
  for (byte k = 0; k <= 9; k++) {
    voltEff = voltSense(input, sampleInterval, numSamples); // taking measurements
    storVal[k] = voltEff;
    delay(500);
    }

}



///////////////////////////////////////////////////////////////////
//////////////////////////     LOOP      //////////////////////////
///////////////////////////////////////////////////////////////////

void loop() {
// put your main code here, to run repeatedly:
 
  float avStorVal = 0; // setting the average of all values stored in storVal to 0

  // calculating new average of all values stored in storVal
  for (byte j = 0; j <= 9; j++) {
    
    avStorVal = avStorVal + storVal[j];
    
  }

  avStorVal = avStorVal / 10;

  // taking measurements and printing them on serial monitor
  voltEff = voltSense(input, sampleInterval, numSamples); 
  Serial.println(voltEff); 

  // comparing new measurement against average of measurements in storVal
  if ((avStorVal * 0.05) > voltEff) {

    float storVoltDrop[3];  // new array for measurements taken after voltage drop
    byte dropCount = 0; // counter for measurements after voltage drop

    // taking 3 measurements 
    for (byte p = 0; p <= 2; p++) {
      
      voltDrop = voltSense(input, sampleInterval, numSamples); // taking measurements

      Serial.println(voltDrop);
      delay(500);

      // if taken measurements are still on very low voltage level (voltage drop), put them into storVoltDrop array
      if ((avStorVal * 0.05) > voltDrop) {

        dropCount = dropCount + 1;
        storVoltDrop[p] = voltDrop;
        
      }      
      
    }

    // calculation of average of values in storVoltDrop array
    avVoltDrop = 0;
    
    for (byte g = 0; g <= 2; g++) {

      avVoltDrop = avVoltDrop + storVoltDrop[g];
        
    }

    avVoltDrop = avVoltDrop / 3;

    // if 3 measurements fullfill the voltage drop condition, a alarm message is sent through LoRa and state of the system is changed to "false"
    if (dropCount == 3 && pwlOn == true) {
      
      pwlOn = false;
      payload[0] = lineIdent;
      payload[1] = phaseIdent;
      payload[2] = pwlOn;
      debugSerial.println("-- SEND");
      ttn.sendBytes(payload, sizeof(payload));
      lastMess = millis();    

    }

    
    // if voltage measured is increasing again, message is set through LoRa and state of system is changed to "true"
    if ((voltDrop * 0.05) > avVoltDrop && pwlOn == false) {

      pwlOn = true;
      payload[0] = lineIdent;
      payload[1] = phaseIdent;
      payload[2] = pwlOn;
      debugSerial.println("-- SEND");
      ttn.sendBytes(payload, sizeof(payload));
      lastMess = millis();
    
    }
  
  }

  // if voltage measured is increasing again, message is set through LoRa and state of system is changed to "true"
  if ((voltEff * 0.05) > avVoltDrop && pwlOn == false) {

    pwlOn = true;
    payload[0] = lineIdent;
    payload[1] = phaseIdent;
    payload[2] = pwlOn;
    debugSerial.println("-- SEND");
    ttn.sendBytes(payload, sizeof(payload));
    lastMess = millis();
    
  }

  // if no message has been sent during the timed defined in intervMess, a message is being sent with the current state of the system ("true"/"false")
  if (millis() >= lastMess + (intervMess * 1000)) {

    payload[0] = lineIdent;
    payload[1] = phaseIdent;
    payload[2] = pwlOn;
    debugSerial.println("-- SEND");
    ttn.sendBytes(payload, sizeof(payload));
    lastMess = millis();
    
  }

  // storage of values in storVal array during normal measurement cycle (no voltge drop) and incrementing of counter (or reseting in case of reaching the limit)
  if (pwlOn == true) {
    
      storVal[i] = voltEff;
      i++;

        if (i > 9) {
          
          i = 0;
          
        }
          
    }

    delay(500);

}

