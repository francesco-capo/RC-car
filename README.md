# 🚗 Arduino RC Car with ESP32 Wi-Fi Control


A rear-wheel-drive remote-controlled (RC) car powered by an **Arduino Uno** and an **ESP32** Wi-Fi receiver. The car features PWM speed control, servo-actuated steering, an active horn, an onboard DHT11 weather station displayed on an I2C LCD, and an automated failsafe system that halts the vehicle if Wi-Fi connection is lost.

---

## 📸 

> *Add your car photos and build snapshots here!*

| Car Overview | Front Steering & Electronics |
| :---: | :---: |
| ![RC Car Front View](docs/images/car_photo_1.jpg) | ![Electronics & Chassis](docs/images/car_photo_2.jpg) |

*(Place your images in `docs/images/` or update the paths above)*

---

## ⚡ Key Features

* **Wireless Wi-Fi Control**: Commands received through an ESP32 bridge communicating via `SoftwareSerial` using robust packet framing (`<X,Y,Horn>`).
* **Proportional PWM Acceleration**: Driven by an **L293D H-Bridge**, allowing smooth speed modulation from 0 to 255 in both forward and reverse directions.
* **Smart Steering with Anti-Jitter Filter**: Front steering driven by a servo motor. Incorporates a deadband/hysteresis filter ($\ge 2^\circ$) to eliminate servo chatter and humming.
* **Safety Failsafe Mechanism**: Automatic heartbeat watchdog (`TIMEOUT_WIFI = 1000ms`). If communication is interrupted, the car instantly cuts motor power, centers the steering, and silences the horn.
* **Onboard Weather Station**: Reads ambient temperature ($^\circ\text{C}$) and relative humidity ($\%$) via a **DHT11** sensor.
* **Non-Blocking Architecture**: Uses `millis()` timers instead of blocking `delay()` calls, ensuring real-time control loop responsiveness while telemetry updates every 2 seconds.
* **16x2 I2C LCD Dashboard**: Real-time onboard display showing weather and vehicle status over a 2-wire I2C bus.
* **Integrated Horn**: Active buzzer triggered directly from the controller.

---

## 📐 Circuit Schematic & Wiring Diagram

> *Add your circuit diagram or Fritzing schematic here!*

![Schematic Diagram](docs/images/circuit_schematic.png)

### Pinout Mapping

| Component | Component Pin | Arduino Uno Pin | Description |
| :--- | :--- | :--- | :--- |
| **L293D H-Bridge** | `Enable 1` (ENA) | **Pin 11** | PWM speed control (Timer 2) |
| **L293D H-Bridge** | `Input 1` (IN1) | **Pin 13** | Direction control (Forward) |
| **L293D H-Bridge** | `Input 2` (IN2) | **Pin 12** | Direction control (Reverse) |
| **L293D H-Bridge** | `Output 1, 2` | DC Motor Terminals | Motor drive outputs |
| **Steering Servo** | Signal Wire | **Pin 9** | Servo PWM signal (Timer 1) |
| **Horn / Buzzer** | Signal / Positive | **Pin 4** | Active buzzer trigger |
| **DHT11 Sensor** | Data Pin | **Pin 10** | Temperature & Humidity 1-wire |
| **ESP32 Serial** | `TX` (ESP32) $\rightarrow$ `RX` | **Pin 2** | `SoftwareSerial` RX |
| **ESP32 Serial** | `RX` (ESP32) $\leftarrow$ `TX` | **Pin 3** | `SoftwareSerial` TX |
| **I2C LCD Display** | `SDA` | **Pin A4** | Hardware I2C Data line |
| **I2C LCD Display** | `SCL` | **Pin A5** | Hardware I2C Clock line |
| **Power Supply** | `VCC` / `GND` | `5V` / `GND` | Logic power (*Common GND!*) |

> [!IMPORTANT]
> **Power Isolation**: Always power the DC motor and servo from an external battery pack (e.g., 6V–9V). Do **not** power inductive loads directly from the Arduino 5V rail. **Always connect all Ground (GND) lines together** (Battery GND + Arduino GND + ESP32 GND + L293D GND).

---

## 📡 Serial Communication Protocol

The ESP32 sends control packets to the Arduino over serial at **9600 baud** using start/end delimiter framing:

$$\texttt{<X,Y,Horn>}$$

| Field | Range | Description |
| :---: | :---: | :--- |
| **`X`** | `-100` to `+100` | Steering angle (`-100` = Full Left, `0` = Center, `+100` = Full Right) |
| **`Y`** | `-100` to `+100` | Throttle (`-100` = Full Reverse, `0` = Stop, `+100` = Full Forward) |
| **`Horn`** | `0` or `1` | Horn trigger (`0` = Off, `1` = Sounding) |

**Example packet**: `<0,75,0>` commands the car to steer straight, drive forward at 75% throttle, with the horn off.

---

## 🛠️ Hardware Requirements

* 1x **Arduino Uno** (or compatible ATmega328P board)
* 1x **ESP32 Development Board** (Wi-Fi receiver / bridge)
* 1x **L293D Dual H-Bridge Motor Driver**
* 1x **DC Gear Motor** (Rear traction)
* 1x **Micro Servo Motor** (SG90 or MG90S for front steering)
* 1x **DHT11** Temperature and Humidity Sensor
* 1x **16x2 LCD Display** with PCF8574 I2C Backpack
* 1x **5V Active Buzzer**
* 1x 2WD / 4WD Robot Car Chassis with front steering assembly
* 1x Battery Holder / Li-ion 18650 Pack (for motors and microcontrollers)
* Breadboard / PCB and jumper wires

---

## 💻 Software & Libraries

The project is configured for **[PlatformIO](https://platformio.org/)** on VS Code. Dependencies are handled automatically via [`platformio.ini`](platformio.ini):

* **`arduino-libraries/Servo`** (Standard Arduino servo management)
* **`marcoschwartz/LiquidCrystal_I2C`** (I2C LCD driver)
* **`robtillaart/DHTlib`** (DHT11 sensor reading)
* **`SoftwareSerial`** & **`Wire`** (Arduino AVR built-in cores)

---

## 🚀 Getting Started

### 1. Prerequisites
* Install [Visual Studio Code](https://code.visualstudio.com/).
* Install the **PlatformIO IDE** extension.

### 2. Clone the Repository
```bash
git clone https://github.com/your-username/arduino-wifi-rc-car.git
cd arduino-wifi-rc-car
```

### 3. Build & Upload
1. Connect your Arduino Uno to your computer via USB.
2. Open the project folder in VS Code with PlatformIO.
3. Click the **PlatformIO: Build** checkmark $(\checkmark)$ in the status bar to compile.
4. Click the **PlatformIO: Upload** arrow $(\rightarrow)$ to flash the firmware onto the board.

---

## ⚙️ Calibration & Tuning

Open [`src/main.cpp`](src/main.cpp) to adjust vehicle parameters:

```cpp
// Adjust steering physical limits to avoid straining your servo
const int STEER_MIN = 45;    // Max left turn
const int STEER_CENTER = 90; // Wheels straight
const int STEER_MAX = 135;   // Max right turn

// Deadband to ignore noisy inputs at rest
const int DEATH_ZONE = 10;   // Threshold around 0 where motor stays stopped

// Failsafe watchdog timer (in milliseconds)
const unsigned long TIMEOUT_WIFI = 1000;
```

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

