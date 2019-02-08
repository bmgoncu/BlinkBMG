#define USB_CFG_DEVICE_NAME     'D','i','g','i','B','l','i','n','k'
#define USB_CFG_DEVICE_NAME_LEN 9
#include <DigiUSB.h>

typedef struct RgbColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RgbColor;
typedef struct HsvColor
{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;

const int SET_COLOR_MODE = 120;
const int SET_COLOR_WHEEL_MODE = 125;
const int SET_COLOR_BREATHE_MODE = 115;

const int RED_PIN = 0;
const int GREEN_PIN = 1;
const int BLUE_PIN = 2;
byte in = 0;

byte Mode = SET_COLOR_MODE;

int Blue = 0;
int Red = 0;
int Green = 0;

HsvColor HSV_Data;

int TweenDuration = 100; // In milliseconds
int TweenClock = 0;
int TweenIncrementer = 1;
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
  DigiUSB.refresh();
  if (DigiUSB.available() > 0) {
    in = 0;
    // Read the param from the serial-USB communication
    in = DigiUSB.read();
    if (next == 0){
      if(in == SET_COLOR_MODE){
        next = 1;
        Mode = SET_COLOR_MODE;
        DigiUSB.println("Start SET_COLOR_MODE");
      }else if(in == SET_COLOR_WHEEL_MODE){
        next = 1;
        Mode = SET_COLOR_WHEEL_MODE;
        DigiUSB.println("Start SET_COLOR_WHEEL_MODE");
      }else if(in == SET_COLOR_BREATHE_MODE){
        next = 1;
        Mode = SET_COLOR_BREATHE_MODE;
        DigiUSB.println("Start SET_COLOR_BREATHE_MODE");
      }
    }else{
      if(Mode == SET_COLOR_MODE){
        if (next == 1){
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
      }else if(Mode == SET_COLOR_WHEEL_MODE){
        TweenIncrementer = 1;
        HSV_Data.h = 0;
        HSV_Data.s = 255;
        HSV_Data.v = 255;
        if (next == 1){
          // Tween Duration 
          TweenDuration = in;
          next = 0;
        }
      }else if(Mode == SET_COLOR_BREATHE_MODE){
        TweenIncrementer = 1;
        HSV_Data.s = 0;
        if (next == 1){
          // Tween Duration 
          TweenDuration = in;
          next = 2;
        }else if (next == 2){
          // HSV Hue 
          HSV_Data.h = constrain(in, 0, 255);
          next = 3;
        }else if (next == 3){
          // HSV Value 
          HSV_Data.v = constrain(in, 0, 255);
          next = 0;
        }
      }
    }
 }
 
 if(Mode == SET_COLOR_WHEEL_MODE){
    SetColorWheelMode();
 }else if(Mode == SET_COLOR_BREATHE_MODE){
    SetColorBreatheMode();
 }
 
 // Write the RGB values to the outputs
 SetColors();
}

// We make a software simulated PWM
void SetColors(){
    for (int x=0;x<=255;x++){
      digitalWrite(RED_PIN,(Red > x)? HIGH : LOW);
      digitalWrite(GREEN_PIN,(Green > x)? HIGH : LOW);
      digitalWrite(BLUE_PIN,(Blue > x)? HIGH : LOW);
    } 
}
 
void SetColorWheelMode(){
  TweenClock += 1;
  if(TweenClock >= (TweenDuration * 6)){
    TweenClock = 0;
    //delay(50);
    
    RgbColor rgb;
    rgb = HsvToRgb(HSV_Data);
    Red = rgb.r;
    Green = rgb.g;
    Blue = rgb.b;
    
    HSV_Data.h += TweenIncrementer;
    if (HSV_Data.h >= 255)
      HSV_Data.h = 0;
  }
}

void SetColorBreatheMode(){
  TweenClock += 1;
  if(TweenClock >= (TweenDuration * 6)){
    TweenClock = 0;
    //delay(50);
    
    RgbColor rgb;
    rgb = HsvToRgb(HSV_Data);
    Red = rgb.r;
    Green = rgb.g;
    Blue = rgb.b;
    
    HSV_Data.s += TweenIncrementer;
    if (HSV_Data.s <= 0 || HSV_Data.s >= 255)
      TweenIncrementer *= -1;
  }
}

// HSV->RGB conversion based on GLSL version
RgbColor HsvToRgb(HsvColor hsv)
{
    RgbColor rgb;
    unsigned char region, p, q, t;
    unsigned int h, s, v, remainder;
    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }
    h = hsv.h;
    s = hsv.s;
    v = hsv.v;
    
    region = h / 43;
    remainder = (h - (region * 43)) * 6; 
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    switch (region)
    {
        case 0:
            rgb.r = v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = v;
            break;
        default:
            rgb.r = v; rgb.g = p; rgb.b = q;
            break;
    }
    return rgb;
}
