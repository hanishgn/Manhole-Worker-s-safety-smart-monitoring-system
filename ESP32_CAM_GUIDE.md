# ESP32-CAM Pro Commercial - Complete Setup Guide

## 🎯 Key Features

### 1. **Professional Web Interface**
- Dark theme commercial design
- Responsive layout (desktop, tablet, mobile)
- Live video stream display
- Real-time status indicators

### 2. **Easy-to-Access Servo Control (Sidebar)**
- ◀ Pan Left (±10°)
- ● Center (90°)
- ▶ Pan Right (±10°)
- Angle slider (0-180°)
- Digital angle display

### 3. **Video Controls**
- ▶ Start Stream
- ⏹ Stop Stream
- 📷 Snapshot (Download photo)

### 4. **Camera Settings**
- Brightness control (±2)
- Contrast control (±2)
- Resolution selection (SVGA/VGA/QVGA/UXGA)
- Special effects (Normal, Negative, Grayscale, Tint effects)

### 5. **Flash & Orientation**
- 💡 Flash on/off (Toggle)
- ↕ Vertical flip
- ↔ Horizontal mirror

### 6. **Security**
- Basic HTTP authentication
- Admin username/password
- Base64 encoded credentials

---

## 📋 Hardware Requirements

```
- ESP32-CAM (AI-Thinker)
- SG90 Servo motor (Pan control)
- Micro USB cable (power)
- Flash LED (GPIO 4) - already onboard
```

### Pin Configuration
```
SERVO_PIN       = GPIO 13
FLASH_LED_PIN   = GPIO 4
STATUS_LED_PIN  = GPIO 33
```

---

## 🔧 Installation Steps

### Step 1: Install Libraries
In Arduino IDE → Sketch → Include Library → Manage Libraries

Search and install:
- **ESP32** (by Espressif Systems)
- **ESP32Servo** (by Kevin Harrington)

### Step 2: Upload Code
1. Select Board: `AI Thinker ESP32-CAM`
2. Select Port: `COM X` (your ESP32 port)
3. Flash Mode: `DIO`
4. Copy the code from `esp_cam_commercial.ino`
5. Click Upload

### Step 3: Connect to Network
After upload, open Serial Monitor (115200 baud)

You'll see:
```
================================================
✓ ESP32-CAM PRO READY FOR COMMERCIAL USE
================================================
📶 Network:     ESP32-CAM-PRO
🔑 Password:    12345678
👤 Username:    admin
🔐 Password:    esp32cam
🌐 Open:        http://192.168.4.1
================================================
```

### Step 4: Access Web Interface
1. Connect your device to WiFi: **ESP32-CAM-PRO**
2. Password: **12345678**
3. Open browser: `http://192.168.4.1`
4. Login:
   - Username: `admin`
   - Password: `esp32cam`

---

## 🎮 Web Interface Layout

### Main Section (Left)
- **Header**: Status indicator + title
- **Video Stream**: Live camera feed (centered)
- **Control Panel**: 
  - Video controls (Start/Stop/Snap)
  - Camera settings (Brightness, Contrast, etc.)
  - Flash & Orientation controls

### Servo Sidebar (Right)
- **Direction**: Pan Left, Center, Pan Right buttons
- **Angle Slider**: 0-180° vertical slider
- **Angle Display**: Current angle in degrees

*On mobile: Servo sidebar converts to horizontal bar at bottom*

---

## 📱 Responsive Design

### Desktop (>1024px)
- Sidebar on right side (vertical layout)
- Full-size video stream
- All controls visible

### Tablet (768px - 1024px)
- Servo controls move to top (horizontal)
- Adjusted button sizes
- Grid layout optimization

### Mobile (<768px)
- Stacked layout
- Single column buttons
- Touch-optimized controls
- Smaller video frame

---

## 🔐 Security Credentials

Default Login:
- **Username**: `admin`
- **Password**: `esp32cam`

To change credentials:
Edit these lines in the code:
```cpp
const char* http_username = "admin";      // Change username
const char* http_password = "esp32cam";   // Change password
const char* auth_token = "YWRtaW46ZXNwMzJjYW0=";  // Update Base64
```

To generate new Base64:
1. Combine: `username:password`
2. Convert to Base64
3. Replace `auth_token`

---

## 🎛️ Camera Settings Guide

### Brightness
- **-2**: Very dark
- **-1**: Dark
- **0**: Normal (default)
- **+1**: Bright
- **+2**: Very bright

### Contrast
- **-2**: Low contrast
- **-1**: Slightly low
- **0**: Normal (default)
- **+1**: Slightly high
- **+2**: High contrast

### Resolution
- **SVGA (800x600)**: Commercial quality - default
- **VGA (640x480)**: Good balance
- **QVGA (320x240)**: Low bandwidth
- **UXGA (1600x1200)**: High quality

### Special Effects
- **Normal**: No effect
- **Negative**: Inverted colors
- **Grayscale**: Black & white
- **Red/Green/Blue Tint**: Color tint applied

---

## 🤖 Servo Control Details

### Pan Motion
- **Range**: 0° to 180°
- **Default**: 90° (center)
- **Speed**: ~60°/s (typical SG90)

### Control Methods
1. **Direction Buttons**: ±10° increments
2. **Center Button**: Jump to 90°
3. **Slider**: Smooth continuous control (0-180°)

### Mechanical Setup
```
Servo Pin: GPIO 13
PWM: 50 Hz
Pulse Range: 500-2400 µs
```

---

## ✅ Testing Checklist

- [ ] ESP32-CAM connects to AP
- [ ] Web interface loads (http://192.168.4.1)
- [ ] Login with admin/esp32cam works
- [ ] Video stream starts and displays
- [ ] Stream stops when clicked "Stop"
- [ ] Snapshot/photo downloads
- [ ] Flash LED turns on/off
- [ ] Brightness slider works
- [ ] Contrast slider works
- [ ] Resolution changes update video
- [ ] Effects change video appearance
- [ ] V-Flip button flips video vertically
- [ ] H-Mirror button mirrors video horizontally
- [ ] Servo moves with Pan Left button
- [ ] Servo moves with Pan Right button
- [ ] Servo centers with Center button
- [ ] Angle slider controls servo smoothly
- [ ] Angle display updates in real-time

---

## 🐛 Troubleshooting

### Problem: "401 Unauthorized"
**Solution**: Check username/password. Must match Base64 auth_token.

### Problem: Camera not initializing
**Solution**: 
- Check USB power (needs 2A+)
- Verify pins are correct
- Try different USB cable

### Problem: Servo not moving
**Solution**:
- Check GPIO 13 is free
- Verify servo power (separate 5V supply recommended)
- Test with Serial output

### Problem: Video streaming slow
**Solution**:
- Reduce JPEG quality
- Lower resolution
- Check WiFi signal strength
- Disable effects

### Problem: Flash not working
**Solution**:
- Verify GPIO 4 is not used by other devices
- Check LED polarity
- Try higher brightness setting

### Problem: Low WiFi range
**Solution**:
- Move closer to device
- Reduce obstacles
- Check antenna connection
- Verify WiFi power setting

---

## 📊 Performance Metrics

| Metric | Value |
|--------|-------|
| Video FPS | ~30 |
| Stream Latency | <200ms |
| Resolution | Up to 1600x1200 |
| WiFi Mode | AP Mode (no channel hopping) |
| Servo Speed | ~60°/s |
| Power Draw | ~500mA (with flash) |

---

## 🔄 API Endpoints

All endpoints require Basic Auth.

### Video Stream
```
GET /stream
```
Returns MJPEG stream

### Capture Photo
```
GET /capture
```
Returns single JPEG image

### Toggle Flash
```
GET /toggle_flash
```
Toggles LED on/off

### Servo Control
```
GET /servo?angle=<0-180>
```
Sets servo angle

### Camera Control
```
GET /control?var=<variable>&val=<value>
```
Variables: `framesize`, `brightness`, `contrast`, `vflip`, `hmirror`, `special_effect`

---

## 💡 Commercial Use Tips

1. **Reliability**: Use industrial-grade power supply (2A+ @ 5V)
2. **Cooling**: Add heatsink if device runs hot
3. **Servo**: Use quality servo for precise control
4. **Network**: Keep WiFi secure, change default password
5. **Updates**: Test code changes on test unit first
6. **Backup**: Save working code configurations
7. **Monitoring**: Log important events via Serial

---

## 📞 Support Information

Default Credentials (Change These!):
- **SSID**: ESP32-CAM-PRO
- **Password**: 12345678
- **Username**: admin
- **Password**: esp32cam
- **Access**: http://192.168.4.1

---

Version: 1.0 (Commercial Release)
Last Updated: 2024
