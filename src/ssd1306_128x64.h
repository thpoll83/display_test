#pragma once

#include "sw_i2c.h"
#include "font.h"

struct ssd1306_128x64 {
   const struct sw_i2c *i2c;
   const uint8_t address;
};

void ssd1306_128x64_send_cmd(const struct ssd1306_128x64 & disp, uint8_t cmd) {
    noInterrupts();  
    #ifdef DEBUG_I2C
        Serial.print("Cmd1 ");
    #endif
    i2c_start(disp.i2c);
    i2c_send(disp.i2c, disp.address);
    i2c_send(disp.i2c, 0x00);  // write command
    i2c_send(disp.i2c, cmd);
    i2c_stop(disp.i2c);
    #ifdef DEBUG_I2C
        Serial.print('\n');
    #endif
    interrupts();
}

void ssd1306_128x64_send_cmd(const struct ssd1306_128x64 & disp, int8_t cmd, uint8_t value1) {
    #ifdef DEBUG_I2C
        Serial.print("Cmd2 ");
    #endif
    i2c_start(disp.i2c);
    i2c_send(disp.i2c, disp.address);
    i2c_send(disp.i2c, 0x00);  // write command
    i2c_send(disp.i2c, cmd);
    i2c_send(disp.i2c, value1);
    i2c_stop(disp.i2c);
    #ifdef DEBUG_I2C
        Serial.print('\n');
    #endif
}

void ssd1306_128x64_send_cmd(const struct ssd1306_128x64 & disp, uint8_t cmd, uint8_t value1, uint8_t value2) {
    #ifdef DEBUG_I2C
        Serial.print("Cmd2 ");
    #endif
    i2c_start(disp.i2c);
    i2c_send(disp.i2c, disp.address);
    i2c_send(disp.i2c, 0x00);  // write command
    i2c_send(disp.i2c, cmd);
    i2c_send(disp.i2c, value1);
    i2c_send(disp.i2c, value2);
    i2c_stop(disp.i2c);
    #ifdef DEBUG_I2C
        Serial.print('\n');
    #endif
}

void ssd1306_128x64_send_cmd(const struct ssd1306_128x64 & disp, uint8_t cmd, uint8_t value1, uint8_t value2, uint8_t value3) {
    #ifdef DEBUG_I2C
        Serial.print("Cmd3 ");
    #endif
    i2c_start(disp.i2c);
    i2c_send(disp.i2c, disp.address);
    i2c_send(disp.i2c, 0x00);  // write command
    i2c_send(disp.i2c, cmd);
    i2c_send(disp.i2c, value1);
    i2c_send(disp.i2c, value2);
    i2c_send(disp.i2c, value3);
    i2c_stop(disp.i2c);
    #ifdef DEBUG_I2C
        Serial.print('\n');
    #endif
}


void ssd1306_128x64_init(const struct ssd1306_128x64 & disp) {
    #ifdef DEBUG_I2C
        Serial.println("*** Init SSD1206 128x64 Display ***");
        char buffer[32];
        sprintf (buffer, "Address: 0x%02x\n", disp.address);
        Serial.print(buffer);
        Serial.println("Sending Commands:");
    #endif

    i2c_init(disp.i2c);

    ssd1306_128x64_send_cmd(disp, 0xae);		      // display off
    ssd1306_128x64_send_cmd(disp, 0x00);  //set lower column address
    ssd1306_128x64_send_cmd(disp, 0x12);  //set higher column address
    ssd1306_128x64_send_cmd(disp, 0xb0);
    ssd1306_128x64_send_cmd(disp, 0xd5, 0x80);		// clock divide ratio (0x00=1) and oscillator frequency (0x8)
    ssd1306_128x64_send_cmd(disp, 0xa8, 0x3f);		// multiplex ratio: 0x1f = 32 pixel height, 0x3f = 63 pixel height
    ssd1306_128x64_send_cmd(disp, 0xd3, 0x00);		// vertical display offset
    ssd1306_128x64_send_cmd(disp, 0x40);	        // set display start line to 0
    ssd1306_128x64_send_cmd(disp, 0x8d, 0x14);		// [2] charge pump setting (p62): 0x014 enable, 0x010 disable
    ssd1306_128x64_send_cmd(disp, 0x20, 0x02);		// page addressing mode 2==page
    ssd1306_128x64_send_cmd(disp, 0xa1);		    	// segment remap a0/a1
    ssd1306_128x64_send_cmd(disp, 0xc8);			    // c0: scan dir normal, c8: reverse
    ssd1306_128x64_send_cmd(disp, 0xda, 0x12);		// com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5)
    ssd1306_128x64_send_cmd(disp, 0x81, 0xcf);		// [2] set contrast control datasheet: 0xcf
    ssd1306_128x64_send_cmd(disp, 0xd9, 0xf1);		// [2] pre-charge period 0x022/f1
    ssd1306_128x64_send_cmd(disp, 0xdb, 0x40);		// vcomh deselect level
    ssd1306_128x64_send_cmd(disp, 0x2e);			  	//- Deactivate scroll 
    ssd1306_128x64_send_cmd(disp, 0xa4);				  // output ram to display
    ssd1306_128x64_send_cmd(disp, 0xa6);			  	// none inverted normal display mode
    ssd1306_128x64_send_cmd(disp, 0xaf);         // --turn on oled panel

     #ifdef DEBUG_I2C
        Serial.println("****************************\n");
    #endif
}

void ssd1306_128x64_send_data_start(const struct ssd1306_128x64 & disp) {
    #ifdef DEBUG_I2C
        Serial.print("DATA ");
    #endif
    i2c_start(disp.i2c);
    i2c_send(disp.i2c, disp.address);
    i2c_send(disp.i2c, 0x40);  //write data 
    #ifdef DEBUG_I2C
        Serial.print(" [ ");
    #endif
}

void ssd1306_128x64_send_data_stop(const struct ssd1306_128x64 & disp) {
    i2c_stop(disp.i2c);
    #ifdef DEBUG_I2C  
        Serial.println(" ]\n");
    #endif
}

void ssd1306_128x64_set_pos(const struct ssd1306_128x64 & disp, uint8_t x, uint8_t page)
{
  //if (page > 7 || x>127) return;

  ssd1306_128x64_send_cmd(disp, 0xb0 + page, ((x & 0xf0) >> 4) | 0x10, (x & 0x0f) | 0x01);
}

void ssd1306_128x64_send_stream(const struct ssd1306_128x64 & disp, const uint8_t data[], uint8_t num_bytes) {
    for (uint8_t n = 0; n < num_bytes; ++n)
    {
      i2c_send(disp.i2c, data[n]);
    }
}

void ssd1306_128x64_send_data_ntimes(const struct ssd1306_128x64 & disp, uint8_t fill_Data, uint8_t times) {
    for (uint8_t n = 0; n < times; n++)
    {
      i2c_send(disp.i2c, fill_Data);
    }
}

void ssd1306_128x64_send_stream_to_page(const struct ssd1306_128x64 & disp, int x, int page, const uint8_t data[], uint8_t num_bytes) {
  ssd1306_128x64_set_pos(disp, x, page);
  ssd1306_128x64_send_data_start(disp);
  ssd1306_128x64_send_stream(disp, data, num_bytes);
  ssd1306_128x64_send_data_stop(disp);
}

void ssd1306_128x64_fillscreen(const struct ssd1306_128x64 & disp, uint8_t fill_Data) {
  for (uint8_t page = 0; page < 8; ++page)
  {
    ssd1306_128x64_send_cmd(disp, 0xb0 + page); //page0-page1
    ssd1306_128x64_send_cmd(disp, 0x00);   //low column start address
    ssd1306_128x64_send_cmd(disp, 0x10);   //high column start address
    ssd1306_128x64_send_data_start(disp);
    ssd1306_128x64_send_data_ntimes(disp, fill_Data, 128);
    ssd1306_128x64_send_data_stop(disp);

  }
}

void ssd1306_128x64_send_char(const struct ssd1306_128x64 & disp, uint8_t index) {
    for (uint8_t byteIndex = 0; byteIndex<8; ++byteIndex) {    
      i2c_send(disp.i2c, pgm_read_byte(&font[index-32][byteIndex]));   
    }
}

void ssd1306_128x64_scroll_right(const struct ssd1306_128x64 & disp, uint8_t start, uint8_t stop, uint8_t speed) {
    ssd1306_128x64_send_cmd(disp, 0x26, 0x00);
    ssd1306_128x64_send_cmd(disp, start);
    ssd1306_128x64_send_cmd(disp, speed);
    ssd1306_128x64_send_cmd(disp, stop);
    ssd1306_128x64_send_cmd(disp, 0x00, 0xff, 0x2f);  
}

void ssd1306_128x64_stop_scroll(const struct ssd1306_128x64 & disp) {
    ssd1306_128x64_send_cmd(disp, 0x2e);
}

void ssd1306_128x64_print(const struct ssd1306_128x64 & disp, uint8_t x, uint8_t page, const char text[]) {
  for(uint8_t index=0; text[index]!=0;++index)
  {
    ssd1306_128x64_set_pos(disp, x, page); 
    ssd1306_128x64_send_data_start(disp);
    ssd1306_128x64_send_char(disp, text[index]);
    ssd1306_128x64_send_data_stop(disp);
    x+=8;
    
    if(x>127) { //no need to print outside of the display
      return;
    }
  }
}

//};