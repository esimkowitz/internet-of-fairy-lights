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

unsigned long alternate_blink_last_time = 0;
unsigned long alternate_blink_delta_time = 250;
bool alternate_blink_state = 0;

unsigned long steady_last_time = 0;
unsigned long steady_delta_time = 2;
bool steady_state = 0;


void change_mode_subscribe_handler(const char *event, const char *data)
{
  String new_mode = String(data);
  change_mode(new_mode, 0);
}

// setup() runs once, when the device is first turned on.
void setup() {
  pinMode(fairy_light_pin_1, OUTPUT);
  pinMode(fairy_light_pin_2, OUTPUT);

  #if (PLATFORM_ID == PLATFORM_ARGON)
  Particle.function("change_mode", change_mode_function_handler);
  #endif

  Mesh.subscribe("new_mode", change_mode_subscribe_handler);

  blink_mode = off;
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  switch (blink_mode) {
    case off:
    {
      analogWrite(fairy_light_pin_1, 0);
      analogWrite(fairy_light_pin_2, 0);
      break;
    }
    case alternate_blink:
    {
      if (millis() - alternate_blink_last_time > alternate_blink_delta_time) {
        alternate_blink_last_time = millis();
        if (alternate_blink_state == 0) {
          analogWrite(fairy_light_pin_1, 0);
          analogWrite(fairy_light_pin_2, 230);
        } else {
          analogWrite(fairy_light_pin_2, 0);
          analogWrite(fairy_light_pin_1, 230);
        }
        alternate_blink_state = !alternate_blink_state;
      }
      break;
    }
    case steady:
    {
      if (millis() - steady_last_time > steady_delta_time) {
        steady_last_time = millis();
        if (steady_state == 0) {
          analogWrite(fairy_light_pin_1, 0);
          analogWrite(fairy_light_pin_2, 230);
        } else {
          analogWrite(fairy_light_pin_2, 0);
          analogWrite(fairy_light_pin_1, 230);
        }
        steady_state = !steady_state;
      }
      break;
    }
  }  
}

#if (PLATFORM_ID == PLATFORM_ARGON)
int change_mode_function_handler(String new_mode) {
  return change_mode(new_mode, 1);
}
#endif

int change_mode(String new_mode, bool publish) {
  new_mode = new_mode.toLowerCase();
  if (new_mode == "off") {
    blink_mode = off;
  } else if (new_mode == "alternate_blink") {
    blink_mode = alternate_blink;
  } else if (new_mode == "steady") {
    blink_mode = steady;
  } else return -1;
  if (publish == 1) {
    Mesh.publish("new_mode", new_mode);
  }
  return 1;
}