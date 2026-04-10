#include "../Inc/state_machine.h"

static uint32_t user_interval = 10;
volatile bool fire_flag = false;

void set_user_interval(const uint32_t interval) {
  user_interval = interval;
}
uint32_t get_user_interval() {
  return user_interval;
}
void set_fire_flag(const bool flag) {
  fire_flag = flag;
}

void state_machine_update(void) {
  if (fire_flag) {
    fire_flag = false;
    shutter_fire();
  }
}