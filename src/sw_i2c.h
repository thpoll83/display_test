#pragma once
#include <Arduino.h>

struct sw_i2c {
    const uint8_t port_scl;
    const uint8_t port_sda;
    const uint8_t delay_data_usec;
    const uint8_t delay_clk_usec;
    void (*digital_port_high)(uint8_t);
    void (*digital_port_low)(uint8_t);
    void (*digital_port_enable)(uint8_t);
};

void i2c_init(const struct sw_i2c* i2c) {
    #ifdef DEBUG_I2C
        Serial.println("*** Init SW I2C ***");
        char buffer[32];
        sprintf (buffer, "SCL Port: 0x%02x\n", i2c->port_scl);
        Serial.print(buffer);

        sprintf (buffer, "SDA Port: 0x%02x\n", i2c->port_sda);
        Serial.print(buffer);

        sprintf (buffer, "Data Delay: %d usec\n", i2c->delay_data_usec);
        Serial.print(buffer);
        sprintf (buffer, "Clock Delay: %d usec\n", i2c->delay_clk_usec);
        Serial.print(buffer);
        Serial.println("*******************\n");
    #endif

    i2c->digital_port_enable(i2c->port_scl);
    i2c->digital_port_enable(i2c->port_sda);
}

void i2c_send(const struct sw_i2c* i2c, uint8_t byte) {
    for (uint8_t i = 0; i < 9; ++i)
    {
        if (i==8 || (byte << i) & 0x80) {
            i2c->digital_port_high(i2c->port_sda);
        }   else {
            i2c->digital_port_low(i2c->port_sda);
        }
        delayMicroseconds(i2c->delay_clk_usec);

        i2c->digital_port_high(i2c->port_scl);
        delayMicroseconds(i2c->delay_data_usec);
        i2c->digital_port_low(i2c->port_scl);

        delayMicroseconds(i2c->delay_clk_usec);
    }

    i2c->digital_port_low(i2c->port_sda);
    delayMicroseconds(i2c->delay_clk_usec);
    #ifdef DEBUG_I2C
        char buffer[8];
        sprintf (buffer, "0x%02x,", byte);
        Serial.print (buffer);
    #endif
    }

void i2c_start(const struct sw_i2c* i2c) {
    i2c->digital_port_high(i2c->port_scl);
    i2c->digital_port_high(i2c->port_sda);
    delayMicroseconds(i2c->delay_clk_usec);
    i2c->digital_port_low(i2c->port_sda);
    delayMicroseconds(i2c->delay_clk_usec);
    i2c->digital_port_low(i2c->port_scl);
    delayMicroseconds(i2c->delay_clk_usec);
}

void i2c_stop(const struct sw_i2c* i2c) {
    i2c->digital_port_low(i2c->port_scl);
    i2c->digital_port_low(i2c->port_sda);
    delayMicroseconds(i2c->delay_clk_usec);
    i2c->digital_port_high(i2c->port_scl);
    delayMicroseconds(i2c->delay_clk_usec);
    i2c->digital_port_high(i2c->port_sda);
    delayMicroseconds(i2c->delay_clk_usec);
}