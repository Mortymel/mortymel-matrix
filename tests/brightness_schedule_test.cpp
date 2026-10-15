#include <assert.h>
#include "brightness_schedule.h"
int main() {
  BrightnessSchedule s; s.enabled = 1;
  assert(scheduleActive(s, 22*60, true));
  assert(scheduleActive(s, 0, true));
  assert(scheduleActive(s, 7*60-1, true));
  assert(!scheduleActive(s, 7*60, true));
  assert(!scheduleActive(s, 21*60+59, true));
  assert(scheduledBrightness(96, s, 0, true) == 16);
  assert(scheduledBrightness(8, s, 0, true) == 8);
  assert(scheduledBrightness(96, s, 0, false) == 96);
  assert(scheduledBrightness(96, s, 8*60, true) == 96);
  s.enabled = 0; assert(!scheduleActive(s, 0, true));
  s.enabled = 1; s.start = 9*60; s.end = 17*60;
  assert(scheduleActive(s, 9*60, true));
  assert(!scheduleActive(s, 17*60, true));
  assert(!scheduleActive(s, 0, true));
  s.end = s.start; assert(!validSchedule(s));
  uint16_t minute;
  assert(parseScheduleTime("00:00", minute) && minute == 0);
  assert(parseScheduleTime("23:59", minute) && minute == 1439);
  assert(!parseScheduleTime("24:00", minute));
  assert(!parseScheduleTime("22:60", minute));
  assert(!parseScheduleTime("7:00", minute));
  assert(!parseScheduleTime("ab:cd", minute));
  assert(!parseScheduleTime(nullptr, minute));
}
