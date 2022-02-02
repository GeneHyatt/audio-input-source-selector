#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>
#include <DHT.h>
#include <SPI.h>
#include <AudioDevice.h>;

// Establish pins for read and write.
#define TFT_CS 10
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 8
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DEVICEPIN_A 3     // Digital pin connected to turntable
#define DEVICEPIN_B 4     // Digital pin connected to tape player
#define DEVICEPIN_C 5     // Digital pin connected to bluetooth
#define DEVICEPIN_D 6     // Digital pin connected to aux

#define ENC_BUTT_PIN 24
#define NEO_PIN 6

#define DHTTYPE DHT11
#define SS_ADDR 0x36

const char dev_0[] PROGMEM = "Turntable";
const char dev_1[] PROGMEM = "Cassette";
const char dev_2[] PROGMEM = "Bluetooth player";
const char dev_3[] PROGMEM = "Aux";

const char *const device_name_table[] PROGMEM = {dev_0, dev_1, dev_2, dev_3};

char buffer[30];  // make sure this is large enough for the largest string it must hold

AudioDevice deviceA(DEVICEPIN_A, getDeviceName(0));
AudioDevice deviceB(DEVICEPIN_B, getDeviceName(1));
AudioDevice deviceC(DEVICEPIN_C, getDeviceName(2));
AudioDevice deviceD(DEVICEPIN_D, getDeviceName(3));

Adafruit_seesaw ss;
seesaw_NeoPixel pixel = seesaw_NeoPixel(1, NEO_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// color definitions
const uint16_t  Display_Color_Black        = 0x0000;
const uint16_t  Display_Color_Blue         = 0x001F;
const uint16_t  Display_Color_Red          = 0xF800;
const uint16_t  Display_Color_Green        = 0x07E0;
const uint16_t  Display_Color_Cyan         = 0x07FF;
const uint16_t  Display_Color_Magenta      = 0xF81F;
const uint16_t  Display_Color_Yellow       = 0xFFE0;
const uint16_t  Display_Color_White        = 0xFFFF;

const int stepCount = 5;     // number of detents to count before next switching to next device.
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button



// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

int encPos;
uint16_t Display_Text_Color          = Display_Color_Green;
uint16_t Display_Background_Color    = Display_Color_Black;
volatile bool dhtEnabled = false;
volatile bool screenState    = true;
volatile bool isDisplayVisible        = false;


void setup(void) {
  Serial.begin(115200);
  Serial.println(F("Hello! Starting audio selector unit."));
  Serial.println("Looking for seesaw!");
  if (! ss.begin(SS_ADDR) || ! pixel.begin(SS_ADDR)) {
    Serial.println("Couldn't find seesaw on default address");
    while (1) delay(10);
  }

  pinMode(DEVICEPIN_A, OUTPUT);
  pinMode(DEVICEPIN_B, OUTPUT);
  pinMode(DEVICEPIN_C, OUTPUT);
  pinMode(DEVICEPIN_D, OUTPUT);

  screenState = false;
  Serial.println("seesaw started");

  Serial.println("Setup Pixel");
  pixel.setBrightness(30);
  pixel.show();

  // use a pin for the built in encoder switch
  ss.pinMode(ENC_BUTT_PIN, INPUT_PULLUP);

  // get starting position of encoder
  encPos = ss.getEncoderPosition();

  Serial.println("Turning on interrupts");
  delay(10);
  ss.setGPIOInterrupts((uint32_t)1 << ENC_BUTT_PIN, 1);
  ss.enableEncoderInterrupt();

  Serial.println("Turning on DHT sensor");
  dht.begin();
  delay(10);
  if (!dht.readTemperature()) {
    Serial.println("DHT sensor not found.");
  } else {
    dhtEnabled = true;
  }

  Serial.println("Turning on Display");
  tft.initR(INITR_BLACKTAB);
  tft.setFont();
  tft.fillScreen(Display_Background_Color);
  tft.setTextColor(Display_Text_Color, Display_Background_Color);
  tft.setTextSize(1);
  tft.setRotation(1);
  tft.setTextWrap(true);
  // the display is now on
  isDisplayVisible = true;
  Serial.println("Startup complete.");
  Serial.println("Enjoy!!");
}

void loop() {

  tft.enableDisplay(isDisplayVisible);

  if (! ss.digitalRead(ENC_BUTT_PIN)) {
    buttonPress();
  }



  int32_t newPos = ss.getEncoderPosition();
  // did we move around?
  if (encPos != newPos) {
    Serial.println(newPos);         // display new position
    tft.setCursor(0, 100);
    tft.print(F("Input select = "));
    tft.print(newPos);

    if (newPos >= 1 && newPos <= 10) {
      digitalWrite(DEVICEPIN_A, HIGH);
      digitalWrite(DEVICEPIN_B, LOW);
      Serial.println(digitalRead(DEVICEPIN_A));
      Serial.println(digitalRead(DEVICEPIN_B));
      Serial.println(getDeviceName(0));
    } else if (newPos >= 11 && newPos < 21) {
      digitalWrite(DEVICEPIN_A, LOW);
      digitalWrite(DEVICEPIN_B, HIGH);
      Serial.println(digitalRead(DEVICEPIN_A));
      Serial.println(digitalRead(DEVICEPIN_B));
      Serial.println(getDeviceName(1));

    } else {
      digitalWrite(DEVICEPIN_A, LOW);
      digitalWrite(DEVICEPIN_B, LOW);
      Serial.println(digitalRead(DEVICEPIN_A));
      Serial.println(digitalRead(DEVICEPIN_B));
      Serial.println("No device selected.");
    }

    // change the neopixel color
    pixel.setPixelColor(0, Wheel(newPos & 0xFF));
    pixel.show();
    encPos = newPos;      // and save for next round
  }

  // don't overwhelm serial port
  // delay(10);


}


String getDeviceName(byte deviceIndex) {
  strcpy_P(buffer, (char *)pgm_read_word(&(device_name_table[deviceIndex])));
  return buffer;
}

uint32_t Wheel(byte wPOS) {
  wPOS = 255 - wPOS;
  if (wPOS < 85) {
    return pixel.Color(255 - wPOS * 3, 0, wPOS * 3);
  }
  if (wPOS < 170) {
    wPOS -= 85;
    return pixel.Color(0, wPOS * 3, 255 - wPOS * 3);
  }
  wPOS -= 170;
  return pixel.Color(wPOS * 3, 255 - wPOS * 3, 0);
}


void buttonPress() {
  screenState = !screenState;
  Serial.print("Button pressed with interupt ");
  Serial.println(screenState);
  if (screenState) {
    drawCurrentDevice();
  } else {
    tftPrintRunTime();
    if (dhtEnabled) {
      drawAmbientData();
    }
  }
}

void drawCurrentDevice() {
  tft.setTextColor(Display_Text_Color, Display_Background_Color);
  tft.setCursor(0, 0);
  tft.println("Turntable");
}

void drawAmbientData() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Compute heat index in Fahrenheit
  float hif = dht.computeHeatIndex(f, h);

  tft.setTextColor(Display_Text_Color, Display_Background_Color);
  tft.setCursor(0, 50);;
  tft.print(F("Temp: "));
  tft.print(f);
  tft.print((char)247);
  tft.println(F("F"));

  tft.print(F("Humidity: "));
  tft.print(h);
  tft.println(F("%"));

  tft.print(F("Heat index: "));
  tft.print(hif);
  tft.print((char)247);
  tft.println(F("F"));
}

void tftPrintRunTime() {
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}
