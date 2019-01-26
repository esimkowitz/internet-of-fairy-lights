/*
 * Project internet-of-fairy-lights-firmware
 * Description: A mesh network designed to connect fairy lights to the cloud.
 * Author: Evan Simkowitz
 * Date: 1/26/2019
 */

#define fairy_light_pin_1 A0
#define fairy_light_pin_2 A2

int change_mode(String new_mode);

enum Mode {
  alternate_blink,
  steady
} blink_mode;

unsigned long alternate_blink_last_time = 0;
unsigned long alternate_blink_delta_time = 250;
bool alternate_blink_state = 0;

unsigned long steady_last_time = 0;
unsigned long steady_delta_time = 2;
bool steady_state = 0;


// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  pinMode(fairy_light_pin_1, OUTPUT);
  pinMode(fairy_light_pin_2, OUTPUT);

  Particle.function("change_mode", change_mode);

  blink_mode = alternate_blink;
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  switch (blink_mode) {
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

int change_mode(String new_mode) {
  new_mode = new_mode.toLowerCase();
  if (new_mode == "alternate_blink") {
    blink_mode = alternate_blink;
  } else if (new_mode == "steady") {
    blink_mode = steady;
  } else return -1;
  return 1;
}