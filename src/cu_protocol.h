#pragma once

#include <Arduino.h>

#define MAX_CARS 6

#define CU_GET_NIBBLE(x) static_cast<uint8_t>(x & 0x0f)

constexpr uint8_t CU_RQ_LAP_TRACK_STATE[] = {(uint8_t)'"', (uint8_t)'?'};
constexpr uint8_t CU_RQ_LAP_TRACK_STATE_SIZE = sizeof(CU_RQ_LAP_TRACK_STATE);
constexpr uint8_t CU_RQ_LAP_TRACK_STATE_REPLY_SIZE = 32;

constexpr uint8_t CU_RQ_VERSION[] = {(uint8_t)'"', (uint8_t)'0'};
constexpr uint8_t CU_RQ_VERSION_SIZE = sizeof(CU_RQ_VERSION);
constexpr uint8_t CU_RQ_VERSION_REPLY_SIZE = 8;

#define FULL_TRACK_UPDATE_MSEC 4000

struct cu_track {
  enum STATE {
    STATE_RACE_IN_PROGRESS = 0,
    STATE_PREPARING_COUNTDOWN = 1,
    STATE_COUNTDOWN_5 = 2,
    STATE_COUNTDOWN_4 = 3,
    STATE_COUNTDOWN_3 = 4,
    STATE_COUNTDOWN_2 = 5,
    STATE_COUNTDOWN_1 = 6,
    STATE_COUNTDOWN_0 = 7,
    STATE_STARTED_TOO_EARLY_0 = 8,
    STATE_STARTED_TOO_EARLY_1 = 9
  };
  uint8_t state;
  enum FLAGS {
    FLAGS_UNSET = 0,
    FLAGS_FUEL_UNUSED = 0,
    FLAGS_FUEL_NORMAL = 1,
    FLAGS_FUEL_REAL = 2,
    FLAGS_FLAG_PIT_LANE = 4,
    FLAGS_FLAG_COUNTER = 8
  };
  uint8_t flags;

  uint32_t last_full_update;
};

struct cu_car {
  uint32_t last_timestamp_counter; // timestamp of the MCU when the
                                   // last_timestamp was written
  uint32_t last_timestamp; // last timestamp when car passed CU
  uint32_t best_time;      // best time so far (in milli sec)
  uint16_t lap_count;      // current lap count
  uint8_t number;          // car number (1~6)
  uint8_t position;        // position in the current race
  uint8_t fuel;            // gasoline in the tank 0~15

#ifdef CREATE_STRING_CACHES
  enum CACHE_FLAG {
    CACHE_FLAG_BEST_TIME = 1,
    CACHE_FLAG_LAP_COUNT = 1<<1,
    CACHE_FLAG_NUMBER = 1<<2,
    CACHE_FLAG_POSITION = 1<<3,
    CACHE_FLAG_FUEL = 1<<4
  };
  uint8_t dirty;

  char str_cache_best_time[11];
  char str_cache_lap_count[6];
  char str_cache_number[2];
  char str_cache_position[2];
  char str_cache_fuel[11];
#endif
};

struct cu {
  struct cu_track track;
  struct cu_car cars[MAX_CARS];
  struct cu_car *positioned_cars[MAX_CARS];

  enum REPLY{  REPLY_EMPTY = 1,
  REPLY_BROKEN = 1<<1 ,
  REPLY_LAP_STATUS = 1 << 2,
  REPLY_IGNORED_INTERIM = 1 <<3,
  REPLY_IGNORED_PACE_CAR = 1 <<4,
  REPLY_IGNORED_GHOST_CAR = 1 <<5,
  REPLY_TRACK_STATUS = 1 <<5,
   };
  enum CONSTANTS{ TRACK_STATUS = 0xa, GHOST_CAR_NUMBER = 7, PACE_CAR_NUMBER = 8 };
};

void cu_init(struct cu *control_unit) {

  memset(control_unit, 0, sizeof(configUSE_RECURSIVE_MUTEXES));

  for (uint8_t idx = 0; idx < MAX_CARS; idx++) {
    auto& car = control_unit->cars[idx];
    car.best_time = UINT32_MAX;
    car.last_timestamp_counter = UINT32_MAX;
    car.number = idx + 1;
#ifdef CREATE_STRING_CACHES
    car.str_cache_number[0] = static_cast<char>('1' + idx);
#endif
    control_unit->positioned_cars[idx] = &car;
  }
}

bool cu_process_version_reply(char *buffer, int byte_count) {

  if (byte_count > 5 && buffer[0] == '0') {
      buffer[0] = 'v';
      buffer[5] = 0;
      return true;
  }
  return false;
}

void cu_set_fuel_bar(struct cu *control_unit, uint8_t idx, uint8_t f) {
  auto val = CU_GET_NIBBLE(f);
  if (control_unit->cars[idx].fuel == val) {
    return;
  }

  control_unit->cars[idx].fuel = val;

#ifdef CREATE_STRING_CACHES
  control_unit->cars[idx].dirty |= cu_car::CACHE_FLAG_FUEL;
  switch(val) {
    case 0:  memcpy(control_unit->cars[idx].str_cache_fuel, "[        ]", 10); break;
    case 1:  memcpy(control_unit->cars[idx].str_cache_fuel, "[-       ]", 10); break;
    case 2:  memcpy(control_unit->cars[idx].str_cache_fuel, "[=       ]", 10); break;
    case 3:  memcpy(control_unit->cars[idx].str_cache_fuel, "[=-      ]", 10); break;
    case 4:  memcpy(control_unit->cars[idx].str_cache_fuel, "[==      ]", 10); break;
    case 5:  memcpy(control_unit->cars[idx].str_cache_fuel, "[==-     ]", 10); break;
    case 6:  memcpy(control_unit->cars[idx].str_cache_fuel, "[===     ]", 10); break;
    case 7:  memcpy(control_unit->cars[idx].str_cache_fuel, "[===-    ]", 10); break;
    case 8:  memcpy(control_unit->cars[idx].str_cache_fuel, "[====    ]", 10); break;
    case 9:  memcpy(control_unit->cars[idx].str_cache_fuel, "[====-   ]", 10); break;
    case 10: memcpy(control_unit->cars[idx].str_cache_fuel, "[=====   ]", 10); break;
    case 11: memcpy(control_unit->cars[idx].str_cache_fuel, "[=====-  ]", 10); break;
    case 12: memcpy(control_unit->cars[idx].str_cache_fuel, "[======  ]", 10); break;
    case 13: memcpy(control_unit->cars[idx].str_cache_fuel, "[======- ]", 10); break;
    case 14: memcpy(control_unit->cars[idx].str_cache_fuel, "[======= ]", 10); break;
    case 15: memcpy(control_unit->cars[idx].str_cache_fuel, "[========]", 10); break;
    default: break;
  }
#endif
}

struct cu_lap_timestamp_and_idx {
  uint16_t lap;
  uint32_t ts;
  uint8_t idx;
};

int cu_sort_order(const void *v1, const void *v2) {
  auto lap_a = ((cu_lap_timestamp_and_idx *)v1)->lap;
  auto lap_b = ((cu_lap_timestamp_and_idx *)v2)->lap;

  //in case of identical lap we compare timestamps
  if(lap_a==lap_b) {
    auto a = ((cu_lap_timestamp_and_idx *)v1)->ts;
    auto b = ((cu_lap_timestamp_and_idx *)v2)->ts;
    return a > b ? 1 : (a < b ? -1 : 0);
  }
  return lap_a < lap_b ? 1 : (lap_a > lap_b ? -1 : 0);
}

void cu_evaluate_positions(struct cu *control_unit){
  cu_lap_timestamp_and_idx timestamps [] = {
      {control_unit->cars[0].lap_count, control_unit->cars[0].last_timestamp_counter, 0},
      {control_unit->cars[1].lap_count, control_unit->cars[1].last_timestamp_counter, 1},
      {control_unit->cars[2].lap_count, control_unit->cars[2].last_timestamp_counter, 2},
      {control_unit->cars[3].lap_count, control_unit->cars[3].last_timestamp_counter, 3},
      {control_unit->cars[4].lap_count, control_unit->cars[4].last_timestamp_counter, 4},
      {control_unit->cars[5].lap_count, control_unit->cars[5].last_timestamp_counter, 5}
    };

  const auto elemSize = sizeof(cu_lap_timestamp_and_idx);
  const auto length = sizeof(timestamps) / elemSize;
  qsort(timestamps, length, elemSize, cu_sort_order);

  for (uint8_t pos = 0; pos < MAX_CARS; pos++) {
    auto idx = timestamps[pos].idx;
    auto& car = control_unit->cars[idx];
    car.position = pos + 1;
#ifdef CREATE_STRING_CACHES
    control_unit->cars[idx].dirty |= cu_car::CACHE_FLAG_POSITION;
    car.str_cache_position[0] = '1' + pos;
#endif
    control_unit->positioned_cars[pos] = &car;
  }
}

uint32_t cu_read_32bit_from_nibbles(char* buffer){
  return (static_cast<uint32_t>(CU_GET_NIBBLE(buffer[1])) << 28) + (static_cast<uint32_t>(CU_GET_NIBBLE(buffer[0])) << 24) +
    (static_cast<uint32_t>(CU_GET_NIBBLE(buffer[3])) << 20) + (static_cast<uint32_t>(CU_GET_NIBBLE(buffer[2])) << 16) +
    (static_cast<uint32_t>(CU_GET_NIBBLE(buffer[5])) << 12) + (static_cast<uint32_t>(CU_GET_NIBBLE(buffer[4])) << 8) +
    (static_cast<uint32_t>(CU_GET_NIBBLE(buffer[7])) << 4) +  (static_cast<uint32_t>(CU_GET_NIBBLE(buffer[6])));
}

uint8_t cu_process_lap_or_track_reply(struct cu *control_unit, char *buffer, int byte_count) {
  // empty (happens when connecting)
  if (buffer[0] == 0) {
    return cu::REPLY_EMPTY;
  }

  if (buffer[0] != '?' || buffer[byte_count - 1] != '$') {
    return cu::REPLY_BROKEN;
  }

  // LAP STATUS REPLY
  // Byte in reply
  // 0   : '?' Reply (type 1)
  // 1   : Car index starting with 1; 7 == ghost car, 8== pace car
  // 2~9 : 32bit counter when passing the finish line
  // 10  : Sensor group; 1 == CU
  // 11  : Parity (ignored for now)
  // 12  : '$' End of message
  uint8_t car_number = CU_GET_NIBBLE(buffer[1]);

  if (car_number != cu::TRACK_STATUS) {
    uint8_t car_idx = car_number - 1;
    uint8_t sensor_group = CU_GET_NIBBLE(buffer[10]);
    // currently we do not track the ghost/ pace cars nor interim timings
    if (car_idx >= MAX_CARS) {
      uint8_t reply = car_number == cu::GHOST_CAR_NUMBER
                          ? cu::REPLY_IGNORED_GHOST_CAR
                          : cu::REPLY_IGNORED_PACE_CAR;
      if (sensor_group == 1) {
        reply |= cu::REPLY_IGNORED_INTERIM;
      }
      return reply;
    }
    if (sensor_group != 1) {
      return cu::REPLY_IGNORED_INTERIM;
    }
    auto& car = control_unit->cars[car_idx];
    car.lap_count++;
#ifdef CREATE_STRING_CACHES
    snprintf(car.str_cache_lap_count, 6, "%u",
            car.lap_count);
    car.dirty |= cu_car::CACHE_FLAG_LAP_COUNT;
#endif
    uint32_t timestampCuCounter = cu_read_32bit_from_nibbles(&buffer[2]);
    if (car.last_timestamp_counter != 0) {
      uint32_t newLapTime =
          timestampCuCounter - car.last_timestamp_counter;

      if (newLapTime < car.best_time) {
        car.best_time = newLapTime;
#ifdef CREATE_STRING_CACHES
        snprintf(car.str_cache_best_time, 11, "%1.3f  ",
                static_cast<float>(newLapTime) / 1000.0f);
        car.dirty |= cu_car::CACHE_FLAG_BEST_TIME;
#endif
      }
    }
    car.last_timestamp = millis();
    car.last_timestamp_counter = timestampCuCounter;

    cu_evaluate_positions(control_unit);
    return cu::REPLY_LAP_STATUS;
  }

  // TRACK STATUS REPLY
  // Byte in reply
  // 0    : '?' Reply
  // 1    : 0x0a Type 2
  // 2~7  : Fuel values for car 1 to 6 each from 0 to 16(==full)
  // 8,9  : Unknown, maybe fuel of ghost and pace car?
  // 10   : Start indicator
  // 11   : Flags (fuel mode etc)
  // 12,13: Bitmask for cars in the pitlane (not clear yet)
  // 14   : Number of cars to be displayed by the position tower (6 or 8);
  //        only supporting 6 at the moment
  //15,16: Optional values depending on CU
  //       version (purpose unknown)
  //15/17: Parity 16/18: '$' End of message

  uint8_t new_state = CU_GET_NIBBLE(buffer[10]);
  auto& tr = control_unit->track;

  if (new_state == cu_track::STATE_RACE_IN_PROGRESS &&
      tr.state == cu_track::STATE_COUNTDOWN_0) {
    cu_init(control_unit);
  } else {
    tr.state = new_state;
  }

  const auto timestamp = millis();
  if(timestamp - tr.last_full_update > FULL_TRACK_UPDATE_MSEC ) {
    tr.last_full_update = timestamp;

    for (uint8_t i = 0; i < 6; ++i) {
      cu_set_fuel_bar(control_unit, i, buffer[i + 2]);
    }

    tr.flags = CU_GET_NIBBLE(buffer[11]);
  }
  return cu::REPLY_TRACK_STATUS;
}