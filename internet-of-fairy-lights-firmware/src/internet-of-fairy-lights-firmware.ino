/*
 * Project internet-of-fairy-lights-firmware
 * Description: A mesh network designed to connect fairy lights to the cloud.
 * Author: Evan Simkowitz
 * Date: 1/26/2019
 */

#define fairy_light_pin_1 A0
#define fairy_light_pin_2 A2

#define fairy_light_high 4095

#if (PLATFORM_ID == PLATFORM_XENON)
SYSTEM_MODE(MANUAL);
#endif

float brightness;

enum LightMode
{
  off,
  alternate_blink,
  steady,
  test
} blink_mode;

// Define the functions to handle the change_mode functionality
int change_mode(String new_mode);

// Only the gateway (the Argon device) needs the cloud-facing change_mode function, the others just subscribe.
// This function is the handler for the subscribed event.
#if (PLATFORM_ID == PLATFORM_XENON)
void new_mode_subscribe_handler(const char *event, const char *data)
{
  String new_mode = String(data);
  change_mode(new_mode);
}
#endif

// Define the functions to handle the change_brightness functionality
int change_brightness(String new_brightness);

// Only the gateway (the Argon device) needs the cloud-facing change_brightness function, the others just subscribe.
// This function is the handler for the subscribed event.
#if (PLATFORM_ID == PLATFORM_XENON)
void new_brightness_subscribe_handler(const char *event, const char *data)
{
  String new_brightness = String(data);
  change_brightness(new_brightness);
}
#endif

#if (PLATFORM_ID == PLATFORM_ARGON)
void request_mode_subscribe_handler(const char *event, const char *data)
{
  Particle.publish("request_mode");
  String mode_str;
  switch (blink_mode)
  {
  case off:
    mode_str = "off";
    break;
  case steady:
    mode_str = "steady";
    break;
  case alternate_blink:
    mode_str = "alternate_blink";
    break;
  case test:
    mode_str = "test";
    break;
  default:
    break;
  }
  if (mode_str.length() > 0)
  {
    Particle.publish("new_mode", mode_str);
    Mesh.publish("new_mode", mode_str);
  }
}

void request_brightness_subscribe_handler(const char *event, const char *data)
{
  Mesh.publish("new_brightness", String(brightness));
}
#endif

// Set up the timers for the various blink modes
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
  if (alternate_blink_state == false)
  {
    analogWrite(fairy_light_pin_1, 0);
    analogWrite(fairy_light_pin_2, fairy_light_high * brightness);
  }
  else
  {
    analogWrite(fairy_light_pin_2, 0);
    analogWrite(fairy_light_pin_1, fairy_light_high * brightness);
  }
  alternate_blink_state = !alternate_blink_state;
}

Timer alternate_blink_timer(alternate_blink_delta_time, alternate_blink_handler);

unsigned long steady_delta_time = 10;
bool steady_state = false;

void steady_handler()
{
  if (steady_state == false)
  {
    analogWrite(fairy_light_pin_1, 0);
    analogWrite(fairy_light_pin_2, fairy_light_high * brightness);
  }
  else
  {
    analogWrite(fairy_light_pin_2, 0);
    analogWrite(fairy_light_pin_1, fairy_light_high * brightness);
  }
  steady_state = !steady_state;
}

Timer steady_timer(steady_delta_time, steady_handler);

// Set up the timers for the test mode, which is used to test the voltage and power to the lights
void test_handler()
{
  analogWrite(fairy_light_pin_2, 0);
  analogWrite(fairy_light_pin_1, fairy_light_high * brightness, 1000);
}

Timer test_timer(1, test_handler, true);

#if (PLATFORM_ID == PLATFORM_XENON)
enum MeshMode
{
  mesh_idle,
  mesh_connect,
  mesh_wait_to_subscribe,
  mesh_subscribe,
  mesh_wait_to_publish,
  mesh_publish,
  mesh_wait_to_off,
  mesh_off
} mesh_mode;

unsigned long mesh_connect_last_time = 0;
unsigned long mesh_connect_delta_time = 19000;
unsigned long mesh_subscribe_last_time = 0;
unsigned long mesh_subscribe_offset = 1000;
unsigned long mesh_publish_last_time = 0;
unsigned long mesh_publish_offset = 200;
unsigned long mesh_off_offset = 500;
#endif

// setup() runs once, when the device is first turned on.
void setup()
{
  Serial.begin(9600);
  pinMode(fairy_light_pin_1, OUTPUT);
  pinMode(fairy_light_pin_2, OUTPUT);

  analogWriteResolution(fairy_light_pin_1, 12);
  analogWriteResolution(fairy_light_pin_2, 12);

  analogWrite(fairy_light_pin_1, 0);
  analogWrite(fairy_light_pin_2, 0);

#if (PLATFORM_ID == PLATFORM_ARGON)
  // while (!Particle.connected())
  // {
  //   delay(1000);
  // }
  Particle.function("mode", change_mode);
  Particle.function("brightness", change_brightness);
  Mesh.subscribe("request_mode", request_mode_subscribe_handler);
  Mesh.subscribe("request_brightness", request_brightness_subscribe_handler);
#elif (PLATFORM_ID == PLATFORM_XENON)
  mesh_mode = mesh_idle;
#endif

  blink_mode = off;
  brightness = 1.0;

  // mesh_connect_timer.start();
  Serial.println("finished setup");
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
#if (PLATFORM_ID == PLATFORM_XENON)
  // do nothing, everything is handled by timers
  switch (mesh_mode)
  {
  case mesh_idle:
    if ((millis() - mesh_connect_last_time) > mesh_connect_delta_time)
    {
      mesh_mode = mesh_connect;
    }
    break;
  case mesh_connect:
    mesh_connect_last_time = millis();
    Mesh.on();
    Mesh.connect();
    mesh_mode = mesh_wait_to_subscribe;
    break;
  case mesh_wait_to_subscribe:
    if ((millis() - mesh_connect_last_time) > mesh_subscribe_offset)
    {
      mesh_mode = mesh_subscribe;
    }
    break;
  case mesh_subscribe:
    mesh_subscribe_last_time = millis();
    Mesh.subscribe("new_mode", new_mode_subscribe_handler);
    Mesh.subscribe("new_brightness", new_brightness_subscribe_handler);
    mesh_mode = mesh_wait_to_publish;
    break;
  case mesh_wait_to_publish:
    if ((millis() - mesh_subscribe_last_time) > mesh_subscribe_offset)
    {
      mesh_mode = mesh_publish;
    }
    break;
  case mesh_publish:
    mesh_publish_last_time = millis();
    Mesh.publish("request_mode");
    Mesh.publish("request_brightness");
    mesh_mode = mesh_wait_to_off;
    break;
  case mesh_wait_to_off:
    if ((millis() - mesh_publish_last_time) > mesh_off_offset)
    {
      mesh_mode = mesh_off;
    }
    break;
  case mesh_off:
    Mesh.off();
    mesh_mode = mesh_idle;
    break;
  default:
    mesh_mode = mesh_idle;
    break;
  }
#endif
}

void change_timer(LightMode old_mode, LightMode new_mode)
{
  switch (old_mode)
  {
  case off:
    // no need to stop off_timer, it stops automatically
    break;
  case steady:
    steady_timer.stop();
    break;
  case alternate_blink:
    alternate_blink_timer.stop();
    break;
  case test:
    // no need to stop test_timer, it stops automatically
    break;
  default:
    break;
  }
  switch (new_mode)
  {
  case off:
    off_timer.start();
    break;
  case steady:
    steady_timer.start();
    break;
  case alternate_blink:
    alternate_blink_timer.start();
    break;
  case test:
    test_timer.start();
    break;
  default:
    break;
  }
  return;
}

int change_mode(String new_mode)
{
  new_mode = new_mode.toLowerCase();
  LightMode old_mode = blink_mode;
  if (new_mode == "off")
  {
    blink_mode = off;
  }
  else if (new_mode == "alternate_blink")
  {
    blink_mode = alternate_blink;
  }
  else if (new_mode == "steady")
  {
    blink_mode = steady;
  }
  else if (new_mode == "test")
  {
    blink_mode = test;
  }
  else
    return -1;

// The gateway (the Argon device) publishes a Mesh event that the other devices listen to.
#if (PLATFORM_ID == PLATFORM_ARGON)
  Mesh.publish("new_mode", new_mode);
#endif

  if (old_mode != blink_mode)
  {
    change_timer(old_mode, blink_mode);
  }
  return 1;
}

int change_brightness(String new_brightness)
{
  int new_brightness_int = new_brightness.toInt();
  if (new_brightness_int < 0 || new_brightness_int > 10)
  {
    return -1;
  }

  brightness = new_brightness_int / 10.0;

  // The gateway (the Argon device) publishes a Mesh event that the other devices listen to.
#if (PLATFORM_ID == PLATFORM_ARGON)
  Mesh.publish("new_brightness", new_brightness);
#endif

  return 1;
}