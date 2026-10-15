#pragma once
#include <stdint.h>
#include <string.h>

struct BrightnessSchedule {
  uint16_t start = 22 * 60;
  uint16_t end = 7 * 60;
  uint8_t level = 16;
  uint8_t enabled = 0;
};

inline bool validSchedule(const BrightnessSchedule &s) {
  return s.enabled <= 1 && s.start < 1440 && s.end < 1440 &&
         s.start != s.end && s.level > 0;
}
inline bool parseScheduleTime(const char *text, uint16_t &minute) {
  if (!text || strlen(text) != 5 || text[2] != ':') return false;
  for (int i = 0; i < 5; ++i)
    if (i != 2 && (text[i] < '0' || text[i] > '9')) return false;
  int hour = (text[0] - '0') * 10 + text[1] - '0';
  int min = (text[3] - '0') * 10 + text[4] - '0';
  if (hour > 23 || min > 59) return false;
  minute = uint16_t(hour * 60 + min);
  return true;
}
inline bool scheduleActive(const BrightnessSchedule &s, uint16_t minute, bool clockValid) {
  if (!clockValid || !s.enabled || !validSchedule(s) || minute >= 1440) return false;
  return s.start < s.end ? minute >= s.start && minute < s.end
                         : minute >= s.start || minute < s.end;
}
inline uint8_t scheduledBrightness(uint8_t normal, const BrightnessSchedule &s,
                                   uint16_t minute, bool clockValid) {
  return scheduleActive(s, minute, clockValid) && s.level < normal ? s.level : normal;
}
