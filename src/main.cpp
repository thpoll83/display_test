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

struct race {
  enum START_STATE { IN_PROGRESS = 0, PREPARING = 1, COUNTDOWN_5 = 2, COUNTDOWN_4 = 3, COUNTDOWN_3 = 4, COUNTDOWN_2 = 5, COUNTDOWN_1 = 6, COUNTDOWN_0 = 7};
  uint8_t start_state;
  enum STATE  { FUEL_UNUSED = 0, FUEL_NORMAL = 1, FUEL_REAL = 2, FLAG_PIT_LANE=4, FLAG_COUNTER=8};
  uint8_t state;

  race() : start_state(IN_PROGRESS), state(FUEL_UNUSED) {}
};

struct car {
  uint32_t last_timestamp_counter; //timestamp of the MCU when the last_timestamp was written
  uint32_t last_timestamp; //last timestamp when car passed CU
  uint32_t best_time; //best time so far (in milli sec)
  uint16_t lap_count; //current lap count
  uint8_t number; //car number (1~6)
  uint8_t position; //position in the current race
  uint8_t gasoline; //gasoline in the tank 0~15
  enum DIRTY_CACHE {BEST_TIME=1,LAP_COUNT=2, NUMBER=4, POSITION=8, GASOLINE=16};
  uint8_t dirty;
  char str_cache_best_time[11];
  char str_cache_lap_count[6];
  char str_cache_number[2];
  char str_cache_position[2];
  char str_cache_gasoline[11];
};

#define MAX_CARS 6
static struct race RACE;
static struct car CARS[MAX_CARS];

static uint8_t REQUEST_STATE[] = {(uint8_t)'"', (uint8_t)'?'};
static uint8_t REQUEST_CU_VERSION[] = {(uint8_t)'"', (uint8_t)'0'};

void prepare_race() {

  display_index = 0xffff;
  
  ssd1306_128x64_init(display);
  ssd1306_128x64_fillscreen(display, 0x00);

  memset(CARS, 0, sizeof(CARS));

  for(uint8_t idx=0;idx<MAX_CARS;idx++) {
    display_index = 1 << idx;
    
    CARS[idx].best_time = UINT32_MAX;
    CARS[idx].number = idx+1;
    CARS[idx].str_cache_number[0] = static_cast<char>('1' + idx);
    ssd1306_128x64_print(display, 14*8, 0, CARS[idx].str_cache_number);
    ssd1306_128x64_print(display, 0, 0, "~~___    Car #" );
    ssd1306_128x64_print(display, 0, 1, ".-'--`-._");
    ssd1306_128x64_print(display, 0, 2, "'-O----O--");
    //ssd1306_128x64_print(display, 0, 3, "[========]");
    ssd1306_128x64_print(display, 0, 4, "Pos: ");
    ssd1306_128x64_print(display, 0, 5, "Time: ");
    ssd1306_128x64_print(display, 0, 6, "Best: ");
    ssd1306_128x64_print(display, 0, 7, "Lap#: ");
  }
}

void start_communication() {
  display_index = 0xffff;

  //second serial port for communication with CU
  //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial2.write(REQUEST_CU_VERSION, sizeof(REQUEST_CU_VERSION)); //query CU serial number
  
  char version[8];
  if(Serial2.readBytes(version, 7)>0){
    if(version[0]=='0') {
      version[0] = ' ';
      version[5] = 0;
      Serial.printf("CU replied with version number: %s\n", version);
      ssd1306_128x64_print(display, 0, 3, version);
    } else {
      version[7] = 0;
      Serial.printf("Unknown reply from CU: %s\n", version);
      ssd1306_128x64_print(display, 0, 3, "?");
      ssd1306_128x64_print(display, 2*8, 3, version);
    }
  } else {
    Serial.print("No reply from CU\n");
    ssd1306_128x64_print(display, 10*8, 3, "No CU!");
  }

  Serial.println("Initialized.");
  time_millis = millis();

  Serial2.print("\"?");
}

void set_fuel_bar(uint8_t idx, uint8_t fuel){
  CARS[idx].dirty |= car::GASOLINE;
  CARS[idx].gasoline = fuel;
  switch(fuel) {
    case 0:  memcpy(CARS[idx].str_cache_gasoline, "[        ]", 10); break;
    case 1:  memcpy(CARS[idx].str_cache_gasoline, "[-       ]", 10); break;
    case 2:  memcpy(CARS[idx].str_cache_gasoline, "[=       ]", 10); break;
    case 3:  memcpy(CARS[idx].str_cache_gasoline, "[=-      ]", 10); break;
    case 4:  memcpy(CARS[idx].str_cache_gasoline, "[==      ]", 10); break;
    case 5:  memcpy(CARS[idx].str_cache_gasoline, "[==-     ]", 10); break;
    case 6:  memcpy(CARS[idx].str_cache_gasoline, "[===     ]", 10); break;
    case 7:  memcpy(CARS[idx].str_cache_gasoline, "[===-    ]", 10); break;
    case 8:  memcpy(CARS[idx].str_cache_gasoline, "[====    ]", 10); break;
    case 9:  memcpy(CARS[idx].str_cache_gasoline, "[====-   ]", 10); break;
    case 10: memcpy(CARS[idx].str_cache_gasoline, "[=====   ]", 10); break;
    case 11: memcpy(CARS[idx].str_cache_gasoline, "[=====-  ]", 10); break;
    case 12: memcpy(CARS[idx].str_cache_gasoline, "[======  ]", 10); break;
    case 13: memcpy(CARS[idx].str_cache_gasoline, "[======- ]", 10); break;
    case 14: memcpy(CARS[idx].str_cache_gasoline, "[======= ]", 10); break;
    case 15: memcpy(CARS[idx].str_cache_gasoline, "[=======-]", 10); break;
    default: memcpy(CARS[idx].str_cache_gasoline, "[   ??   ]", 10); break;
  }
}

int sort_order(const void *v1, const void *v2)
{
  int a = *((uint32_t *)v1);
  int b = *((uint32_t *)v2);
  // The comparison
  return a > b ? 1 : (a < b ? -1 : 0);

}

void evaluate_positions(){
  uint32_t timestamps [] = {
    CARS[0].last_timestamp_counter,
    CARS[1].last_timestamp_counter,
    CARS[2].last_timestamp_counter,
    CARS[3].last_timestamp_counter,
    CARS[4].last_timestamp_counter,
    CARS[5].last_timestamp_counter};
  
  const auto elemSize = sizeof(uint32_t);
  const auto length = sizeof(timestamps) / elemSize;
  qsort(timestamps, length, elemSize, sort_order);

  //TODO update 
}

void process_serial_data(int byte_count) {
  char buffer[64];

  if(byte_count>64) {
    Serial.printf("Reply too big (%d bytes)", byte_count);
    display_index = 0xffff;
    snprintf(buffer, sizeof(buffer), "E:%db", byte_count);
    ssd1306_128x64_print(display, 9*8, 3, buffer);
    return;
  }
  
  memset(buffer,0,sizeof(buffer));
  Serial2.readBytes(buffer, sizeof(buffer));
  if(buffer[0]!='?' || buffer[byte_count-1]=='$') {
      display_index = 0xffff;
      ssd1306_128x64_print(display, 9*8, 3, "E:BrkRe");
      Serial.printf("Broken reply %s", buffer);
      return;
  }

  switch(byte_count) {
    //Byte in reply
    //0   : '?' Reply (type 1)
    //1   : Car index starting with 1; 7 == ghost car, 8== pace car
    //2~9 : 32bit counter when passing the finish line
    //10  : Sensor group; 1 == CU
    //11  : Parity (ignored for now)
    //12  : '$' End of message
    case 13:
    {
      uint8_t car_idx = buffer[1]-1;
      uint8_t sensor_group = buffer[10];
      if(car_idx<6 && sensor_group==1) {
        CARS[car_idx].lap_count++;
        sprintf(CARS[car_idx].str_cache_best_time, "%u", CARS[car_idx].lap_count);
        CARS[car_idx].dirty |= car::LAP_COUNT;

        uint32_t timestampCuCounter;
        memcpy(&timestampCuCounter, &buffer[2], 8);
        if(CARS[car_idx].last_timestamp_counter!=0) {
          uint32_t newLapTime = timestampCuCounter - CARS[car_idx].last_timestamp_counter;
          
          if(newLapTime<CARS[car_idx].best_time) {
            CARS[car_idx].best_time = newLapTime;
            sprintf(CARS[car_idx].str_cache_best_time, "%1.3f", static_cast<float>(newLapTime) / 1000.0f);
            CARS[car_idx].dirty |= car::BEST_TIME;
          }
        }
        CARS[car_idx].last_timestamp = millis();
        CARS[car_idx].last_timestamp_counter = timestampCuCounter;
         evaluate_positions();
      }
    } 
    break;

    //Byte in reply
    //0    : '?' Reply
    //1    : ':' Type 2 
    //2~7  : Fuel values for car 1 to 6 each from 0 to 16(==full)
    //8,9  : Unknown, maybe fuel of ghost and pace car?
    //10    : Start indicator
    //11   : Fuel mode
    //12,13: Bitmask for cars in the pitlane (not clear yet)
    //14   : Number of cars to be displayed by the position tower (6 or 8); only supporting 6 at the moment
    //15,16: Optional values depending on CU version (purpose unknown)
    //15/17: Parity
    //16/18: '$' End of message
    case 17:
    case 19:
    {
      for(uint8_t i=0;i<6;++i) {
        set_fuel_bar(0, buffer[i+2]);
      }
      RACE.start_state = buffer[10];
      if(RACE.start_state!=race::COUNTDOWN_1) {
        prepare_race();//reset at this point
      }
      RACE.state = buffer[11];
    }
    break;
    default:
      display_index = 0xffff;
      ssd1306_128x64_print(display, 9*8, 3, "E:UknRe");
      Serial.printf("Unknown reply %s", buffer);
      break;
  }
}

void setup() {
  Serial.begin(115200);


  Serial2.begin(19200, SERIAL_8N1, 16, 17);
  Serial2.setTimeout(1000);

  Serial.println("Initialized.");

  prepare_race();
  Serial.println("Race prepared.");

  start_communication();
  Serial.println("Communication started.");
}

void loop() {
  
  char buffer[16];
  auto currentTimeMillis = millis();  

  auto byte_count = Serial2.available();
  if(byte_count>0) {
    process_serial_data(byte_count);
    Serial2.write(REQUEST_STATE, sizeof(REQUEST_STATE));
  }
  

  for(uint8_t idx=0;idx<MAX_CARS;idx++) {
    display_index = 1 << idx;
       
    if(CARS[idx].dirty|car::NUMBER) {
      ssd1306_128x64_print(display, 14*8, 0, CARS[idx].str_cache_number);
      CARS[idx].dirty &= car::NUMBER;
    }
    if(CARS[idx].dirty|car::GASOLINE) {
      ssd1306_128x64_print(display, 0, 3, CARS[idx].str_cache_gasoline);
      CARS[idx].dirty &= car::GASOLINE;
    }
    if(CARS[idx].dirty|car::POSITION) {
      ssd1306_128x64_print(display, 6*8, 4, CARS[idx].str_cache_position);
      CARS[idx].dirty &= car::POSITION;
    }
    
    //we always have to update the time
    float time_passed = (currentTimeMillis - CARS[idx].last_timestamp) / 1000.0f;
    snprintf(buffer, sizeof(buffer), "%1.3f ", time_passed);
    ssd1306_128x64_print(display, 6*8, 5, buffer);

    if(CARS[idx].dirty|car::BEST_TIME) {
        ssd1306_128x64_print(display, 6*8, 6, CARS[idx].str_cache_best_time);
        CARS[idx].dirty &= car::BEST_TIME;
    }
    if(CARS[idx].dirty|car::LAP_COUNT) {
      ssd1306_128x64_print(display, 6*8, 7, CARS[idx].str_cache_lap_count);
      CARS[idx].dirty &= car::LAP_COUNT;
    }
  }

}