#include <Arduino.h>
#include <SPI.h>
#include "util.h"

//#define DEBUG_I2C
#include "ssd1306_128x64.h"

#ifdef ARDUINO_ARCH_ESP32
enum {
 SSD1306_SCL = 25,
 SSD1306_SCL_PORT = 0, //unused
 SPI_SS = 26,
 SPI_SS_PORT = 0, //unused
 SSD1306_ADDRESS = 0x78,
 USEC_DELAY_DATA = 25,
 USEC_DELAY_CLK = 10,
 USEC_DELAY_SPI = 5
};
#define ENABLE_OUTPUT_PIN(UNUSED, PIN) pinMode(PIN, OUTPUT);
#define SET_PIN(UNUSED, PIN) GPIO.out_w1ts = ((uint32_t)1 << PIN); //for pins<32
#define CLEAR_PIN(UNUSED, PIN) GPIO.out_w1tc = ((uint32_t)1 << PIN); //for pins<32

//auto vspi = new SPIClass(VSPI);
#endif

#ifdef ARDUINO_ARCH_AVR
enum {
 SSD1306_SCL = PORTD2,
 SSD1306_SCL_PORT = DDRD,
 SPI_SS = PORTB2,
 SPI_SS_PORT = DDRB,
 SSD1306_ADDRESS = 0x78,
 USEC_DELAY_DATA = 25,
 USEC_DELAY_CLK = 10,
 USEC_DELAY_SPI = 5
};
#define ENABLE_OUTPUT_PIN(PORT, PIN) PORT |= (1 << PIN);
#define SET_PIN(PORT, PIN) PORT |= (1 << PIN);
#define CLEAR_PIN(PORT, PIN) PORT &= ~(1 << PIN);
#endif

static uint16_t display_index = 0xffff;

static const sw_i2c i2c{
    USEC_DELAY_DATA,
    USEC_DELAY_CLK,

    [](){ SET_PIN(SSD1306_SCL_PORT, SSD1306_SCL); },
    [](){ CLEAR_PIN(SSD1306_SCL_PORT, SSD1306_SCL); },
    [](){ 
      /*CLEAR_PIN(SPI_SS_PORT, SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);
      SPI.transfer(display_index>>8);
      delayMicroseconds(USEC_DELAY_SPI);
      SET_PIN(SPI_SS_PORT, SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);*/
      CLEAR_PIN(SPI_SS_PORT, SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);
      SPI.transfer(display_index);
      delayMicroseconds(USEC_DELAY_SPI);
      SET_PIN(SPI_SS_PORT, SPI_SS);
    },
    [](){ 
      /*CLEAR_PIN(SPI_SS_PORT, SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);
      SPI.transfer(0x00);
      delayMicroseconds(USEC_DELAY_SPI);
      SET_PIN(SPI_SS_PORT, SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);*/
      CLEAR_PIN(SPI_SS_PORT, SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);
      SPI.transfer(0x00);
      delayMicroseconds(USEC_DELAY_SPI);
      SET_PIN(SPI_SS_PORT, SPI_SS);
    },
    [](){ ENABLE_OUTPUT_PIN(SSD1306_SCL_PORT, SSD1306_SCL);},
    [](){
      ENABLE_OUTPUT_PIN(SPI_SS_PORT, SPI_SS);
      SPI.begin(SCK,MISO,MOSI, -1);
    }
};

static const ssd1306_128x64 display{
  &i2c,
  SSD1306_ADDRESS
};

const uint8_t heartImage[8] =
{
  TRANSPOSE8(
    0B01100110,
    0B11111001,
    0B11111101,
    0B11111111,
    0B01111110,
    0B00111100,
    0B00011000,
    0B00000000
  )
};
/*
const uint8_t heartImage[8] =
{
  TRANSPOSE8_REVERSE(
    0B00001110,
    0B00011111,
    0B00111111,
    0B01111110,
    0B01111110,
    0B00111101,
    0B00011001,
    0B00001110
  )
};
*/

#include <SPI.h>

// set pin 10 as the slave select for the digital pot:

void setup() {
  Serial.begin(115200);


  Serial.println("SPI Initialized.");

  ssd1306_128x64_init(display);
  Serial.println("Displays Initialized!");

  ssd1306_128x64_fillscreen(display, 0x80);

  for(uint8_t idx=0;idx<6;idx++) {
    display_index = 1 << idx;
    //ssd1306_64x32_print(display, 0, 0, "This");
    //ssd1306_64x32_print(display, 0, 1, "is a");
    //ssd1306_64x32_print(display, 0, 2, "test");
    //char buffer[] = {'K','E','Y', ' ', '[', '0', ']', 0};
    //buffer[5] += idx;
    char buffer[] = {static_cast<char>('0' + idx), 0};
    ssd1306_128x64_print(display, 0, 0, buffer);
    ssd1306_128x64_print(display, 10, 1, buffer);
    ssd1306_128x64_print(display, 20, 2, buffer);
    ssd1306_128x64_print(display, 30, 3, buffer);
    ssd1306_128x64_print(display, 40, 4, buffer);
    ssd1306_128x64_print(display, 50, 5, buffer);
    ssd1306_128x64_print(display, 60, 6, buffer);
    ssd1306_128x64_print(display, 70, 7, "Test :)");
    //ssd1306_64x32_send_stream_to_page(display, 50,2, heartImage, sizeof(heartImage));   
  }

  

  Serial.println("Done.");
 
  
}

void loop() {
delay(1000);
}