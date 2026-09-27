# 🛠️ Manhole Worker Safety & Smart Monitoring System

A low-cost, ESP32-based safety system for workers in manholes and other confined spaces. It checks the air, temperature, distance to obstacles, and tilt — then shows everything live on a webpage you open on your phone. No internet, no app, no SIM card — each ESP32 creates its own Wi-Fi hotspot that you connect to directly.

This repo contains **three independent sketches**. Each runs on its own ESP32 board and creates its own Wi-Fi network:

| # | Sketch | Board | Purpose |
|---|---|---|---|
| 1 | `Manhole_monitoring_bot_esp32code.ino` | Regular ESP32 Dev Board | **Main safety dashboard** — gas, temperature, distance, tilt, servo, LEDs |
| 2 | `manhole_bot_espcam_servo_code.ino` | ESP32-CAM (AI-Thinker) | Live video feed + pan servo (optional add-on) |
| 3 | `esp32_web_robot.ino` | Regular ESP32 Dev Board + motor driver | Drives a 2-motor wheeled chassis (optional add-on) |

All wiring details below are taken directly from the pin definitions in each `.ino` file, so they match exactly what the code expects.

---

## 🧠 How It Works (Simple Version)

1. The ESP32 is wired to sensors (gas, temperature, distance, tilt) and/or a motor driver / camera.
2. It reads the sensors continuously.
3. It creates its own Wi-Fi hotspot (Access Point) — no router needed.
4. You connect your phone/laptop to that Wi-Fi.
5. You open a webpage that shows live numbers with color-coded warnings (🟢 Safe, 🟡 Warning, 🔴 Danger) and lets you control the servo/LEDs/motors/camera.

---

## 🧰 What You Need (Shopping List)

**For the main monitoring node:**

| Part | Qty |
|---|---|
| ESP32 Dev Board | 1 |
| HC-SR04 Ultrasonic Sensor | 4 |
| MQ-2 / MQ-135 Gas Sensor (analog output) | 1 |
| DHT11 Temperature & Humidity Sensor | 1 |
| MPU-6050 Gyroscope/Accelerometer | 1 |
| SG90 Servo Motor | 1 |
| LED + 220Ω resistor | 2 |
| Breadboard + jumper wires | - |
| 5V regulated power supply (2A+) | 1 |

**For the optional camera add-on:** ESP32-CAM (AI-Thinker) board, an SG90 servo, a USB-to-TTL (FTDI) programmer.

**For the optional robot-drive add-on:** L298N (or similar) 2-channel motor driver, 2 DC gear motors + wheels, chassis, separate motor battery pack.

---

## 🔌 CONNECTIONS GUIDE #1 — Main Monitoring Node
*(`Manhole_monitoring_bot_esp32code.ino`)*

> ⚠️ Connect the **GND** of every sensor to the ESP32's GND (all grounds must be common). Power sensors from **3.3V** unless a table says otherwise. The servo should get 5V from an external supply if possible.

### Ultrasonic Sensors (HC-SR04) — 4 units
Each has 4 pins: `VCC`, `TRIG`, `ECHO`, `GND`.

| Sensor | VCC | TRIG → ESP32 GPIO | ECHO → ESP32 GPIO | GND |
|---|---|---|---|---|
| Ultrasonic #1 | 5V | **5** | **18** | GND |
| Ultrasonic #2 | 5V | **19** | **21** | GND |
| Ultrasonic #3 | 5V | **32** | **14** | GND |
| Ultrasonic #4 | 5V | **25** | **26** | GND |

> 💡 HC-SR04's ECHO pin outputs 5V, but ESP32 GPIOs read 3.3V max. For a permanent build, add a voltage divider (e.g. 1kΩ in series + 2kΩ to GND) on every ECHO line to protect the pin.

### Gas Sensor (MQ-2 / MQ-135)
| Sensor Pin | Connect to |
|---|---|
| VCC | 5V |
| GND | GND |
| **AO** (Analog Out) | **GPIO 34** |
| DO (Digital Out) | Not used |

### DHT11 (Temperature & Humidity)
| Sensor Pin | Connect to |
|---|---|
| VCC | 3.3V |
| GND | GND |
| **DATA** | **GPIO 4** |

> 💡 Add a 10kΩ pull-up resistor between VCC and DATA if your DHT11 module doesn't already have one built in.

### MPU-6050 (Gyroscope / Accelerometer) — I2C
| Sensor Pin | Connect to |
|---|---|
| VCC | 3.3V |
| GND | GND |
| **SDA** | **GPIO 22** |
| **SCL** | **GPIO 23** |

### Servo Motor (Pan Control)
| Servo Wire | Connect to |
|---|---|
| Signal (orange/yellow) | **GPIO 33** |
| Power (red) | 5V (external supply recommended) |
| Ground (brown/black) | GND (shared with ESP32) |

### LEDs (Status Indicators)
| LED | Positive leg → 220Ω resistor → | Negative leg → |
|---|---|---|
| LED 1 | **GPIO 13** | GND |
| LED 2 | **GPIO 2** | GND |

### Full Pin Summary — Monitoring Node

| Function | GPIO |
|---|---|
| Ultrasonic 1 TRIG / ECHO | 5 / 18 |
| Ultrasonic 2 TRIG / ECHO | 19 / 21 |
| Ultrasonic 3 TRIG / ECHO | 32 / 14 |
| Ultrasonic 4 TRIG / ECHO | 25 / 26 |
| Gas Sensor (Analog) | 34 |
| DHT11 Data | 4 |
| MPU-6050 SDA / SCL | 22 / 23 |
| Servo Signal | 33 |
| LED 1 | 13 |
| LED 2 | 2 |

**Wi-Fi:** SSID `ESP32_ROBOT`, password `12345678` → dashboard at `http://192.168.4.1`

---

## 🔌 CONNECTIONS GUIDE #2 — Robot Drive Module (Optional)
*(`esp32_web_robot.ino`, two-channel motor driver such as L298N)*

> ⚠️ Motors draw much more current than the ESP32 can supply — always power the motor driver's motor terminals from a **separate battery pack**, and connect that battery's GND to the ESP32's GND (common ground is required, even with separate power).

| Motor Driver Pin | Connect to ESP32 GPIO | Purpose |
|---|---|---|
| IN1 (Left motor direction A) | **GPIO 18** | Left forward |
| IN2 (Left motor direction B) | **GPIO 19** | Left backward |
| ENA (Left motor speed/PWM) | **GPIO 21** | Left speed control |
| IN3 (Right motor direction A) | **GPIO 22** | Right forward |
| IN4 (Right motor direction B) | **GPIO 23** | Right backward |
| ENB (Right motor speed/PWM) | **GPIO 25** | Right speed control |
| Driver GND | ESP32 GND | Common ground |
| Driver logic VCC (5V, if separate from motor supply) | ESP32 5V or its own regulator | Logic power |
| Motor power terminals (+ / –) | External battery pack | Do NOT power motors from the ESP32 |

Connect your two DC motors to the driver's `OUT1/OUT2` (left) and `OUT3/OUT4` (right) screw terminals as usual for your specific driver board.

**Wi-Fi:** SSID `Yoo` → dashboard/captive portal at `http://192.168.4.1`.
> ⚠️ Note: the password inside the code (`AP_PASS`) is set to `123456798`, while the comment at the top of the file lists a different number (`8722519872`). **Trust the Serial Monitor output after boot** — it will print the exact password the board is actually using — and update the comment or the constant so they match.

---

## 🔌 CONNECTIONS GUIDE #3 — ESP32-CAM Video Module (Optional)
*(`manhole_bot_espcam_servo_code.ino`, AI-Thinker ESP32-CAM board)*

The camera itself is already wired to the board internally (OV2640 ribbon cable) — you don't need to wire the camera pins yourself. You only need to add the pan servo:

| Part | Connect to |
|---|---|
| Servo Signal wire | **GPIO 13** |
| Servo Power (red) | 5V (use an external 5V supply — the ESP32-CAM's onboard regulator can brown out under camera + servo load) |
| Servo Ground | GND (shared with ESP32-CAM) |
| Flash LED | Already built onto the board on **GPIO 4** — no wiring needed |
| Status LED | Already built onto the board on **GPIO 33** — no wiring needed |

### Programming Note
The ESP32-CAM has **no built-in USB port**, so you need a separate USB-to-TTL (FTDI) adapter to upload code:

1. Wire FTDI **TX → U0R (RX)**, **RX → U0T (TX)**, **GND → GND**, **5V → 5V**.
2. Connect **GPIO 0 to GND** before powering on — this puts the board into flashing mode.
3. Upload the sketch.
4. Disconnect GPIO 0 from GND and press the reset button to run the program normally.

**Wi-Fi:** SSID `ESP32-CAM-PRO`, password `12345678` → `http://192.168.4.1`
**Web login:** username `admin`, password `esp32cam`

---

## 🚀 Getting Started (Software Setup)

### Step 1 — Install Arduino IDE Libraries
**Sketch → Include Library → Manage Libraries**, then install:
- `ESP32` (by Espressif Systems)
- `ESP32Servo` (by Kevin Harrington)

### Step 2 — Upload the Code
1. Open the `.ino` file for the module you're building.
2. Select the matching board:
   - Monitoring / robot sketches → your ESP32 Dev Board model
   - Camera sketch → **AI Thinker ESP32-CAM**
3. Select the correct COM port and click **Upload**.

### Step 3 — Connect & Open the Dashboard

| Sketch | Wi-Fi Name (SSID) | Wi-Fi Password | Open in browser |
|---|---|---|---|
| Monitoring Node | `ESP32_ROBOT` | `12345678` | `http://192.168.4.1` |
| Camera Node | `ESP32-CAM-PRO` | `12345678` | `http://192.168.4.1` (login `admin` / `esp32cam`) |
| Robot Controller | `Yoo` | `123456798` (verify in Serial Monitor) | `http://192.168.4.1` (opens automatically) |

**Steps:**
1. Power on the ESP32.
2. On your phone/laptop, join the Wi-Fi network above.
3. Open a browser and go to `http://192.168.4.1`.
4. The dashboard/controller loads automatically.

---

## 🌐 Web Endpoints (For Developers)

### Monitoring Node
| Endpoint | What it does |
|---|---|
| `/` | Loads the dashboard |
| `/data` | Live sensor readings as JSON |
| `/toggle_led` | Turns LED 1 on/off |
| `/toggle_led2` | Turns LED 2 on/off |
| `/servo?pos=<0-180>` | Moves the servo to an angle |

### Camera Node (needs login)
| Endpoint | What it does |
|---|---|
| `/stream` | Live MJPEG video feed |
| `/capture` | Downloads one JPEG photo |
| `/toggle_flash` | Turns the flash LED on/off |
| `/servo?angle=<0-180>` | Moves the pan servo |
| `/control?var=<name>&val=<value>` | Sets `framesize`, `brightness`, `contrast`, `vflip`, `hmirror`, or `special_effect` |

### Robot Controller
| Endpoint | What it does |
|---|---|
| `/cmd?dir=forward\|backward\|left\|right\|stop` | Moves the robot |
| `/speed?val=<0-255>` | Sets motor PWM speed |

---

## 🚦 What the Dashboard Colors Mean

- **Distance sensors:** 🔴 Danger under 15 cm · 🟡 Warning under 35 cm · 🟢 Clear otherwise
- **Gas sensor (raw ADC):** 🟢 Clean under 500 · 🟡 Moderate under 1500 · 🟠 High under 3000 · 🔴 Danger at 3000+

These thresholds live in the dashboard's JavaScript — adjust them to match your site's real safety rules before deployment.

---

## 🔐 Before Real-World Use

- Change every default Wi-Fi name/password and the camera login (`admin` / `esp32cam`).
- Fix the SSID password mismatch in `esp32_web_robot.ino` (see Connections Guide #2 note above).
- Use a proper regulated 5V power supply for servos/motors — phone chargers and USB power banks can brown out under load.
- Test every sensor individually before final assembly.

---

## 🐛 Common Problems

| Problem | Try this |
|---|---|
| Sensor numbers look "too smooth" or don't change with reality | That sensor is wired incorrectly — the firmware fills in a placeholder signal so the dashboard never freezes |
| Servo doesn't move | Confirm it's on the correct GPIO for that sketch (33 for monitoring node, 13 for camera node), and power it from a separate 5V source |
| Can't open the dashboard | Make sure you joined the ESP32's own Wi-Fi network, not your home Wi-Fi |
| Camera page says "401 Unauthorized" | Username/password doesn't match what's set in the code |
| Robot doesn't respond / Wi-Fi password fails | Check the Serial Monitor for the exact password actually flashed to the board |
| Ultrasonic readings jump around | Add the ECHO voltage divider mentioned above; keep sensors away from soft or angled surfaces |

---

## 🗺️ Ideas to Improve This Project

- [ ] Add a buzzer or SMS/Telegram alert when danger thresholds are hit
- [ ] Log data to an SD card or the cloud for history/trends
- [ ] Add a battery level indicator
- [ ] Merge the camera and sensor dashboards into a single page

---


Team Ginza:  Hanish , Gagan , Atharv
