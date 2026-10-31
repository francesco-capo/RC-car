#define REMOTEXY_MODE__WIFI_POINT
#include <WiFi.h>
#include <RemoteXY.h>

//WIFI SETTIGS

#define REMOTEXY_WIFI_SSID "RC_CAR"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377

// REMOTEXY INTERFACE (BUTTON AND JOYSTICK)

#pragma pack(push, 1)
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] = { 255, 3, 0, 0, 0, 31, 0, 19, 0, 0, 0, 0, 31, 1, 200, 84, 1, 1, 2, 0,
                                                  5, 118, 19, 49, 49, 32, 2, 26, 31, 1, 41, 28, 26, 26, 0, 2, 31, 0 };


struct {

  int8_t joystick_x;
  int8_t joystick_y;
  uint8_t clacson;


  uint8_t connect_flag;

} RemoteXY;
#pragma pack(pop)

//SET UP VARIABLES

unsigned long lastSendTime = 0;
const int sendingDelay = 50;

//SETUP

void setup() {

  //REMOTE XY

  RemoteXY_Init();

  // SERIAL2 INITIALIZATION

  Serial2.begin(9600, SERIAL_8N1, 16, 17);
}

//LOOP

void loop() {
  // REMOTEXY
  RemoteXY_Handler();

  unsigned long currentTime = millis();


  if (currentTime - lastSendTime >= sendingDelay) {
    lastSendTime = currentTime;

    // DATA PACKET <X,Y,C>
    // EXAMPLE: "<100,50,1>" (STEER 100, GAS 50, HORN)
    String packet = "<" + String(RemoteXY.joystick_x) + ","
                    + String(RemoteXY.joystick_y) + ","
                    + String(RemoteXY.clacson) + ">";


    Serial2.println(packet);
  }
}
