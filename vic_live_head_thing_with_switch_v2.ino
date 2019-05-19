/* get accelerate data of H3LIS331DL
 * Auth : lawliet(lawliet.zou@gmail.com)
 * version : 0.1
 */
 /*
 * MIDIUSB_test.ino
 *
 * Created: 4/6/2015 10:47:08 AM
 * Author: gurbrinder grewal
 * Modified by Arduino LLC (2015)
 */ 
#include <H3LIS331DL.h> //superclass for accelerometer

#include <H3LIS331DL.h>
#include <Wire.h>

//please get these value by running H3LIS331DL_AdjVal Sketch.
#define VAL_X_AXIS  164
#define VAL_Y_AXIS  11
#define VAL_Z_AXIS  1007

H3LIS331DL h3lis; //constructor for accelerometer

#include "MIDIUSB.h"

//setup for averaging
const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int inputPin = A0;


void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  Serial.begin(115200);
  h3lis.init();
  h3lis.importPara(VAL_X_AXIS,VAL_Y_AXIS,VAL_Z_AXIS);
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void loop() {
  int16_t x,y,z;
  h3lis.readXYZ(&x,&y,&z);

  // this section averages and smooths out readings from accelerometer
  total = total - readings[readIndex];
  readings[readIndex] = y;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  average = total / numReadings;

  // this sensor value takes readings from the LDR  
  int sensorValue0 = map(analogRead(A0), 1023, 700, 0, 127);
  //Serial.println(analogRead(A0));
  //Serial.println(sensorValue0);

  // this sensor value takes readings from the accelerometer
  int sensorValue1 = map(average, -350, 350, 110, 35);
  //Serial.println(sensorValue1);

  // this sensor value reads the pedal status
  int pedalVal = (int)analogRead(A3);
  //Serial.println(pedalVal);

  // this sends a continuous stream on MIDI notes when the pedal is pressed
  if (pedalVal != 1023) {
  //note sender
    noteOn(0, sensorValue1, 64);   // Channel 0, middle C, normal velocity
    MidiUSB.flush();
    delay(125);
    //Serial.println("Sending note off");
    noteOff(0, sensorValue1, 64);  // Channel 0, middle C, normal velocity
    MidiUSB.flush();
    delay(50);
  }

  // this sends the mouthpiece readings as MIDI CC
  if (sensorValue0 > 0)
  {
  controlChange(0, 10, sensorValue0); // Set the value of controller 10 on channel 0 to 65
  //Serial.println(sensorValue0);
  MidiUSB.flush();
  }

  delay(5);
 
}
