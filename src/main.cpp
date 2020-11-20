#include <Arduino.h>
#include "util.h"

//#define DEBUG_I2C
#include "ssd1306_64x32.h"

enum {
 SSD1306_SCL = PORTD5,
 SSD1306_SDA = PORTD4,
 SSD1306_ADDRESS = 0x78,
 USEC_DELAY_DATA = 25,
 USEC_DELAY_CLK = 10
};

static const sw_i2c i2c{
    SSD1306_SCL,
    SSD1306_SDA,
    USEC_DELAY_DATA,
    USEC_DELAY_CLK,
    [](uint8_t port){ PORTD |= (1 << port);}, //set port hight
    [](uint8_t port){ PORTD &= ~(1 << port);}, //set port low
    [](uint8_t port){ DDRD |= (1 << port);}   //set port as output
};

static const ssd1306_64x32 display{
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
void setup() {
  Serial.begin(115200);

  ssd1306_64x32_init(display);
  Serial.println("Display Initialized.");

  ssd1306_64x32_fillscreen(display, 0x01);

  ssd1306_64x32_print(display, 0, 0,"This");
  ssd1306_64x32_print(display, 0, 1,"is a");
  ssd1306_64x32_print(display, 0, 2,"test");
  ssd1306_64x32_print(display, 0, 3,"KEY [3]");

  ssd1306_64x32_send_stream_to_page(display, 50,2, heartImage, sizeof(heartImage));   

  Serial.println("Done.");
 
  
}

void loop() {
delay(1000);
}