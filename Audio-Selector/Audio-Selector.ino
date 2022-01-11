#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>
#include <DHT.h>
#include <SPI.h>


#define TFT_CS 10
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 8
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11
#define ENC_BUTT_PIN 24
#define NEO_PIN 6
#define SS_ADDR 0x36

Adafruit_seesaw ss;
seesaw_NeoPixel pixel = seesaw_NeoPixel(1, NEO_PIN, NEO_GRB + NEO_KHZ800);

// For 1.44" and 1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);
int32_t encPos;
bool dhtEnabled = false;

void setup(void) {
  Serial.begin(115200);
  Serial.println(F("Hello! Starting audio selector unit."));
  Serial.println("Looking for seesaw!");
  if (! ss.begin(SS_ADDR) || ! pixel.begin(SS_ADDR)) {
    Serial.println("Couldn't find seesaw on default address");
    while (1) delay(10);
  }
  Serial.println("seesaw started");

  Serial.println("Setup Pixel");
  pixel.setBrightness(50);
  pixel.show();

  // use a pin for the built in encoder switch
  ss.pinMode(ENC_BUTT_PIN, INPUT_PULLUP);

  // get starting position
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
  tft.setRotation(1);
  tft.setTextWrap(true);
  Serial.println("Startup complete.");
  Serial.println("Enjoy!!");
}

void loop() {

  if (! ss.digitalRead(ENC_BUTT_PIN)) {
    Serial.println("Button pressed!");
    tftPrintRunTime();
    if (dhtEnabled) {
      drawAmbientData();
    }
  }

  int32_t newPos = ss.getEncoderPosition();
  // did we move around?
  if (encPos != newPos) {
    Serial.println(newPos);         // display new position
    tft.setCursor(0, 100);
    tft.print(F("Input select = "));
    tft.print(newPos);

    // change the neopixel color
    pixel.setPixelColor(0, Wheel(newPos & 0xFF));
    pixel.show();
    encPos = newPos;      // and save for next round
  }

  // don't overwhelm serial port
  // delay(10);


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

void drawAmbientData() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Compute heat index in Fahrenheit
  float hif = dht.computeHeatIndex(f, h);
  delay(1500);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(0, 0);
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
