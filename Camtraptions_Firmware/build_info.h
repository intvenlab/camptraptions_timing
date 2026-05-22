#pragma once

#include <stdint.h>

constexpr uint8_t parseTwoDigits(char tens, char ones) {
  return (uint8_t)((tens - '0') * 10 + (ones - '0'));
}

constexpr uint16_t parseFourDigits(char a, char b, char c, char d) {
  return (uint16_t)((a - '0') * 1000 + (b - '0') * 100 + (c - '0') * 10 + (d - '0'));
}

constexpr uint8_t parseBuildMonth(const char* dateStr) {
  return (dateStr[0] == 'J' && dateStr[1] == 'a') ? 1  :
         (dateStr[0] == 'F')                           ? 2  :
         (dateStr[0] == 'M' && dateStr[2] == 'r')      ? 3  :
         (dateStr[0] == 'A' && dateStr[1] == 'p')      ? 4  :
         (dateStr[0] == 'M' && dateStr[2] == 'y')      ? 5  :
         (dateStr[0] == 'J' && dateStr[2] == 'n')      ? 6  :
         (dateStr[0] == 'J' && dateStr[2] == 'l')      ? 7  :
         (dateStr[0] == 'A' && dateStr[1] == 'u')      ? 8  :
         (dateStr[0] == 'S')                           ? 9  :
         (dateStr[0] == 'O')                           ? 10 :
         (dateStr[0] == 'N')                           ? 11 :
                                                        12;
}

constexpr uint8_t parseBuildDay(const char* dateStr) {
  return (dateStr[4] == ' ')
    ? (uint8_t)(dateStr[5] - '0')
    : parseTwoDigits(dateStr[4], dateStr[5]);
}

constexpr uint16_t BUILD_YEAR  = parseFourDigits(__DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10]);
constexpr uint8_t  BUILD_MONTH = parseBuildMonth(__DATE__);
constexpr uint8_t  BUILD_DAY   = parseBuildDay(__DATE__);
constexpr uint8_t  BUILD_HOUR  = parseTwoDigits(__TIME__[0], __TIME__[1]);
constexpr uint8_t  BUILD_MIN   = parseTwoDigits(__TIME__[3], __TIME__[4]);
constexpr uint8_t  BUILD_SEC   = parseTwoDigits(__TIME__[6], __TIME__[7]);
