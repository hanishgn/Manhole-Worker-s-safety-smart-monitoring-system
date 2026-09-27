# 🛠️ Manhole Worker Safety & Smart Monitoring System

A simple, low-cost safety system for workers in manholes and other confined spaces. It uses an **ESP32** microcontroller with sensors to check the air, temperature, distance to walls/obstacles, and tilt/orientation — then shows everything live on a webpage you can open on your phone. No internet, no app, no SIM card needed — the ESP32 creates its own Wi-Fi network that you connect to directly.

This repo also includes two optional extra modules: a **camera module** for live video, and a **robot driving controller** if the unit is mounted on wheels.

---

## 🧠 How It Works (Simple Version)

1. The ESP32 is wired to several sensors (gas, temperature, distance, tilt).
2. It reads all sensors continuously.
3. It creates its own Wi-Fi hotspot (Access Point).
4. You connect your phone/laptop to that Wi-Fi.
5. You open a webpage in your browser — it shows live numbers and color-coded warnings (🟢 Safe, 🟡 Warning, 🔴 Danger).
6. You can also control a servo motor and two LEDs from the same page.

---

## 📦 Files in This Repository

| File | What it does |
|---|---|
| `Manhole_monitoring_bot_esp32code.ino` | **Main project.** Reads all sensors and runs the live safety dashboard. |
| `manhole_bot_espcam_servo_code.ino` | Optional camera module — live video, snapshots, and a pan servo. |
| `esp32_web_robot.ino` | Optional — drives a 2-motor wheeled robot from a webpage. |
| `ESP32_CAM_GUIDE.md` | Extra setup guide just for the camera module. |

---

## 🎯 What It Measures / Controls

- **Gas levels** (MQ gas sensor) — warns of dangerous air
- **Temperature & humidity** (DHT11)
- **Distance to obstacles/walls** in 4 directions (4x ultrasonic sensors)
- **Tilt / orientation / motion** (MPU-6050 gyroscope+accelerometer)
- **Pan servo motor** — aim a sensor or light
- **2 LEDs** — turn on/off remotely (e.g. warning light)

If any sensor is unplugged or not responding, the dashboard still shows moving numbers (a safe simulated value) instead of freezing, so it always looks "alive."

---

## 🧰 What You Need (Shopping List)

| Part | Qty | Notes |
|---|---|---|
| ESP32 Dev Board (e.g. ESP32-WROOM-32) | 1 | Main brain |
| HC-SR04 Ultrasonic Sensor | 4 | Distance sensing |
| MQ-2 / MQ-135 Gas Sensor (analog) | 1 | Air quality |
| DHT11 Temperature & Humidity Sensor | 1 | |
| MPU-6050 Gyroscope/Accelerometer | 1 | |
| SG90 Servo Motor | 1 | |
| LED | 2 | + 220Ω resistors |
| Breadboard + jumper wires | - | |
| 5V power supply (2A or more) | 1 | USB power bank works for testing |

Optional (camera add-on): ESP32-CAM (AI-Thinker) board + FTDI/USB-TTL programmer.
Optional (robot add-on): L298N motor driver + 2 DC motors + chassis.

---

## 🔌 CONNECTIONS GUIDE (Wiring)

> ⚠️ Always connect **GND** from every sensor to a common **GND** on the ESP32. Power sensors from the **3.3V** pin unless noted; the servo needs **5V** (from an external supply if possible, not directly off the ESP32's onboard regulator, to avoid brownouts).

### 1. Ultrasonic Sensors (HC-SR04) ×4
Each sensor has 4 pins: `VCC`, `TRIG`, `ECHO`, `GND`.

| Sensor | VCC | TRIG → ESP32 | ECHO → ESP32 | GND |
|---|---|---|---|---|
| Ultrasonic #1 | 5V | GPIO 5 | GPIO 18 | GND |
| Ultrasonic #2 | 5V | GPIO 19 | GPIO 21 | GND |
| Ultrasonic #3 | 5V | GPIO 32 | GPIO 14 | GND |
| Ultrasonic #4 | 5V | GPIO 25 | GPIO 26 | GND |

> 💡 Tip: HC-SR04 `ECHO` pin outputs 5V, but ESP32 GPIOs are 3.3V-only. For long-term reliability, use a simple voltage divider (e.g. 1kΩ + 2kΩ resistors) on each ECHO line to protect the ESP32. The demo code works without it, but it's safer for permanent installs.

### 2. Gas Sensor (MQ-2 / MQ-135)
| Sensor Pin | Connect to |
|---|---|
| VCC | 5V |
| GND | GND |
| AO (Analog Out) | GPIO 34 |
| DO (Digital Out) | Not used |

### 3. DHT11 (Temperature & Humidity)
| Sensor Pin | Connect to |
|---|---|
| VCC | 3.3V |
| GND | GND |
| DATA | GPIO 4 |

> 💡 If your DHT11 module doesn't have a built-in pull-up resistor, add a 10kΩ resistor between VCC and DATA.

### 4. MPU-6050 (Gyroscope / Accelerometer)
This uses **I2C**, so only 4 wires:

| Sensor Pin | Connect to |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 22 |
| SCL | GPIO 23 |

### 5. Servo Motor (Pan Control)
| Servo Wire | Connect to |
|---|---|
| Signal (usually orange/yellow) | GPIO 33 |
| Power (red) | 5V (external supply recommended) |
| Ground (brown/black) | GND (shared with ESP32) |

### 6. LEDs (Status Indicators)
| LED | Long leg (+) via 220Ω resistor → | Short leg (–) → |
|---|---|---|
| LED 1 | GPIO 13 | GND |
| LED 2 | GPIO 2 | GND |

### Quick Wiring Summary Table

| Function | GPIO Pin |
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

### Camera Module Wiring (Optional — ESP32-CAM)
The camera pins are fixed by the ESP32-CAM board itself (already wired internally), so you only need to add:

| Part | Connect to |
|---|---|
| Servo Signal | GPIO 13 |
| Flash LED | Built-in on GPIO 4 (no wiring needed) |

> To upload code to ESP32-CAM you need a separate USB-to-TTL (FTDI) programmer, since it has no onboard USB port. Connect GPIO 0 to GND before powering on to enter flashing mode, then disconnect it after uploading.

---

## 🚀 Getting Started (Software Setup)

### Step 1 — Install Arduino IDE Libraries
In Arduino IDE go to **Sketch → Include Library → Manage Libraries**, then install:
- `ESP32` (by Espressif Systems)
- `ESP32Servo` (by Kevin Harrington)

### Step 2 — Upload the Code
1. Open `Manhole_monitoring_bot_esp32code.ino` in Arduino IDE.
2. Select your ESP32 board and correct COM port.
3. Click **Upload**.
4. (Optional) Repeat for the camera or robot sketches on separate boards.

### Step 3 — Connect to the Dashboard
| Sketch | Wi-Fi Name (SSID) | Wi-Fi Password | Open in browser |
|---|---|---|---|
| Monitoring Node | `ESP32_ROBOT` | `12345678` | `http://192.168.4.1` |
| Camera Node | `ESP32-CAM-PRO` | `12345678` | `http://192.168.4.1` (login: `admin` / `esp32cam`) |
| Robot Controller | `Yoo` | `123456798`* | `http://192.168.4.1` (opens automatically) |

\*Check your Serial Monitor after boot to confirm the exact password compiled into your board — the code comment and the actual password constant differ slightly in this sketch.

**Steps:**
1. Power on the ESP32.
2. On your phone or laptop, join the Wi-Fi network listed above.
3. Open a browser and go to `http://192.168.4.1`.
4. The live dashboard/controller appears automatically.

---

## 🌐 Web Endpoints (For Developers)

### Monitoring Node
| Endpoint | What it does |
|---|---|
| `/` | Loads the dashboard |
| `/data` | Returns live sensor readings as JSON |
| `/toggle_led` | Turns LED 1 on/off |
| `/toggle_led2` | Turns LED 2 on/off |
| `/servo?pos=<0-180>` | Moves the servo to an angle |

### Camera Node (needs login)
| Endpoint | What it does |
|---|---|
| `/stream` | Live video feed |
| `/capture` | Downloads one photo |
| `/toggle_flash` | Turns the flash LED on/off |
| `/servo?angle=<0-180>` | Moves the pan servo |
| `/control?var=<name>&val=<value>` | Adjusts brightness, contrast, resolution, effects, flip/mirror |

### Robot Controller
| Endpoint | What it does |
|---|---|
| `/cmd?dir=forward\|backward\|left\|right\|stop` | Moves the robot |
| `/speed?val=<0-255>` | Sets motor speed |

---

## 🚦 What the Colors Mean

- **Distance sensors:** 🔴 Danger under 15 cm · 🟡 Warning under 35 cm · 🟢 Clear otherwise
- **Gas sensor:** 🟢 Clean under 500 · 🟡 Moderate under 1500 · 🟠 High under 3000 · 🔴 Danger at 3000+
- These numbers are just starting points — adjust them in the code to match your site's real safety rules.

---

## 🔐 Before Real-World Use

- Change all default Wi-Fi names/passwords and the camera login (`admin` / `esp32cam`) in the code.
- Use a proper regulated 5V power supply — phone chargers can be unstable under servo/camera load.
- Test every sensor individually before final assembly.

---

## 🐛 Common Problems

| Problem | Try this |
|---|---|
| Numbers look "too smooth" or fake | That sensor isn't wired correctly — the code fills in a placeholder signal so the dashboard never freezes |
| Servo doesn't move | Check it's on GPIO 33 (or 13 for camera), and power it from a separate 5V source |
| Can't open the dashboard | Make sure you joined the ESP32's own Wi-Fi, not your home Wi-Fi |
| Camera page says "401 Unauthorized" | Double check username/password match the code |
| Ultrasonic readings jump around | Add the ECHO voltage divider mentioned above, and keep sensors away from soft/angled surfaces |

---

## 🗺️ Ideas to Improve This Project

- [ ] Add a buzzer or SMS/Telegram alert when danger thresholds are hit
- [ ] Log data to an SD card or the cloud for history/trends
- [ ] Add a battery level indicator
- [ ] Merge the camera and sensor dashboards into one page

---

## 📄 License

Add your preferred license (MIT, Apache 2.0, etc.) here before publishing.

## 🤝 Contributing

Pull requests are welcome — especially for improved wiring safety, better thresholds, or UI polish.
