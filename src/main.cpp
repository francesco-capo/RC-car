#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <dht.h>
#include <SoftwareSerial.h>

// PIN CONFIGURATION

// H L293D PINOUT
const int PIN_L293D_ENA = 11;
const int PIN_L293D_IN1 = 13;
const int PIN_L293D_IN2 = 12;

// SERVO
const int PIN_STEERING = 9;

// BUZZER
const int PIN_BUZZER = 4;

// ANALOG JOYSTICK PIN
/*
const int PIN_JOY_X = A0;
const int PIN_JOY_Y = A1;
const int PIN_JOY_SW = 2;
*/

// DHT11 SENSOR
const int PIN_DHT = 10;

// RX-TX SERIAL

const int RXSERIAL = 2;
const int TXSERIAL = 3;

// LCD I2C PIN DEFAULT
// SDA -> Pin A4
// SCL -> Pin A5

// SET UP VARIABLES

// JOYSTICK ( 0 - 1023 CENTER AT ~512)
/*
const int JOYSTICK_CENTER = 512;
const int DEAD_ZONE = 40; // Intervallo [472, 552] in cui l'auto sta ferma
*/

// DIGITAL JOYSTICK (-100 - 100)
const int DEAD_ZONE = 10;
int valueX = 0;
int valueY = 0;
int horn = 0;

// TIMEOUT WIFI
unsigned long LastSignalWiFi = 0;
const unsigned long TIMEOUT_WIFI = 1000;

// STEERING ANGLE
const int STEER_MIN = 45;
const int STEER_CENTER = 90;
const int STEER_MAX = 135;
int LastAngle = STEER_CENTER;

// UPDATE TIME DHT SENSOR AND LCD SCREEN
const unsigned long DELAY_WEATHER = 2000;
unsigned long lastWeatherUpdate = 0;

Servo servo1;
dht DHT;
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial espSerial(RXSERIAL, TXSERIAL);

// DC MOTOR CONTROL PWM

void forward(int speed)
{
  digitalWrite(PIN_L293D_IN1, HIGH);
  digitalWrite(PIN_L293D_IN2, LOW);
  analogWrite(PIN_L293D_ENA, speed);
}

void backwards(int speed)
{
  digitalWrite(PIN_L293D_IN1, LOW);
  digitalWrite(PIN_L293D_IN2, HIGH);
  analogWrite(PIN_L293D_ENA, speed);
}

void stopMotor()
{
  digitalWrite(PIN_L293D_IN1, LOW);
  digitalWrite(PIN_L293D_IN2, LOW);
  analogWrite(PIN_L293D_ENA, 0);
}

// METEO UPDATE

void updateWeather()
{
  int chk = DHT.read11(PIN_DHT);

  if (chk == DHTLIB_OK)
  {
    lcd.setCursor(0, 0);
    lcd.print("    WEATHER     ");
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print((int)DHT.temperature);
    lcd.print((char)223);
    lcd.print("C  U:");
    lcd.print((int)DHT.humidity);
    lcd.print("%   ");
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.print("Sensor Error");
  }
}

// READ ESPCOMMAND

void ReadESP32()
{
  static boolean onGoing = false;
  static String buffer = "";
  while (espSerial.available() > 0)
  {
    char c = espSerial.read();
    if (c == '<')
    {
      onGoing = true;
      buffer = "";
    }
    else if (c == '>')
    {
      onGoing = false;
      int firstComma = buffer.indexOf(',');
      int secondComma = buffer.indexOf(',', firstComma + 1);
      if (firstComma != -1 && secondComma != -1)
      {
        valueX = buffer.substring(0, firstComma).toInt();
        valueY = buffer.substring(firstComma + 1, secondComma).toInt();
        horn = buffer.substring(secondComma + 1).toInt();
        LastSignalWiFi = millis();
      }
    }
    else if (onGoing)
    {
      buffer += c;
    }
  }
}

// SETUP

void setup()
{
  // H BRIDGE
  pinMode(PIN_L293D_ENA, OUTPUT);
  pinMode(PIN_L293D_IN1, OUTPUT);
  pinMode(PIN_L293D_IN2, OUTPUT);
  stopMotor();

  // BUZZER
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  // JOYSTICK PULLUP RESISTOR
  // pinMode(PIN_JOY_SW, INPUT_PULLUP);

  // SERVO
  servo1.attach(PIN_STEERING);
  servo1.write(STEER_CENTER);

  // LCD I2C
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("     WEATHER      ");
  lcd.setCursor(0, 1);
  lcd.print("  SETTING UP...");

  Serial.begin(9600);
  Serial.println("Ready");
  espSerial.begin(9600);
  LastSignalWiFi = millis();
}

// LOOP

void loop()
{

  // ANALOG JOYSTICK READ
  /*
  int valueX = analogRead(PIN_JOY_X);
  int valueY = analogRead(PIN_JOY_Y);
*/

  ReadESP32();

  // FAILSAFE--> STOP THE CAR

  if (millis() - LastSignalWiFi > TIMEOUT_WIFI)
  {
    valueX = 0;
    valueY = 0;
    horn = 0;
  }

  // SERVO STEERING

  int angolo = map(valueX, -100, 100, STEER_MIN, STEER_MAX);
  angolo = constrain(angolo, STEER_MIN, STEER_MAX);
  if (abs(angolo - LastAngle) >= 2)
  {
    servo1.write(angolo);
    LastAngle = angolo;
  }

  // ANALOG CONTROL OF THE DC MOTOR (forward / backwards / Stop)

  /*
  if (valueY > (JOYSTICK_CENTER + DEAD_ZONE)) {
     int speedPWM = map(valueY, JOYSTICK_CENTER + DEAD_ZONE, 1023, 0, 255);
     speedPWM = constrain(speedPWM, 0, 255);
     forward(speedPWM);
   }
   else if (valueY < (JOYSTICK_CENTER - DEAD_ZONE)) {
     int speedPWM = map(valueY, JOYSTICK_CENTER - DEAD_ZONE, 0, 0, 255);
     speedPWM = constrain(speedPWM, 0, 255);
     backwards(speedPWM);
   }
   else {
     stopMotor();
   }
  */

  // DIGITAL CONTROL OF THE DC MOTOR

  if (valueY > (DEAD_ZONE))
  {
    int speedPWM = map(valueY, DEAD_ZONE, 100, 0, 255);
    speedPWM = constrain(speedPWM, 0, 255);
    forward(speedPWM);
  }
  else if (valueY < (-DEAD_ZONE))
  {
    int speedPWM = map(abs(valueY), DEAD_ZONE, 100, 0, 255);
    speedPWM = constrain(speedPWM, 0, 255);
    backwards(speedPWM);
  }
  else
  {
    stopMotor();
  }

  // horn ANALOG CONTROL

  /*
  if (digitalRead(PIN_JOY_SW) == LOW) {
    digitalWrite(PIN_BUZZER, HIGH);
  } else {
    digitalWrite(PIN_BUZZER, LOW);
  }
  */

  // DIGITAL CONTROL horn

  if (horn == 1)
  {
    digitalWrite(PIN_BUZZER, HIGH);
  }
  else
  {
    digitalWrite(PIN_BUZZER, LOW);
  }

  // WEATHER

  unsigned long currentTime = millis();
  if (currentTime - lastWeatherUpdate >= DELAY_WEATHER)
  {
    lastWeatherUpdate = currentTime;
    updateWeather();
  }

  delay(15);
}
