// MPU6050 from: MPU-6050 Short Example Sketch for Arduino; By Arduino User JohnChi; August 17, 2014

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On my frisbee this is D3
#define LED_PIN    D3

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 49

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

int red = 0;
int green = LED_COUNT / 3;
int blue = LED_COUNT * 2 / 3;
#define delta LED_COUNT - 1

#define INIT_TIME 9
#define RGB_BRIGHTNESS 127

//int pos1000 = 0; // 1000°
long mLEDshift = 0; // LED/1024
long lastTime = micros(); // µs
int ledOffs; // LED
int lastOffs; // LED

enum Animation {
  ThreeDotsRGB,
  Glow,
  RedToGreen,
  HueFuu,
  TwoWayRotation,
  SineRotation,
  Radioactive,
  Randots,
  FadingDot,
  Sparkling,
  ColorByDirection,
  ReversedDots,

  NumOfAnimations
};
enum Animation currentAnim = ReversedDots;
long changeAfterXms = 45000;
long lastChangeTime = 0;
long seed = 42;
long tmp1 = 0;

boolean showCollision = true;
boolean switchOnCollision = true;
#define COLLISION_THRESH 30000l

#define GLOW_BRIGHTNESS 17
#define ANIM_BRIGHTNESS 64
#define LIGHTNING_BRIGHTNESS 255


#include<Wire.h>
const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

void setup() {
  WiFi.mode(WIFI_OFF);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  for (int i = 0; i < INIT_TIME; i++) {// wait for INIT_TIME sec to increase chances of successful upload
    strip.setPixelColor(0, 32, 0, 0);
    strip.show();
    delay(500);
    strip.setPixelColor(0, 0, 0, 0);
    strip.show();
    delay(500);
  }
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  // set precision to lowest for larger gyro range
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1B);       // GYRO_CONFIG register
  Wire.write(3 << 3);     // set to 0b00011000 (2000°/s)
  Wire.endTransmission(true);

  // set precision to lowest for larger acc range
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1C);       // ACCEL_CONFIG register
  Wire.write(3 << 3);     // set to 0b00011000 (2000°/s)
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1B);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t) MPU_addr,(size_t) 1,true);  // request a total of 14 registers
  int GYRO_CONFIG = Wire.read();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1C);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t) MPU_addr,(size_t) 1,true);  // request a total of 14 registers
  int ACCEL_CONFIG = Wire.read();
  Serial.begin(9600);
  Serial.print("GYRO_CONFIG: ");
  Serial.println(GYRO_CONFIG);
  Serial.print("ACCEL_CONFIG: ");
  Serial.println(ACCEL_CONFIG);

  lastTime = micros(); // µs
  lastChangeTime = lastTime;

  /*Serial.println("=== RANDOM: ===");
  randomSeed(42);
  Serial.println(random(-2147483648, 2147483647));
  Serial.println(random(-2147483648, 2147483647));
  Serial.println(random(-2147483648, 2147483647));
  randomSeed(42);
  Serial.println(random(256));
  Serial.println(random(256));
  randomSeed(42);
  Serial.println(random(256));
  Serial.println(random(256));
  Serial.println("===   END   ===");*/
}


int sin1024(long x) {
  x %= 1024 * 49;
  x += 1073766400;
  x %= 1024 * 49;
  int sign = (x > 512 * 49) ? -1 : 1;
  if (x > 256 * 49) x = 512*49 - x;
  long a = x*132/1031;
  long b = 121*x/64*x/117649;
  return (int) (a +(- a*b/1537 + (a*b/120 - a*b/5040*b/1024)*b/262144) / 4);
}

void setMPixelColor(long mPos, int r, int g, int b) {
  mPos += 21913600*LED_COUNT;
  mPos %= 1024 * LED_COUNT;
  strip.setPixelColor(mPos / 1024, r - r * (mPos % 1024) / 1024, g - g * (mPos % 1024) / 1024, b - b * (mPos % 1024) / 1024);
  strip.setPixelColor((mPos / 1024 + 1) % LED_COUNT, r * (mPos % 1024) / 1024, g * (mPos % 1024) / 1024, b * (mPos % 1024) / 1024);
}

void fade(int frac = 250) {
  if (frac > 0) {
    for (int i = 0; i < LED_COUNT; i++) {
      uint32_t color = strip.getPixelColor(i);
      int red = (color >> 16) & 0xFF;
      int green = (color >> 8) & 0xFF;
      int blue = color & 0xFF;
      strip.setPixelColor(i, red*frac/255, green*frac/255, blue*frac/255);
    }
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 0, 0, 0);
    }
  }
}

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t) MPU_addr,(size_t) 14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)


  // GyZ in °/s ??
  long degPerSec = GyZ * 2000 / 32768; // °/s
  long curTime = micros(); // µs = s/1000000
  int deltaT = curTime - lastTime; // µs = s/1000000
  lastTime = curTime; // ms = s/1000000
  //pos1000 -= degPerSec * deltaT / 1000; // °/1000 != (°/s * s/1000000) / 1000
  //pos1000 += 360000;
  //pos1000 %= 360000;
  mLEDshift -= degPerSec * deltaT * 98/703125; //1024 * 49 / 1000000 / 360;
  lastOffs = ledOffs; // LED
  //ledOffs = pos1000 * LED_COUNT / 360000; // LED
  ledOffs = ((mLEDshift + 1073766400) % (1024 * 49)) / 1024;
  int change = (ledOffs - lastOffs + LED_COUNT)%LED_COUNT;
  long AcXY = (((long) AcX)*100l/COLLISION_THRESH) * (((long) AcX)*100l/COLLISION_THRESH) + (((long) AcY)*100l/COLLISION_THRESH) * (((long) AcY)*100l/COLLISION_THRESH);


  if (curTime - lastChangeTime > changeAfterXms * 1000) {
    lastChangeTime = curTime;
    switch (currentAnim) {
      case ThreeDotsRGB:
        currentAnim = Glow;
        Serial.println("Glow");
        break;
      case Glow:
        currentAnim = RedToGreen;
        Serial.println("RedToGreen");
        break;
      case RedToGreen:
        currentAnim = HueFuu;
        Serial.println("HueFuu");
        break;
      case HueFuu:
        currentAnim = TwoWayRotation;
        Serial.println("TwoWayRotation");
        break;
      case TwoWayRotation:
        currentAnim = SineRotation;
        Serial.println("SineRotation");
        break;
      case SineRotation:
        currentAnim = Radioactive;
        Serial.println("Radioactive");
        break;
      case Radioactive:
        currentAnim = Randots;
        Serial.println("Randots");
        break;
      case Randots:
        currentAnim = FadingDot;
        Serial.println("FadingDot");
        break;
      case FadingDot:
        currentAnim = Sparkling;
        Serial.println("Sparkling");
        break;
      case Sparkling:
        currentAnim = ColorByDirection;
        Serial.println("ColorByDirection");
        break;
      case ColorByDirection:
        currentAnim = ReversedDots;
        Serial.println("ReversedDots");
        break;

      default:
        currentAnim = ThreeDotsRGB;
        Serial.println("ThreeDotsRGB");
        break;
    }
  }


  switch (currentAnim) {
    case ThreeDotsRGB:
      strip.setPixelColor(red, 0, 0, 0);
      strip.setPixelColor(green, 0, 0, 0);
      strip.setPixelColor(blue, 0, 0, 0);
      strip.setPixelColor((red + LED_COUNT / 2) % LED_COUNT, 0, 0, 0);
      strip.setPixelColor((green + LED_COUNT / 2) % LED_COUNT, 0, 0, 0);
      strip.setPixelColor((blue + LED_COUNT / 2) % LED_COUNT, 0, 0, 0);
      red = ledOffs; green = ledOffs + LED_COUNT / 3; blue = ledOffs + LED_COUNT * 2 / 3;
      red %= LED_COUNT; green %= LED_COUNT; blue %= LED_COUNT;
      strip.setPixelColor(red, RGB_BRIGHTNESS, 0, 0);
      strip.setPixelColor(green, 0, RGB_BRIGHTNESS, 0);
      strip.setPixelColor(blue, 0, 0, RGB_BRIGHTNESS);
      strip.setPixelColor((red + LED_COUNT / 2) % LED_COUNT, RGB_BRIGHTNESS, 0, 0);
      strip.setPixelColor((green + LED_COUNT / 2) % LED_COUNT, 0, RGB_BRIGHTNESS, 0);
      strip.setPixelColor((blue + LED_COUNT / 2) % LED_COUNT, 0, 0, RGB_BRIGHTNESS);
      break;
    case Glow:
      {
        int bright = 127;
        int x = bright * abs(degPerSec) / 10000;// * 10 / 360;
        if (x > bright) x = bright;
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, GLOW_BRIGHTNESS - x * GLOW_BRIGHTNESS / bright, 0, x);
        }
      }
      break;
    case RedToGreen:
      for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor((i+ledOffs)%LED_COUNT, i, 0,  LED_COUNT-i);
      }
      break;
    case HueFuu:
      for (int i = 0; i < LED_COUNT; i++) {
        // 16725 ^= 120°
        {
          int loops = 6;
          int x = (mLEDshift - i*1024)/loops;
          int base = GLOW_BRIGHTNESS;
          int scale = 30;
          int r = max(0, sin1024(x)/scale+base);
          int g = max(0, sin1024(x+16725)/scale+base);
          int b = max(0, sin1024(x+16725*2)/scale+base);
          strip.setPixelColor(i, r, g, b);
        }
      }
      break;
    case TwoWayRotation:
      fade(0);
      {
        int mPos = mLEDshift;
        setMPixelColor(mPos + 1024*LED_COUNT*0/6 + curTime/1000*LED_COUNT*1024/1000, RGB_BRIGHTNESS, 0, 0);
        setMPixelColor(mPos + 1024*LED_COUNT*0/6 - curTime/1000*LED_COUNT*1024/1000, 0, RGB_BRIGHTNESS, 0);
        setMPixelColor(mPos + 1024*LED_COUNT*0/6, 0, 0, RGB_BRIGHTNESS);
        setMPixelColor(mPos + 1024*LED_COUNT*2/6 + curTime/1000*LED_COUNT*1024/1000, RGB_BRIGHTNESS, 0, 0);
        setMPixelColor(mPos + 1024*LED_COUNT*2/6 - curTime/1000*LED_COUNT*1024/1000, 0, RGB_BRIGHTNESS, 0);
        setMPixelColor(mPos + 1024*LED_COUNT*2/6, 0, 0, RGB_BRIGHTNESS);
        setMPixelColor(mPos + 1024*LED_COUNT*4/6 + curTime/1000*LED_COUNT*1024/1000, RGB_BRIGHTNESS, 0, 0);
        setMPixelColor(mPos + 1024*LED_COUNT*4/6 - curTime/1000*LED_COUNT*1024/1000, 0, RGB_BRIGHTNESS, 0);
        setMPixelColor(mPos + 1024*LED_COUNT*4/6, 0, 0, RGB_BRIGHTNESS);
      }
      break;
    case SineRotation:
      fade(0);
      {
        int mPos = mLEDshift;
        int t = curTime/100;
        setMPixelColor(mPos + LED_COUNT*sin1024(t), RGB_BRIGHTNESS, RGB_BRIGHTNESS, 0);
        setMPixelColor(mPos + LED_COUNT*sin1024(t) + 1024*LED_COUNT*3/6, RGB_BRIGHTNESS, 0, 0);
        setMPixelColor(mPos + LED_COUNT*sin1024(t + 1024*LED_COUNT*1/3), 0, RGB_BRIGHTNESS, RGB_BRIGHTNESS);
        setMPixelColor(mPos + LED_COUNT*sin1024(t + 1024*LED_COUNT*1/3) + 1024*LED_COUNT*3/6, 0, RGB_BRIGHTNESS, 0);
        setMPixelColor(mPos + LED_COUNT*sin1024(t + 1024*LED_COUNT*2/3), RGB_BRIGHTNESS, 0, RGB_BRIGHTNESS);
        setMPixelColor(mPos + LED_COUNT*sin1024(t + 1024*LED_COUNT*2/3) + 1024*LED_COUNT*3/6, 0, 0, RGB_BRIGHTNESS);
      }
      break;
    case Radioactive:
      for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, ((i - mLEDshift/1024 + 16384) * 6 / LED_COUNT % 2) * ANIM_BRIGHTNESS,
                               ((i - mLEDshift/1024 + 16384) * 6 / LED_COUNT % 2) * ANIM_BRIGHTNESS, 0);
      }
      break;
    case Randots:
      fade(0);
      randomSeed(seed);
      if (curTime - tmp1 > 3*1000000) {
        tmp1 = curTime;
        random(1);random(1);random(1);
        seed = random(-2147483648, 2147483647);
      }
      for(int i = 1; i <= 10; i++) {
        setMPixelColor(random(1024*LED_COUNT) + mLEDshift, random(RGB_BRIGHTNESS*i/10), random(RGB_BRIGHTNESS*i/10), random(RGB_BRIGHTNESS*i/10));
      }
      break;
    case FadingDot:
      fade(250);
      strip.setPixelColor(ledOffs, 50, 100, 150);
      break;
    case Sparkling:
      fade(254);
      randomSeed(seed);
      if (curTime - tmp1 > 100000) {
        tmp1 = curTime;
        random(1);random(1);random(1);
        seed = random(-2147483648, 2147483647);
      }
      for(int i = 1; i <= 10; i++) {
        setMPixelColor(random(1024*LED_COUNT) + mLEDshift, random(RGB_BRIGHTNESS*i/10), random(RGB_BRIGHTNESS*i/10), random(RGB_BRIGHTNESS*i/10));
      }
      break;
    case ColorByDirection:
      fade(245);
      {
        int rotation = degPerSec * deltaT / 10000;
        int red = min(max(rotation, 0), 255);
        int blue = min(max(-rotation, 0), 255);
        int greed = min(255 - red, 255 - blue);
        strip.setPixelColor(ledOffs, red, green, blue);
        strip.setPixelColor((ledOffs+LED_COUNT/3)%LED_COUNT, red, green, blue);
        strip.setPixelColor((ledOffs+LED_COUNT*2/3)%LED_COUNT, red, green, blue);
      }
      break;
    case ReversedDots:
      fade(0);
      strip.setPixelColor((mLEDshift*11/10/1024 + 00 + 16384) % LED_COUNT, RGB_BRIGHTNESS, RGB_BRIGHTNESS, 0);
      strip.setPixelColor((mLEDshift*11/10/1024 + 10 + 16384) % LED_COUNT, 0, RGB_BRIGHTNESS, RGB_BRIGHTNESS);
      strip.setPixelColor((mLEDshift*11/10/1024 + 20 + 16384) % LED_COUNT, RGB_BRIGHTNESS, RGB_BRIGHTNESS/3, 0);
      strip.setPixelColor((mLEDshift*11/10/1024 + 30 + 16384) % LED_COUNT, RGB_BRIGHTNESS, 0, RGB_BRIGHTNESS);
      strip.setPixelColor((mLEDshift*11/10/1024 + 40 + 16384) % LED_COUNT, 0, RGB_BRIGHTNESS, RGB_BRIGHTNESS/2);
      break;
  }
  if (AcXY >= 10000) {
    if (showCollision) {
      //for (int i = 0; i < sqrt(AcXY)/10; i++) {
      for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, LIGHTNING_BRIGHTNESS, LIGHTNING_BRIGHTNESS, LIGHTNING_BRIGHTNESS);
      }
    }
    if (switchOnCollision) {
      currentAnim = (Animation) random(NumOfAnimations);
    }
  }
  strip.show();
  delay(2);
}

