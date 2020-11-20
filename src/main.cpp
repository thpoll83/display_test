#include <Arduino.h>
#include <SPI.h>
#include "util.h"

//#define DEBUG_I2C
#include "ssd1306_64x32.h"

enum {
 SSD1306_SCL = PORTD2,
 SPI_SS = PORTB2,
 SSD1306_ADDRESS = 0x78,
 USEC_DELAY_DATA = 25,
 USEC_DELAY_CLK = 10,
 USEC_DELAY_SPI = 5
};

#define SPI_SS PORTB2

static uint16_t display_index = 0xffff;

static const sw_i2c i2c{
    USEC_DELAY_DATA,
    USEC_DELAY_CLK,

    [](){ PORTD |= (1 << SSD1306_SCL);},
    [](){ PORTD &= ~(1 << SSD1306_SCL);},
    [](){ 
      PORTB &= ~(1 << SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);
      SPI.transfer(display_index>>8);
      delayMicroseconds(USEC_DELAY_SPI);
      PORTB |= (1 << SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);
      PORTB &= ~(1 << SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);
      SPI.transfer(display_index);
      delayMicroseconds(USEC_DELAY_SPI);
      PORTB |= (1 << SPI_SS);
    },
    [](){ 
      PORTB &= ~(1 << SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);
      SPI.transfer(0x00);
      delayMicroseconds(USEC_DELAY_SPI);
      PORTB |= (1 << SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);
      PORTB &= ~(1 << SPI_SS);
      delayMicroseconds(USEC_DELAY_SPI);
      SPI.transfer(0x00);
      delayMicroseconds(USEC_DELAY_SPI);
      PORTB |= (1 << SPI_SS);
    },
    [](){ DDRD |= (1 << SSD1306_SCL);},
    [](){
      DDRB |= (1 << SPI_SS);
      SPI.begin();
    }
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

#include <SPI.h>

// set pin 10 as the slave select for the digital pot:

const int slaveSelectPin = 10;

void setup() {
  Serial.begin(115200);


  Serial.println("SPI Initialized.");

  ssd1306_64x32_init(display);
  Serial.println("Displays Initialized.");

  ssd1306_64x32_fillscreen(display, 0x00);

  for(uint8_t idx=0;idx<10;idx++) {
    display_index = 1 << idx;
    //ssd1306_64x32_print(display, 0, 0, "This");
    //ssd1306_64x32_print(display, 0, 1, "is a");
    //ssd1306_64x32_print(display, 0, 2, "test");
    //char buffer[] = {'K','E','Y', ' ', '[', '0', ']', 0};
    //buffer[5] += idx;
    char buffer[] = {'0' + idx, 0};
    ssd1306_64x32_print(display, 30, 1, buffer);
    //ssd1306_64x32_send_stream_to_page(display, 50,2, heartImage, sizeof(heartImage));   
  }

  

  Serial.println("Done.");
 
  
}

void loop() {
delay(1000);
}