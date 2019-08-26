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
};
enum Animation currentAnim = HueFuu;
long changeAfterXms = 15000;
long lastChangeTime = 0;

boolean showCollision = true;
#define COLLISION_THRESH 30000l

#define GLOW_BRIGHTNESS 17
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
    switch (currentAnim) {
      case ThreeDotsRGB:
      case Glow:
      case RedToGreen:
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, 0, 0, 0);
        }
    }
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
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, 0, 0, 64);
        }
        break;
      //case HueFuu:
        currentAnim = TwoWayRotation;
        Serial.println("TwoWayRotation");
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
      for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, GLOW_BRIGHTNESS, 0, 0);
      }
      break;
    case RedToGreen:
      for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor((i+ledOffs)%LED_COUNT, i, i,  LED_COUNT-i);
      }
      break;
    case HueFuu:
      for (int i = 0; i < LED_COUNT; i++) {
        // 16725 ^= 120°
        {
          int loops = 3;
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
      for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor((i+ledOffs)%LED_COUNT, i, LED_COUNT-i, 0);
      }
      break;
  }
  if (showCollision) {
    if (AcXY >= 10000) {
      //for (int i = 0; i < sqrt(AcXY)/10; i++) {
      for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, LIGHTNING_BRIGHTNESS, LIGHTNING_BRIGHTNESS, LIGHTNING_BRIGHTNESS);
      }
    }
  }
  strip.show(); // Initialize all pixels to 'off'
  delay(2);
}

