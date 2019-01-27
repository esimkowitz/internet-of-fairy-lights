/*
 * Project internet-of-fairy-lights-firmware
 * Description: A mesh network designed to connect fairy lights to the cloud.
 * Author: Evan Simkowitz
 * Date: 1/26/2019
 */

#define fairy_light_pin_1 A0
#define fairy_light_pin_2 A2

int change_mode(String new_mode, bool publish);

#if (PLATFORM_ID == PLATFORM_ARGON)
int change_mode_function_handler(String new_mode);
#endif

enum Mode {
  off,
  alternate_blink,
  steady
} blink_mode;


void off_handler()
{
  analogWrite(fairy_light_pin_2, 0);
  analogWrite(fairy_light_pin_1, 0);
}

Timer off_timer(1, off_handler, true);

unsigned long alternate_blink_delta_time = 250;
bool alternate_blink_state = false;

void alternate_blink_handler()
{
  if (alternate_blink_state == false) {
    analogWrite(fairy_light_pin_1, 0);
    analogWrite(fairy_light_pin_2, 230);
  } else {
    analogWrite(fairy_light_pin_2, 0);
    analogWrite(fairy_light_pin_1, 230);
  }
  alternate_blink_state = !alternate_blink_state;
}

Timer alternate_blink_timer(alternate_blink_delta_time, alternate_blink_handler);

unsigned long steady_delta_time = 10;
bool steady_state = false;

void steady_handler()
{
  if (steady_state == false) {
    analogWrite(fairy_light_pin_1, 0);
    analogWrite(fairy_light_pin_2, 230);
  } else {
    analogWrite(fairy_light_pin_2, 0);
    analogWrite(fairy_light_pin_1, 230);
  }
  steady_state = !steady_state;
}

Timer steady_timer(steady_delta_time, steady_handler);


void change_mode_subscribe_handler(const char *event, const char *data)
{
  String new_mode = String(data);
  change_mode(new_mode, false);
}

// setup() runs once, when the device is first turned on.
void setup() {
  pinMode(fairy_light_pin_1, OUTPUT);
  pinMode(fairy_light_pin_2, OUTPUT);

  analogWrite(fairy_light_pin_1, 0);
  analogWrite(fairy_light_pin_2, 0);

  #if (PLATFORM_ID == PLATFORM_ARGON)
  Particle.function("change_mode", change_mode_function_handler);
  #endif

  Mesh.subscribe("new_mode", change_mode_subscribe_handler);

  blink_mode = off;
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // do nothing, everything is handled by timers
}

#if (PLATFORM_ID == PLATFORM_ARGON)
int change_mode_function_handler(String new_mode) {
  return change_mode(new_mode, true);
}
#endif

void change_timer(Mode old_mode, Mode new_mode) {
  switch (old_mode) {
    case off:
      // no need to stop off_timer, it stops automatically
      break;
    case steady:
      steady_timer.stop();
      break;
    case alternate_blink:
      alternate_blink_timer.stop();
    default:
      break;
  }
  switch (new_mode) {
    case off:
      off_timer.start();
      break;
    case steady:
      steady_timer.start();
      break;
    case alternate_blink:
      alternate_blink_timer.start();
    default:
      break;
  }
  return;
}

int change_mode(String new_mode, bool publish) {
  new_mode = new_mode.toLowerCase();
  Mode old_mode = blink_mode;
  if (new_mode == "off") {
    blink_mode = off;
  } else if (new_mode == "alternate_blink") {
    blink_mode = alternate_blink;
  } else if (new_mode == "steady") {
    blink_mode = steady;
  } else return -1;
  if (publish == true) {
    Mesh.publish("new_mode", new_mode);
  }
  if (old_mode != blink_mode) {
    change_timer(old_mode, blink_mode);
  }
  return 1;
}