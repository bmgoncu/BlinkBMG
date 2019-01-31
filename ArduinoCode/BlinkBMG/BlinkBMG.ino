#define USB_CFG_DEVICE_NAME     'D','i','g','i','B','l','i','n','k'
#define USB_CFG_DEVICE_NAME_LEN 9
#include <DigiUSB.h>

const int RED_PIN = 0;
const int GREEN_PIN = 1;
const int BLUE_PIN = 2;

byte in = 0;
int Blue = 0;
int Red = 0;
int Green = 0;

int next = 0;

// Initialize
void setup() {
  DigiUSB.begin();
  pinMode(RED_PIN,OUTPUT);
  pinMode(GREEN_PIN,OUTPUT);
  pinMode(BLUE_PIN,OUTPUT);
}

// Code Loop
void loop() {
  setBlue();
  DigiUSB.refresh();
  setBlue();
  if (DigiUSB.available() > 0) {
    in = 0;

    // Read the param from the serial-USB communication
    in = DigiUSB.read();
    if (next == 0){
      if(in == 115){
        next = 1;
        DigiUSB.println("Start");
      }
    }
    else if (next == 1){
      // If you're using a common-cathode LED, just use "constrain(color, 0, 255);"
      // If you're using a common-anode LED, just use "255 - constrain(in, 0, 255);" for the voltage sink effect
      Red = 255 - constrain(in, 0, 255);
      DigiUSB.print("Red ");
      DigiUSB.println(in,DEC);
      next = 2;
    }
    else if (next == 2){
      // If you're using a common-cathode LED, just use "constrain(color, 0, 255);"
      // If you're using a common-anode LED, just use "255 - constrain(in, 0, 255);" for the voltage sink effect
      Green = 255 - constrain(in, 0, 255);
      DigiUSB.print("Green ");
      DigiUSB.println(in,DEC);
      next = 3;
    }
    else if (next == 3){
      // If you're using a common-cathode LED, just use "constrain(color, 0, 255);"
      // If you're using a common-anode LED, just use "255 - constrain(in, 0, 255);" for the voltage sink effect
      Blue = 255 - constrain(in, 0, 255);
      DigiUSB.print("Blue ");
      DigiUSB.println(in,DEC);
      next = 0;
    }
 }
 // Write the RGB values to the outputs
 analogWrite(RED_PIN,Red);
 analogWrite(GREEN_PIN,Green);
   setBlue();
}

// PIN 2 does not support PWM, so we make a software simulated PWM
void setBlue(){
    if(Blue == 0){
      digitalWrite(BLUE_PIN,LOW);
      return;
    }
    else if(Blue == 255){
      digitalWrite(BLUE_PIN,HIGH);
      return;
    }
    
    // On period  
    for (int x=0;x<Blue;x++){
      digitalWrite(BLUE_PIN,HIGH);
    } 
    
    // Off period
    for(int x=0;x<(255-Blue);x++){
      digitalWrite(BLUE_PIN,LOW);
    }
  
} 
