#include <Arduino.h>
#include <SPI.h>
#include "util.h"
//#define DEBUG_I2C
#include "ssd1306_128x64.h"

#define CREATE_STRING_CACHES
#include "cu_protocol.h"

#ifdef USE_WIFI
  #include "ota.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
enum {
 SSD1306_SCL = 25,
 SSD1306_SCL_PORT = 0, //unused
 SPI_SS = 26,
 SPI_SS_PORT = 0, //unused
 SSD1306_ADDRESS = 0x78,
 USEC_DELAY_DATA = 0,
 USEC_DELAY_CLK = 0,
 USEC_DELAY_SPI = 0
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
static uint32_t time_millis;


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


struct cu control_unit;

void prepare_race() {

  display_index = 0xffff;

  ssd1306_128x64_init(display);
  ssd1306_128x64_fillscreen(display, 0x00);

  cu_init(&control_unit);

  for (uint8_t idx = 0; idx < MAX_CARS; idx++) {
    display_index = 1 << idx;

    ssd1306_128x64_print(display, 14 * 8, 0, control_unit.cars[idx].str_cache_number);
    ssd1306_128x64_print(display, 0, 0, "~~___    Car #");
    ssd1306_128x64_print(display, 0, 1, ".-'--`-._");
    ssd1306_128x64_print(display, 0, 2, "'-O----O--");
    // ssd1306_128x64_print(display, 0, 3, "[========]");
    ssd1306_128x64_print(display, 0, 4, "Pos: ");
    ssd1306_128x64_print(display, 0, 5, "Time: ");
    ssd1306_128x64_print(display, 0, 6, "Best: ");
    ssd1306_128x64_print(display, 0, 7, "Lap#: ");
  }
}

void start_communication() {
  display_index = 0xffff;

  Serial2.write(CU_RQ_VERSION, CU_RQ_VERSION_SIZE);

  char buffer[CU_RQ_VERSION_REPLY_SIZE];
  uint8_t len = Serial2.readBytes(buffer, CU_RQ_VERSION_REPLY_SIZE);

  if (cu_process_version_reply(buffer, len)) {
    Serial.printf("CU replied with version number: %s\n", buffer);
    ssd1306_128x64_print(display, 9 * 8, 3, buffer);

    Serial2.write(CU_RQ_LAP_TRACK_STATE, CU_RQ_LAP_TRACK_STATE_SIZE);
  } else if (len > 0) {
    buffer[7] = 0;
    char disp[20];
    Serial.printf("Unknown reply from CU: %s\n", buffer);
    uint8_t *v = (uint8_t *)buffer;
    snprintf(disp, sizeof(disp), "E:%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
             v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
    ssd1306_128x64_print(display, 0, 1, disp);
    ssd1306_128x64_print(display, 10 * 8, 3, "Ver?");
    ssd1306_128x64_print(display, 2 * 8, 3, buffer);
  } else {
    Serial.print("No reply from CU\n");
    ssd1306_128x64_print(display, 10 * 8, 3, "No CU!");
  }

  time_millis = millis();
}


void setup() {
  Serial.begin(115200);

  Serial2.begin(19200, SERIAL_8N1, 16, 17);
  Serial2.setTimeout(1000);

#ifdef USE_WIFI
  setupOTA("SuperPositionTower", "", "");
#endif

  prepare_race();
  Serial.println("Race prepared.");

  start_communication();
  Serial.println("Communication started.");
}

uint8_t no_reply_counter = 0;
void loop() {

#ifdef USE_WIFI
  handleOTA();
#endif

  
  char buffer[CU_RQ_LAP_TRACK_STATE_REPLY_SIZE];

  auto byte_count = Serial2.available();
  if (byte_count > 0 || no_reply_counter==40) {
    if(no_reply_counter == 40) {
      display_index = 0xffff;
      ssd1306_128x64_print(display, 9 * 8, 3, "E:NoRe");
    } else {
      
      uint8_t len = Serial2.readBytes(buffer, CU_RQ_LAP_TRACK_STATE_REPLY_SIZE);

      cu_process_lap_or_track_reply(&control_unit, buffer, len);
    }
    Serial2.write(CU_RQ_LAP_TRACK_STATE, CU_RQ_LAP_TRACK_STATE_SIZE);
    no_reply_counter = 0;
  }
  no_reply_counter++;

  auto currentTimeMillis = millis();
  for (uint8_t idx = 0; idx < MAX_CARS; idx++) {
    display_index = 1 << idx;
    auto &car = control_unit.cars[idx];

    if (car.dirty | cu_car::CACHE_FLAG_NUMBER) {
      ssd1306_128x64_print(display, 14 * 8, 0, car.str_cache_number);
    }
    if (car.dirty | cu_car::CACHE_FLAG_FUEL) {
      ssd1306_128x64_print(display, 0, 3, car.str_cache_fuel);
    }
    if (car.dirty | cu_car::CACHE_FLAG_POSITION) {
      ssd1306_128x64_print(display, 6 * 8, 4, car.str_cache_position);
    }

    // we always have to update the time
    float time_passed =
        (currentTimeMillis - car.last_timestamp) / 1000.0f;
    snprintf(buffer, sizeof(buffer), "%1.3f ", time_passed);
    ssd1306_128x64_print(display, 6 * 8, 5, buffer);

    if (car.dirty | cu_car::CACHE_FLAG_BEST_TIME) {
      ssd1306_128x64_print(display, 6 * 8, 6, car.str_cache_best_time);
    }
    if (car.dirty | cu_car::CACHE_FLAG_LAP_COUNT) {
      ssd1306_128x64_print(display, 6 * 8, 7, car.str_cache_lap_count);
    }
    car.dirty = 0;
  }
}