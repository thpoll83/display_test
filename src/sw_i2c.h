#pragma once
#include <Arduino.h>

struct sw_i2c {
    const uint8_t delay_data_usec;
    const uint8_t delay_scl_usec;
    void (*set_scl_high)();
    void (*set_scl_low)();
    void (*set_sda_high)();
    void (*set_sda_low)();
    void (*enable_scl)();
    void (*enable_sda)();
};

void i2c_init(const struct sw_i2c* i2c) {
    #ifdef DEBUG_I2C
        Serial.println("*** Init SW I2C ***");
        char buffer[32];
        sprintf (buffer, "Data Delay: %d usec\n", i2c->delay_data_usec);
        Serial.print(buffer);
        sprintf (buffer, "Clock Delay: %d usec\n", i2c->delay_scl_usec);
        Serial.print(buffer);
        Serial.println("*******************\n");
    #endif

    i2c->enable_scl();
    i2c->enable_sda();
}

void i2c_send(const struct sw_i2c* i2c, uint8_t byte) {
    for (uint8_t i = 0; i < 9; ++i)
    {
        if (i==8 || (byte << i) & 0x80) {
            i2c->set_sda_high();
        }   else {
            i2c->set_sda_low();
        }
        delayMicroseconds(i2c->delay_scl_usec);

        i2c->set_scl_high();
        delayMicroseconds(i2c->delay_data_usec);
        i2c->set_scl_low();

        delayMicroseconds(i2c->delay_scl_usec);
    }

    i2c->set_sda_low();
    delayMicroseconds(i2c->delay_scl_usec);
    #ifdef DEBUG_I2C
        char buffer[8];
        sprintf (buffer, "0x%02x,", byte);
        Serial.print (buffer);
    #endif
    }

void i2c_start(const struct sw_i2c* i2c) {
    i2c->set_scl_high();
    i2c->set_sda_high();
    delayMicroseconds(i2c->delay_scl_usec);
    i2c->set_sda_low();
    delayMicroseconds(i2c->delay_scl_usec);
    i2c->set_scl_low();
    delayMicroseconds(i2c->delay_scl_usec);
}

void i2c_stop(const struct sw_i2c* i2c) {
    i2c->set_scl_low();
    i2c->set_sda_low();
    delayMicroseconds(i2c->delay_scl_usec);
    i2c->set_scl_high();
    delayMicroseconds(i2c->delay_scl_usec);
    i2c->set_sda_high();
    delayMicroseconds(i2c->delay_scl_usec);
}