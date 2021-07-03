#include "Arduino.h"
/*
******************************************************************************************************************************************************************
*
* Markisol protocol remote control capture
* Compatible with remotes like BF-101, BF-301, BF-305, possibly others
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* Use this code to capture the commands from your remotes. Outputs to serial
* (Tools -> Serial Monitor). What you need for mastering your shades are 41
* bits commands. Protocol is described in Markisol.ino.
* 
* 
* HOW TO USE
* 
* Plug a 433.92MHz receiver to digital pin 2 and start pressing buttons
* from your original remotes (copy pasting them to Markisol.ino).
* 
* 
* NOTE ABOUT THE L (LIMITS) BUTTON
* 
* You have to hold down this button for 6 seconds before the remote
* starts transmitting.
*
******************************************************************************************************************************************************************
*/



// Plug your 433.92MHz receiver to digital pin 2:
#define RECEIVE_PIN   3

// Enable debug mode if there's no serial output or if you're modifying this code for
// another protocol/device. However, note that serial output delays receiving, causing
// data bit capture to fail. So keep debug disabled unless absolutely required:
#define DEBUG         false
#define ADDITIONAL    false    // Display some additional info after capture


unsigned long t1;
unsigned long t2;
unsigned long t3;

unsigned long sum_t1;
unsigned long sum_t0;
unsigned long count_t1;
unsigned long count_t0;
unsigned long avg_t1;
unsigned long avg_t0;


// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
int convertStringToInt(String s) {
  char carray[2];
  int i = 0;
  
  s.toCharArray(carray, sizeof(carray));
  i = atoi(carray);

  return i;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
long convertBinaryStringToInt(String s) {
  int c = 0;
  long b = 0;
  
  for (int i = 0; i < s.length(); i++) {
    c = convertStringToInt(s.substring(i, i + 1));
    b = b << 1;
    b += c;
  }
  
  return b;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printChannel(String channel) {

  int c = convertBinaryStringToInt(channel);

  switch (c) {

    case 0x8: // 1000
      return "1"; // ...or remote model BF-301

    case 0x4: // 0100
      return "2"; // ...or remote model BF-101

    case 0xC: // 1100
      return "3";

    case 0x2: // 0010
      return "4";

    case 0xA: // 1010
      return "5";

    case 0x6: // 0110
      return "6";
    
    case 0xE: // 1110
      return "7";
    
    case 0x1: // 0001
      return "8";
    
    case 0x9: // 1001
      return "9";

    case 0x5: // 0101
      return "10";
    
    case 0xD: // 1101
      return "11";
    
    case 0x3: // 0011
      return "12";
    
    case 0xB: // 1011
      return "13";

    case 0x7: // 0111
      return "14";

    case 0xF: // 1111
      return "ALL";

  }
  return "UNKNOWN/NEW (" + channel + ")";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printCommand(String command) {

  int c = convertBinaryStringToInt(command);

  switch (c) {
    
    case 0x8: // 1000
      return "DOWN";

    case 0x3: // 0011
      return "UP";

    case 0xA: // 1010
      return "STOP";

    case 0x2: // 0010
      return "PAIR/CONFIRM (C)";

    case 0x4: // 0100
      return "PROGRAM LIMITS (L)";

    case 0x1: // 0001
      return "CHANGE DIRECTION OF ROTATION (STOP + L)";

  }
  return "UNKNOWN";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printRemoteModel(String model) {

  int c = convertBinaryStringToInt(model);

  switch (c) {

    case 0x86: // 10000110
      return "BF-305 (5 channels)";

    case 0x3: // 00000011
      return "BF-101 (single channel)";

    case 0x83: // 10000011
      return "BF-301 (single channel)";
    
    case 0x80: // 10000000
      return "BF-315 (14 channel)";
  }
  return "UNKNOWN/NEW (" + model + ")";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  pinMode(RECEIVE_PIN, INPUT);
  Serial.begin(9600);
  Serial.println("Starting up...");
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  int i = 0;
  unsigned long t = 0;
  String command = "";


  // *********************************************************************
  // Wait for the first AGC:
  // *********************************************************************
  // HIGH between 4500-6000 us
  // *********************************************************************
  
  while (t < 4500 || t > 6000) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // / Waits for HIGH and times it
    
    if (DEBUG) { // For finding AGC timings
      if (t > 3000 && t < 7000) Serial.println(t);
    }
  }
  t1 = t;

  if (DEBUG) {
    Serial.print("AGC 1: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Wait for the second AGC:
  // *********************************************************************
  // LOW between 2300-2600 us
  // *********************************************************************
  
  while (t < 2300 || t > 2600) {
    t = pulseIn(RECEIVE_PIN, LOW, 1000000); // / Waits for LOW and times it
    
    if (DEBUG) { // For finding AGC timings
      if (t > 2300 && t < 2600) Serial.println(t);
    }
  }

  t2 = t;

  if (DEBUG) {
    Serial.print("AGC 2: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Wait for the third AGC:
  // *********************************************************************
  // HIGH between 1100-1900 us
  // *********************************************************************
  
  while (t < 1100 || t > 1900) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // Waits for HIGH and times it

    if (DEBUG) { // For finding AGC timings
      if (t > 500 && t < 3000) Serial.println(t);
    }
  }
  t3 = t;

  if (DEBUG) {
    Serial.print("AGC 3: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Command bits, locate them simply by HIGH waveform spikes:
  // *********************************************************************  
  // 0 = 200-400 us
  // 1 = 500-800 us
  // *********************************************************************

  sum_t1 = 0;
  sum_t0 = 0;
  count_t1 = 0;
  count_t0 = 0;
  avg_t1 = 0;
  avg_t0 = 0;

  while (i < 41) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // Waits for HIGH and times it
    
    if (DEBUG) {
      Serial.print(t);
      Serial.print(": ");
    }

    if (t > 500 && t < 800) { // Found 1
      command += "1";
      sum_t1 += t;
      count_t1++;
      if (DEBUG) Serial.println("1");
      
    } else if (t > 200 && t < 400) { // Found 0
      command += "0";
      sum_t0 += t;
      count_t0++;
      if (DEBUG) Serial.println("0");
      
    } else { // Unrecognized bit, finish
      if (ADDITIONAL) {
        Serial.print("INVALID TIMING BIT ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(t);
      }
      i = 0;
      break;
    }

    i++;
  }

  // *********************************************************************
  // Done! Display results:
  // *********************************************************************  

  // Correct data bits length is 41 bits, dismiss bad captures:
  if (command.length() < 40) {
    
    if (ADDITIONAL) {
      Serial.print("Bad capture, invalid command length ");
      Serial.println(command.length());
      Serial.println();
    }
    
  } else {
    Serial.println("Successful capture, full command is: " + command);
    Serial.println("Remote control (unique) ID: " + command.substring(0, 16));
    Serial.println("Channel: " + printChannel(command.substring(16, 20)));
    Serial.println("Command: " + printCommand(command.substring(20, 24)));
    Serial.println("Remote control model: " + printRemoteModel(command.substring(24, 32)));
    
    Serial.print("AGC1:" );
    Serial.println(t1);
    Serial.print("AGC2:" );
    Serial.println(t2);
    Serial.print("AGC3:" );
    Serial.println(t3);

    avg_t1 = sum_t1/count_t1;
    avg_t0 = sum_t0/count_t0;
    Serial.print("Avg for 0:" );
    Serial.println(avg_t0);
    Serial.print("Avg for 1:" );
    Serial.println(avg_t1);

    if (ADDITIONAL) {
      Serial.print("Remote control ID (DEC): ");
      Serial.println(convertBinaryStringToInt(command.substring(0, 16)), DEC);
      Serial.print("Trailing bits: ");
      Serial.println(convertBinaryStringToInt(command.substring(32, 41)), DEC);
    }
    Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

