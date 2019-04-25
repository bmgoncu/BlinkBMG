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

const char RED_PIN = 0;
const char GREEN_PIN = 1;
const char BLUE_PIN = 2;

const char SET_COLOR_MODE = 1;
const char SET_COLOR_WHEEL_MODE = 2;
const char SET_COLOR_BREATHE_MODE = 3;

// current color
RgbColor cur_Rgb;
HsvColor cur_Hsv;

RgbColor target_Rgb;
RgbColor from_Rgb;

HsvColor tmp_Hsv;
unsigned char tmp_TweenSpeed;
unsigned char is_target_reached;

// 0 idle
// 1 selecting mode
// 2 getting parameters
unsigned char command_status = 0;

unsigned char Mode;

unsigned char input_mode = 0;
unsigned char input = -1;
unsigned char next = 0;

unsigned char TweenSpeed = 0;
unsigned int TweenClock = 0;
unsigned char TweenIncrementer = 1;


unsigned char current_time = 0;
short tween_speed = 2;

// Initialize
void setup() {
  DigiUSB.begin();
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

// Code Loop
void loop() {
  DigiUSB.refresh();
  if (DigiUSB.available() > 0) {

    // Read the param from the serial-USB communication
    input = DigiUSB.read();
    DigiUSB.write(input);

    if (command_status == 0) {
      if (input == SET_COLOR_MODE) {
        input_mode = SET_COLOR_MODE;
        command_status = 1;
        next = 0;
      } else if (input == SET_COLOR_WHEEL_MODE) {
        input_mode = SET_COLOR_WHEEL_MODE;
        command_status = 1;
        next = 0;
      } else if (input == SET_COLOR_BREATHE_MODE) {
        input_mode = SET_COLOR_BREATHE_MODE;
        command_status = 1;
        next = 0;
      }
    } else if (command_status == 1) {
      if (input_mode == SET_COLOR_MODE) {
        ColorModeInput();
      } else if (input_mode == SET_COLOR_WHEEL_MODE) {
        ColorWheelModeInput();
      }  else if (input_mode == SET_COLOR_BREATHE_MODE) {
        ColorBreatheModeInput();
      }
    } else if (command_status == 2) {
      command_status = 0;
    }
  }


  if (Mode == SET_COLOR_WHEEL_MODE) {
    SetColorWheelMode();
  } else if (Mode == SET_COLOR_BREATHE_MODE) {
    SetColorBreatheMode();
  }

  SetHsvColors();
}

void ColorModeInput() {
  if (next == 0) {
    tmp_Hsv.h = constrain(input, 0, 255);
    next = 1;
  } else if (next == 1) {
    tmp_Hsv.s = constrain(input, 0, 255);
    next = 2;
  } else if (next == 2) {
    tmp_Hsv.v = constrain(input, 0, 255);
    
    SetNewTarget(tmp_Hsv, 0);

    command_status = 0;
    Mode = input_mode;
  }
}

void ColorWheelModeInput() {
  // Tween Duration
  TweenSpeed = input;
  TweenIncrementer = 1;

  cur_Hsv.h = 0;
  cur_Hsv.s = 255;
  cur_Hsv.v = 255;

  command_status = 0;
  Mode = input_mode;
}

void ColorBreatheModeInput() {
  if (next == 0) {
    // Tween Duration
    tmp_TweenSpeed = input;
    next = 1;
  } else if (next == 1) {
    // Hue
    tmp_Hsv.h = constrain(input, 0, 255);
    next = 2;
  } else if (next == 2) {
    // Saturation
    tmp_Hsv.s = constrain(input, 0, 255);
    next = 3;
  } else if (next == 3) {
    // Value
    tmp_Hsv.v = constrain(input, 0, 255);
    
    tween_speed = tmp_TweenSpeed;
    SetNewTarget(tmp_Hsv, 0);

    command_status = 0;
    Mode = input_mode;
  }
}

void SetNewTarget(HsvColor target, unsigned char instant) {
  cur_Hsv = target;
  from_Rgb = cur_Rgb;
  target_Rgb = HsvToRgb(target);
  if (instant > 0) {
    current_time = 255;
    is_target_reached = 1;
  } else {
    current_time = 0;
    is_target_reached = 0;
  }
}

void SetHsvColors() {
  if (is_target_reached == 0) {
    if (current_time + tween_speed >= 255) {
      current_time = 255;
      is_target_reached = 1;
    } else {
      current_time += tween_speed;
    }
  }

  cur_Rgb.r = map(current_time, 0, 255, from_Rgb.r, target_Rgb.r);
  cur_Rgb.g = map(current_time, 0, 255, from_Rgb.g, target_Rgb.g);
  cur_Rgb.b = map(current_time, 0, 255, from_Rgb.b, target_Rgb.b);
  
  SetColors();
}

// set led color, software simulated PWM
void SetColors() {
  for (int x = 0; x <= 255; x++) {
    digitalWrite(RED_PIN, (cur_Rgb.r > x) ? LOW : HIGH);
    digitalWrite(GREEN_PIN, (cur_Rgb.g > x) ? LOW : HIGH);
    digitalWrite(BLUE_PIN, (cur_Rgb.b > x) ? LOW : HIGH);
  }
}

void SetColorWheelMode() {
  TweenClock += 1;
  if (TweenClock >= TweenSpeed) {
    TweenClock = 0;

    cur_Hsv.h += TweenIncrementer;
    if (cur_Hsv.h >= 255) {
      cur_Hsv.h = 0;
    }

    SetNewTarget(cur_Hsv, 1);
  }
}

void SetColorBreatheMode() {
  if (is_target_reached) {
    if (cur_Hsv.v == 255) {
      cur_Hsv.v = 0;
      SetNewTarget(cur_Hsv, 0);
    } else if (cur_Hsv.v == 0) {
      cur_Hsv.v = 255;
      SetNewTarget(cur_Hsv, 0);
    }
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

HsvColor RgbToHsv(RgbColor rgb)
{
    HsvColor hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * ((long)(rgbMax - rgbMin)) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}