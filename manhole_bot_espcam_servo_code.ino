#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_http_server.h"
#include <string.h>
#include <ESP32Servo.h>

// ================================================================
// Wi-Fi Configuration
// ================================================================
const char* ap_ssid     = "ESP32-CAM-PRO";
const char* ap_password = "12345678";
const char* http_username = "admin";
const char* http_password = "esp32cam";
const char* auth_token = "YWRtaW46ZXNwMzJjYW0="; // Base64: admin:esp32cam

// ================================================================
// Pin Configuration - AI-THINKER ESP32-CAM
// ================================================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define FLASH_LED_PIN      4
#define STATUS_LED_PIN    33
#define SERVO_PIN         13

// ================================================================
// Global Variables
// ================================================================
Servo camServo;
int currentServoAngle = 90;
bool flash_status = false;
bool streaming_active = false;
bool camera_initialized = false;
httpd_handle_t camera_httpd = NULL;

// ================================================================
// Authorization Check
// ================================================================
static bool is_authorized(httpd_req_t *req) {
  char auth_hdr[128];
  if (httpd_req_get_hdr_value_str(req, "Authorization", auth_hdr, sizeof(auth_hdr)) == ESP_OK) {
    const char *prefix = "Basic ";
    if (strncmp(auth_hdr, prefix, 6) == 0) {
      const char *b64 = auth_hdr + 6;
      if (strcmp(b64, auth_token) == 0) {
        return true;
      }
    }
  }
  return false;
}

// ================================================================
// Professional HTML UI with Servo Control Sidebar
// ================================================================
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>ESP32-CAM Pro Control</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #f8f9fa 0%, #ffffff 100%);
      color: #1a1a1a;
      min-height: 100vh;
    }
    .container {
      display: flex;
      flex-direction: row;
      height: 100vh;
      overflow: hidden;
    }
    /* ===== MAIN CONTENT ===== */
    .main-content {
      flex: 1;
      display: flex;
      flex-direction: column;
      padding: 12px;
      gap: 8px;
      overflow-y: auto;
      background: #f5f5f5;
    }
    /* ===== HEADER ===== */
    .header {
      background: linear-gradient(135deg, #2563eb 0%, #1e40af 100%);
      padding: 14px 18px;
      border-radius: 10px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      box-shadow: 0 4px 12px rgba(37, 99, 235, 0.25);
    }
    .header-title {
      font-size: 1.3rem;
      font-weight: 700;
      color: #fff;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .status-indicator {
      width: 12px;
      height: 12px;
      border-radius: 50%;
      background: #10b981;
      animation: pulse 2s infinite;
      box-shadow: 0 0 8px #10b981;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.5; }
    }
    /* ===== VIDEO STREAM ===== */
    .video-container {
      background: #000;
      border-radius: 12px;
      overflow: hidden;
      display: flex;
      justify-content: center;
      align-items: center;
      flex: 1;
      min-height: 65vh;
      border: 3px solid #2563eb;
      box-shadow: 0 8px 24px rgba(37, 99, 235, 0.2);
    }
    #stream {
      width: 100%;
      height: 100%;
      object-fit: contain;
      background: #000;
    }
    .loading {
      color: #10b981;
      font-weight: 600;
      text-align: center;
    }
    /* ===== CONTROL PANEL ===== */
    .control-panel {
      background: #ffffff;
      border-radius: 10px;
      padding: 12px;
      border: 1px solid #e0e0e0;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
      max-height: auto;
      overflow-y: auto;
    }
    .panel-title {
      font-size: 0.75rem;
      font-weight: 700;
      color: #2563eb;
      text-transform: uppercase;
      letter-spacing: 0.08em;
      margin-bottom: 8px;
      display: block;
    }
    .controls-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(110px, 1fr));
      gap: 8px;
    }
    .btn {
      background: #f0f0f0;
      color: #1a1a1a;
      border: 2px solid #d0d0d0;
      padding: 11px;
      font-size: 0.85rem;
      font-weight: 600;
      border-radius: 8px;
      cursor: pointer;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 4px;
      transition: all 0.2s ease;
      user-select: none;
    }
    .btn:hover {
      background: #e0e0e0;
      border-color: #2563eb;
      transform: translateY(-2px);
      box-shadow: 0 4px 8px rgba(37, 99, 235, 0.15);
    }
    .btn:active {
      transform: scale(0.98);
    }
    .btn.active {
      background: #2563eb;
      border-color: #1e40af;
      color: #fff;
      font-weight: 700;
      box-shadow: 0 4px 12px rgba(37, 99, 235, 0.35);
    }
    #flashBtn.active {
      background: #fbbf24;
      border-color: #f59e0b;
      color: #000;
      box-shadow: 0 0 16px rgba(251, 191, 36, 0.6);
    }
    #flashBtn.active:hover {
      background: #f59e0b;
      box-shadow: 0 0 20px rgba(251, 191, 36, 0.8);
    }
    .btn.danger {
      background: #ffebee;
      border-color: #dc2626;
      color: #dc2626;
    }
    .btn.danger:hover {
      background: #dc2626;
      color: #fff;
      border-color: #b91c1c;
    }
    .btn.danger.active {
      background: #dc2626;
      border-color: #991b1b;
      color: #fff;
      animation: pulse-danger 1.5s infinite;
    }
    @keyframes pulse-danger {
      0%, 100% { box-shadow: 0 0 12px rgba(220, 38, 38, 0.6); }
      50% { box-shadow: 0 0 20px rgba(220, 38, 38, 0.2); }
    }
    .btn.success {
      background: #e8f5e9;
      border-color: #10b981;
      color: #059669;
    }
    .btn.success:hover {
      background: #10b981;
      border-color: #059669;
      color: #fff;
    }
    /* ===== SETTINGS ROW ===== */
    .settings-row {
      display: grid;
      grid-template-columns: 1fr 1fr 1fr;
      gap: 10px;
    }
    .setting-group {
      display: flex;
      flex-direction: column;
      gap: 6px;
    }
    .setting-label {
      font-size: 0.75rem;
      font-weight: 600;
      color: #2563eb;
      text-transform: uppercase;
    }
    select, input[type="range"] {
      background: #ffffff;
      color: #1a1a1a;
      border: 2px solid #d0d0d0;
      padding: 8px;
      border-radius: 6px;
      font-size: 0.85rem;
      font-weight: 600;
      transition: border-color 0.2s;
    }
    select:hover, input[type="range"]:hover {
      border-color: #2563eb;
    }
    select:focus, input[type="range"]:focus {
      border-color: #2563eb;
      outline: none;
    }
    input[type="range"] {
      cursor: pointer;
      height: 6px;
    }
    /* ===== SERVO SIDEBAR ===== */
    .servo-sidebar {
      width: 130px;
      background: #ffffff;
      border-left: 3px solid #2563eb;
      padding: 15px 10px;
      display: flex;
      flex-direction: column;
      gap: 12px;
      align-items: center;
      overflow-y: auto;
      box-shadow: -2px 0 8px rgba(37, 99, 235, 0.12);
      border-top: 1px solid #e0e0e0;
      border-bottom: 1px solid #e0e0e0;
    }
    .servo-title {
      font-size: 0.7rem;
      font-weight: 800;
      color: #2563eb;
      text-transform: uppercase;
      text-align: center;
      width: 100%;
      border-bottom: 2px solid #2563eb;
      padding-bottom: 8px;
      letter-spacing: 0.05em;
    }
    .servo-controls {
      display: flex;
      flex-direction: column;
      gap: 8px;
      width: 100%;
      align-items: center;
    }
    .servo-btn {
      width: 100%;
      padding: 12px;
      background: #f0f0f0;
      color: #1a1a1a;
      border: 2px solid #2563eb;
      border-radius: 8px;
      cursor: pointer;
      font-size: 1.3rem;
      font-weight: 700;
      transition: all 0.2s ease;
    }
    .servo-btn:hover {
      background: #2563eb;
      border-color: #1e40af;
      color: #fff;
      transform: scale(1.05);
      box-shadow: 0 4px 8px rgba(37, 99, 235, 0.25);
    }
    .servo-btn:active {
      transform: scale(0.95);
    }
    #servoCenter {
      background: #2563eb;
      color: #fff;
      border-color: #1e40af;
    }
    #servoCenter:hover {
      background: #1e40af;
    }
    .angle-display {
      font-size: 0.9rem;
      font-weight: 800;
      color: #2563eb;
      text-align: center;
      background: #e3f2fd;
      padding: 10px;
      border-radius: 8px;
      width: 100%;
      border: 2px solid #2563eb;
    }
    .servo-slider {
      width: 100%;
      writing-mode: bt-lr;
      -webkit-appearance: slider-vertical;
      appearance: slider-vertical;
      height: 200px;
      background: #f0f0f0;
      border-radius: 6px;
    }
    /* ===== RESPONSIVE ===== */
    @media (max-width: 1024px) {
      .container {
        flex-direction: column;
      }
      .servo-sidebar {
        width: 100%;
        height: auto;
        flex-direction: row;
        border-left: none;
        border-top: 3px solid #2563eb;
        border-bottom: 1px solid #e0e0e0;
        padding: 10px 15px;
        gap: 8px;
      }
      .servo-title {
        border-bottom: none;
        border-right: 2px solid #2563eb;
        padding-bottom: 0;
        padding-right: 8px;
        width: auto;
        min-width: 80px;
      }
      .servo-controls {
        flex-direction: row;
        gap: 6px;
      }
      .servo-slider {
        writing-mode: lr-tb;
        -webkit-appearance: slider-horizontal;
        appearance: slider-horizontal;
        height: auto;
        width: 150px;
      }
      .controls-grid {
        grid-template-columns: repeat(auto-fit, minmax(90px, 1fr));
      }
      .video-container {
        min-height: 400px;
      }
      .settings-row {
        grid-template-columns: 1fr 1fr;
      }
    }
    @media (max-width: 768px) {
      .main-content {
        padding: 8px;
        gap: 6px;
      }
      .header {
        padding: 10px;
      }
      .header-title {
        font-size: 1rem;
      }
      .btn {
        padding: 9px;
        font-size: 0.75rem;
      }
      .settings-row {
        grid-template-columns: 1fr;
      }
      .controls-grid {
        grid-template-columns: repeat(auto-fit, minmax(75px, 1fr));
      }
      .video-container {
        min-height: 300px;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <!-- ===== MAIN CONTENT ===== -->
    <div class="main-content">
      <!-- Header -->
      <div class="header">
        <div class="header-title">
          <span class="status-indicator"></span>
          ESP32-CAM Pro
        </div>
        <div style="color: #fff; font-size: 0.85rem; font-weight: 600;">Commercial Live Control</div>
      </div>

      <!-- Video Stream -->
      <div class="video-container">
        <img id="stream" src="/stream" alt="Camera Stream" onload="console.log('Stream loaded')" onerror="console.log('Stream error')" />
      </div>

      <!-- Control Panel -->
      <div class="control-panel">
        <!-- Video Controls -->
        <span class="panel-title">🎥 Video Controls</span>
        <div class="controls-grid">
          <button class="btn success" id="btnStart" onclick="startStream()" title="Start Video Stream">
            ▶ Start
          </button>
          <button class="btn danger" id="btnStop" onclick="stopStream()" title="Stop Video Stream">
            ⏹ Stop
          </button>
          <button class="btn" id="btnSnap" onclick="capturePhoto()" title="Capture Photo">
            📷 Snap
          </button>
        </div>

        <!-- Camera Settings -->
        <span class="panel-title" style="margin-top: 12px;">⚙️ Camera Settings</span>
        <div class="settings-row">
          <div class="setting-group">
            <label class="setting-label">Brightness</label>
            <input type="range" id="brightness" min="-2" max="2" value="0" step="1" onchange="setBrightness(this.value)" oninput="console.log('Brightness:', this.value)">
          </div>
          <div class="setting-group">
            <label class="setting-label">Contrast</label>
            <input type="range" id="contrast" min="-2" max="2" value="0" step="1" onchange="setContrast(this.value)" oninput="console.log('Contrast:', this.value)">
          </div>
          <div class="setting-group">
            <label class="setting-label">Resolution</label>
            <select id="resSelect" onchange="setFramesize(this.value)">
              <option value="7">SVGA (800x600)</option>
              <option value="6">VGA (640x480)</option>
              <option value="5">QVGA (320x240)</option>
              <option value="8">UXGA (1600x1200)</option>
            </select>
          </div>
        </div>

        <div class="settings-row">
          <div class="setting-group">
            <label class="setting-label">Effects</label>
            <select id="effectSelect" onchange="setEffect(this.value)">
              <option value="0">Normal</option>
              <option value="1">Negative</option>
              <option value="2">Grayscale</option>
              <option value="3">Red Tint</option>
              <option value="4">Green Tint</option>
              <option value="5">Blue Tint</option>
            </select>
          </div>
          <div class="setting-group">
            <label class="setting-label">Saturation</label>
            <input type="range" id="saturation" min="-2" max="2" value="0" step="1" onchange="setSaturation(this.value)">
          </div>
          <div class="setting-group">
            <label class="setting-label">Quality</label>
            <input type="range" id="quality" min="10" max="63" value="30" step="1" onchange="setQuality(this.value)">
          </div>
        </div>

        <!-- Flash & Flip Controls -->
        <span class="panel-title" style="margin-top: 12px;">💡 Flash & Orientation</span>
        <div class="controls-grid">
          <button class="btn" id="flashBtn" type="button" onclick="toggleFlash()" title="Toggle Flash LED" style="cursor: pointer;">
            💡 Flash
          </button>
          <button class="btn" id="vFlipBtn" onclick="toggleVFlip()" title="Vertical Flip">
            ↕ V-Flip
          </button>
          <button class="btn" id="hMirrorBtn" onclick="toggleHMirror()" title="Horizontal Mirror">
            ↔ H-Mirror
          </button>
        </div>
      </div>
    </div>

    <!-- ===== SERVO SIDEBAR ===== -->
    <div class="servo-sidebar">
      <div class="servo-title">🎯 SERVO PAN</div>
      <div class="servo-controls">
        <button class="servo-btn" id="servoPanLeft" onclick="servoMove(-10)" title="Pan Left">◀</button>
        <button class="servo-btn" id="servoCenter" onclick="servoCenter()" title="Center">●</button>
        <button class="servo-btn" id="servoPanRight" onclick="servoMove(10)" title="Pan Right">▶</button>
      </div>
      <div class="angle-display" id="angleDisplay">90°</div>
      <input type="range" class="servo-slider" id="servoSlider" min="0" max="180" value="90" 
             onchange="servoSetAngle(this.value)" oninput="updateAngleDisplay(this.value)" title="Servo Angle Control">
    </div>
  </div>

  <script>
    let currentAngle = 90;
    let isStreaming = false;
    let flashState = false;
    let vFlipState = false;
    let hMirrorState = false;

    console.log('Page loaded - ESP32-CAM Control Panel Ready');

    // ===== ANGLE DISPLAY UPDATE =====
    function updateAngleDisplay(angle) {
      document.getElementById('angleDisplay').textContent = parseInt(angle) + '°';
    }

    // ===== STREAM FUNCTIONS =====
    function startStream() {
      console.log('Starting stream...');
      document.getElementById('stream').src = '/stream?t=' + Date.now();
      isStreaming = true;
      document.getElementById('btnStart').classList.add('active');
      document.getElementById('btnStop').classList.remove('active');
      console.log('Stream started');
    }

    function stopStream() {
      console.log('Stopping stream...');
      document.getElementById('stream').src = '';
      isStreaming = false;
      document.getElementById('btnStart').classList.remove('active');
      document.getElementById('btnStop').classList.add('active');
      console.log('Stream stopped');
    }

    function capturePhoto() {
      console.log('Capturing photo...');
      const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
      const img = new Image();
      img.onload = function() {
        const link = document.createElement('a');
        link.href = '/capture?t=' + Date.now();
        link.download = 'photo_' + timestamp + '.jpg';
        link.click();
        console.log('Photo downloaded');
      };
      img.src = '/capture?t=' + Date.now();
    }

    // ===== CAMERA SETTINGS =====
    function setBrightness(val) {
      console.log('Setting brightness:', val);
      fetch('/control?var=brightness&val=' + val)
        .then(r => console.log('Brightness set:', r.status))
        .catch(e => console.error('Brightness error:', e));
    }

    function setContrast(val) {
      console.log('Setting contrast:', val);
      fetch('/control?var=contrast&val=' + val)
        .then(r => console.log('Contrast set:', r.status))
        .catch(e => console.error('Contrast error:', e));
    }

    function setSaturation(val) {
      console.log('Setting saturation:', val);
      fetch('/control?var=saturation&val=' + val)
        .then(r => console.log('Saturation set:', r.status))
        .catch(e => console.error('Saturation error:', e));
    }

    function setQuality(val) {
      console.log('Setting quality:', val);
      fetch('/control?var=quality&val=' + val)
        .then(r => console.log('Quality set:', r.status))
        .catch(e => console.error('Quality error:', e));
    }

    function setFramesize(val) {
      console.log('Setting framesize:', val);
      fetch('/control?var=framesize&val=' + val)
        .then(r => console.log('Resolution set:', r.status))
        .catch(e => console.error('Resolution error:', e));
    }

    function setEffect(val) {
      console.log('Setting effect:', val);
      fetch('/control?var=special_effect&val=' + val)
        .then(r => console.log('Effect set:', r.status))
        .catch(e => console.error('Effect error:', e));
    }

    function toggleVFlip() {
      vFlipState = !vFlipState;
      console.log('Toggling V-Flip:', vFlipState);
      const btn = document.getElementById('vFlipBtn');
      btn.classList.toggle('active');
      fetch('/control?var=vflip&val=' + (vFlipState ? 1 : 0))
        .then(r => console.log('V-Flip toggled:', r.status))
        .catch(e => console.error('V-Flip error:', e));
    }

    function toggleHMirror() {
      hMirrorState = !hMirrorState;
      console.log('Toggling H-Mirror:', hMirrorState);
      const btn = document.getElementById('hMirrorBtn');
      btn.classList.toggle('active');
      fetch('/control?var=hmirror&val=' + (hMirrorState ? 1 : 0))
        .then(r => console.log('H-Mirror toggled:', r.status))
        .catch(e => console.error('H-Mirror error:', e));
    }

    // ===== FLASH CONTROL =====
    function toggleFlash() {
      const btn = document.getElementById('flashBtn');
      console.log('Flash button clicked - Current state:', flashState);
      
      // Send request to toggle flash
      fetch('/toggle_flash', {method: 'GET'})
        .then(response => {
          console.log('Flash response status:', response.status);
          if (!response.ok) {
            throw new Error('Flash request failed: ' + response.status);
          }
          return response.text();
        })
        .then(data => {
          console.log('Flash response data:', data);
          // Toggle state after successful response
          flashState = !flashState;
          console.log('Flash toggled to:', flashState ? 'ON' : 'OFF');
          
          // Update button visual state
          if (flashState) {
            btn.classList.add('active');
            console.log('Button marked as ACTIVE (BLUE)');
          } else {
            btn.classList.remove('active');
            console.log('Button marked as INACTIVE (GRAY)');
          }
        })
        .catch(error => {
          console.error('❌ Flash toggle error:', error);
          alert('Flash control failed: ' + error.message);
        });
    }

    // ===== SERVO CONTROL =====
    function servoSetAngle(angle) {
      currentAngle = parseInt(angle);
      updateAngleDisplay(currentAngle);
      console.log('Setting servo angle:', currentAngle);
      fetch('/servo?angle=' + currentAngle)
        .then(r => console.log('Servo set:', r.status))
        .catch(e => console.error('Servo error:', e));
    }

    function servoMove(increment) {
      let newAngle = currentAngle + increment;
      if (newAngle < 0) newAngle = 0;
      if (newAngle > 180) newAngle = 180;
      console.log('Moving servo:', increment, 'to', newAngle);
      document.getElementById('servoSlider').value = newAngle;
      updateAngleDisplay(newAngle);
      servoSetAngle(newAngle);
    }

    function servoCenter() {
      console.log('Centering servo');
      document.getElementById('servoSlider').value = 90;
      updateAngleDisplay(90);
      servoSetAngle(90);
    }

    // Auto-start stream on page load
    window.addEventListener('load', function() {
      console.log('Window loaded - starting stream');
      startStream();
    });
  </script>
</body>
</html>
)rawliteral";

// ================================================================
// HTTP Handlers
// ================================================================

// Index (Main Page)
static esp_err_t index_handler(httpd_req_t *req) {
  if (!is_authorized(req)) {
    httpd_resp_set_status(req, "401 Unauthorized");
    httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"ESP32-CAM\"");
    return httpd_resp_send(req, NULL, 0);
  }
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

// Flash Control Handler
static esp_err_t flash_handler(httpd_req_t *req) {
  if (!is_authorized(req)) {
    httpd_resp_set_status(req, "401 Unauthorized");
    httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"ESP32-CAM\"");
    return httpd_resp_send(req, NULL, 0);
  }
  
  flash_status = !flash_status;
  digitalWrite(FLASH_LED_PIN, flash_status ? HIGH : LOW);
  
  Serial.printf("[FLASH] Status: %s (GPIO4: %d)\n", flash_status ? "ON" : "OFF", flash_status ? 1 : 0);
  
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  
  const char* response = flash_status ? "FLASH_ON" : "FLASH_OFF";
  return httpd_resp_send(req, response, strlen(response));
}

// Capture Photo Handler
static esp_err_t capture_handler(httpd_req_t *req) {
  if (!is_authorized(req)) {
    httpd_resp_set_status(req, "401 Unauthorized");
    return httpd_resp_send(req, NULL, 0);
  }
  if (!camera_initialized) return ESP_FAIL;
  
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return ESP_FAIL;
  
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "attachment; filename=photo.jpg");
  esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  
  return res;
}

// Servo Control Handler
static esp_err_t servo_handler(httpd_req_t *req) {
  if (!is_authorized(req)) {
    httpd_resp_set_status(req, "401 Unauthorized");
    return httpd_resp_send(req, NULL, 0);
  }
  
  char buf[32];
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
    char val[8];
    if (httpd_query_key_value(buf, "angle", val, sizeof(val)) == ESP_OK) {
      int angle = atoi(val);
      if (angle < 0) angle = 0;
      if (angle > 180) angle = 180;
      camServo.write(angle);
      currentServoAngle = angle;
    }
  }
  
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, "OK", 2);
}

// Camera Control Handler
static esp_err_t cmd_handler(httpd_req_t *req) {
  if (!is_authorized(req)) {
    httpd_resp_set_status(req, "401 Unauthorized");
    return httpd_resp_send(req, NULL, 0);
  }
  if (!camera_initialized) return ESP_FAIL;
  
  char buf[64];
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
    char variable[32];
    char value[32];
    if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
        httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
      
      sensor_t *s = esp_camera_sensor_get();
      int val = atoi(value);
      
      if (!strcmp(variable, "framesize")) s->set_framesize(s, (framesize_t)val);
      else if (!strcmp(variable, "brightness")) s->set_brightness(s, val);
      else if (!strcmp(variable, "contrast")) s->set_contrast(s, val);
      else if (!strcmp(variable, "vflip")) s->set_vflip(s, val);
      else if (!strcmp(variable, "hmirror")) s->set_hmirror(s, val);
      else if (!strcmp(variable, "special_effect")) s->set_special_effect(s, val);
    }
  }
  
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, "OK", 2);
}

// Video Stream Handler
static esp_err_t stream_handler(httpd_req_t *req) {
  if (!is_authorized(req)) {
    httpd_resp_set_status(req, "401 Unauthorized");
    return httpd_resp_send(req, NULL, 0);
  }
  if (!camera_initialized) return ESP_FAIL;
  
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  char part_buf[128];

  res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
  if (res != ESP_OK) return res;

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "X-Framerate", "30");

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      res = ESP_FAIL;
      break;
    }

    size_t hlen = snprintf((char *)part_buf, sizeof(part_buf),
      "\r\n--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
    
    res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    }

    esp_camera_fb_return(fb);
    fb = NULL;

    if (res != ESP_OK) break;
    taskYIELD();
  }
  
  return res;
}

// Start Server
void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.max_open_sockets = 7;
  config.lru_purge_enable = true;

  httpd_uri_t index_uri   = {.uri = "/",             .method = HTTP_GET, .handler = index_handler,   .user_ctx = NULL};
  httpd_uri_t stream_uri  = {.uri = "/stream",       .method = HTTP_GET, .handler = stream_handler,  .user_ctx = NULL};
  httpd_uri_t capture_uri = {.uri = "/capture",      .method = HTTP_GET, .handler = capture_handler, .user_ctx = NULL};
  httpd_uri_t flash_uri   = {.uri = "/toggle_flash", .method = HTTP_GET, .handler = flash_handler,   .user_ctx = NULL};
  httpd_uri_t cmd_uri     = {.uri = "/control",      .method = HTTP_GET, .handler = cmd_handler,     .user_ctx = NULL};
  httpd_uri_t servo_uri   = {.uri = "/servo",        .method = HTTP_GET, .handler = servo_handler,   .user_ctx = NULL};

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &stream_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &flash_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
    httpd_register_uri_handler(camera_httpd, &servo_uri);
  }
}

// ================================================================
// Setup
// ================================================================
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(50);
  Serial.println("\n\n=== ESP32-CAM PRO - COMMERCIAL VERSION ===\n");

  // GPIO Setup
  pinMode(FLASH_LED_PIN, OUTPUT);        // GPIO 4 - Flash LED
  pinMode(STATUS_LED_PIN, OUTPUT);       // GPIO 33 - Status LED
  
  // Initial state - both LOW (OFF)
  digitalWrite(FLASH_LED_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  // Verify GPIO state
  Serial.printf("[GPIO] FLASH_LED_PIN (GPIO 4) set to OUTPUT\n");
  Serial.printf("[GPIO] STATUS_LED_PIN (GPIO 33) set to OUTPUT\n");
  Serial.printf("[GPIO] Both LEDs initialized to OFF\n");

  // Wi-Fi Setup - AP Mode
  WiFi.persistent(false);
  WiFi.disconnect(true);
  delay(50);
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.softAP(ap_ssid, ap_password, 1, 0, 4);
  IPAddress apIP = WiFi.softAPIP();

  // Camera Configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;  // 800x600 - Commercial quality
  config.jpeg_quality = 18;

  if (psramFound()) {
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    camera_initialized = false;
  } else {
    Serial.println("[OK] Camera initialized");
    camera_initialized = true;
    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL) {
      s->set_vflip(s, 1);
      s->set_brightness(s, 0);
    }
  }

  // Servo Setup
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  camServo.setPeriodHertz(50);
  camServo.attach(SERVO_PIN, 500, 2400);
  camServo.write(90);
  Serial.println("[OK] Servo initialized");

  // Start Server
  startCameraServer();

  // Status Output
  Serial.println("\n================================================");
  Serial.println("✓ ESP32-CAM PRO READY FOR COMMERCIAL USE");
  Serial.println("================================================");
  Serial.printf("📶 Network:     %s\n", ap_ssid);
  Serial.printf("🔑 Password:    %s\n", ap_password);
  Serial.printf("👤 Username:    %s\n", http_username);
  Serial.printf("🔐 Password:    %s\n", http_password);
  Serial.printf("🌐 Open:        http://%s\n", apIP.toString().c_str());
  Serial.println("================================================\n");
}

// ================================================================
// Loop
// ================================================================
void loop() {
  digitalWrite(STATUS_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(STATUS_LED_PIN, LOW);
  delay(1000);
}