/*
 * ==============================================================================
 * ESP32 Wi-Fi Robot Controller (Two-Channel Motor Driver)
 * ==============================================================================
 * Wi-Fi SSID (Username): Yoo
 * Wi-Fi Password:        8722519872
 * Web Controller IP:     http://192.168.4.1
 * 
 * Features:
 *   - Captive Portal: Automatically pops up or redirects ANY page to the controller
 *   - Explicit AP IP (192.168.4.1) configuration
 *   - Real-time command status & Serial monitor debug logs
 *   - Compatible with ESP32 Core 3.x and 2.x
 * ==============================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_arduino_version.h>

// ==============================================================================
// 1. PIN DEFINITIONS
// ==============================================================================
// Motor A (Left Tyre)
const int IN1 = 18;  // Left Forward
const int IN2 = 19;  // Left Backward
const int ENA = 21;  // Left PWM Speed

// Motor B (Right Tyre)
const int IN3 = 22;  // Right Forward
const int IN4 = 23;  // Right Backward
const int ENB = 25;  // Right PWM Speed

// PWM Configuration
const int PWM_FREQ       = 1000; // 1 kHz
const int PWM_RESOLUTION = 8;    // 8-bit (0 - 255)
const int PWM_CHAN_A     = 0;    // Core 2.x fallback
const int PWM_CHAN_B     = 1;    // Core 2.x fallback

int motorSpeed = 230; // Initial speed (0 - 255, 230 provides strong torque for 4 wheels)

// ==============================================================================
// 2. WI-FI & CAPTIVE PORTAL CONFIGURATION
// ==============================================================================
const char* AP_SSID = "Yoo";
const char* AP_PASS = "123456798";

const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);

// Default AP IP: 192.168.4.1
IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// ==============================================================================
// 3. MOTOR CONTROL FUNCTIONS
// ==============================================================================
void setMotorSpeed(int speedVal) {
  motorSpeed = constrain(speedVal, 0, 255);
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(ENA, motorSpeed);
  ledcWrite(ENB, motorSpeed);
#else
  ledcWrite(PWM_CHAN_A, motorSpeed);
  ledcWrite(PWM_CHAN_B, motorSpeed);
#endif
  Serial.print("[MOTOR] Speed set to: ");
  Serial.println(motorSpeed);
}

void stopBot() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  Serial.println("[CMD] STOP");
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("[CMD] FRONT (FORWARD)");
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("[CMD] BACK (BACKWARD)");
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("[CMD] LEFT");
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("[CMD] RIGHT");
}

// ==============================================================================
// 4. EMBEDDED CONTROLLER WEB PAGE
// ==============================================================================
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>ESP32 Robot Controller</title>
  <style>
    * {
      box-sizing: border-box;
      user-select: none;
      -webkit-user-select: none;
      margin: 0;
      padding: 0;
    }
    body {
      background: radial-gradient(circle at center, #1b263b 0%, #0d1b2a 100%);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      color: #e0e1dd;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      min-height: 100vh;
      padding: 16px;
    }
    .card {
      background: rgba(255, 255, 255, 0.05);
      backdrop-filter: blur(14px);
      -webkit-backdrop-filter: blur(14px);
      border: 1px solid rgba(255, 255, 255, 0.12);
      border-radius: 24px;
      padding: 24px;
      width: 100%;
      max-width: 380px;
      box-shadow: 0 20px 50px rgba(0, 0, 0, 0.6);
      text-align: center;
    }
    h1 {
      font-size: 1.4rem;
      font-weight: 700;
      letter-spacing: 1px;
      color: #00f0ff;
      text-shadow: 0 0 10px rgba(0, 240, 255, 0.4);
      margin-bottom: 6px;
    }
    .status-badge {
      display: inline-block;
      font-size: 0.78rem;
      background: rgba(0, 240, 255, 0.15);
      color: #70e000;
      padding: 4px 12px;
      border-radius: 12px;
      margin-bottom: 16px;
      font-weight: 600;
    }
    .state-box {
      background: #0f172a;
      border: 1px solid #334155;
      padding: 8px 12px;
      border-radius: 10px;
      font-size: 0.85rem;
      color: #00f0ff;
      margin-bottom: 20px;
      font-family: monospace;
    }
    .d-pad {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-template-rows: repeat(3, 1fr);
      gap: 12px;
      width: 240px;
      height: 240px;
      margin: 0 auto 20px auto;
    }
    .btn {
      background: linear-gradient(145deg, #1e293b, #0f172a);
      border: 1px solid rgba(255, 255, 255, 0.1);
      border-radius: 18px;
      color: #ffffff;
      font-size: 1.2rem;
      font-weight: bold;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      box-shadow: 0 6px 14px rgba(0,0,0,0.4);
      transition: all 0.1s ease;
      touch-action: manipulation;
    }
    .btn span {
      font-size: 0.7rem;
      font-weight: normal;
      color: #94a3b8;
      margin-top: 2px;
    }
    .btn:active, .btn.active {
      transform: scale(0.92);
      background: linear-gradient(145deg, #00f0ff, #0077b6);
      color: #000;
      box-shadow: 0 0 16px rgba(0, 240, 255, 0.8);
    }
    .btn:active span, .btn.active span {
      color: #000;
    }
    .btn-up    { grid-column: 2; grid-row: 1; }
    .btn-left  { grid-column: 1; grid-row: 2; }
    .btn-stop  { 
      grid-column: 2; 
      grid-row: 2; 
      background: linear-gradient(145deg, #d90429, #720026);
      font-size: 0.9rem;
    }
    .btn-stop:active {
      background: #ff0054 !important;
      box-shadow: 0 0 16px rgba(255, 0, 84, 0.8) !important;
    }
    .btn-right { grid-column: 3; grid-row: 2; }
    .btn-down  { grid-column: 2; grid-row: 3; }

    .slider-container {
      margin-top: 10px;
      text-align: left;
    }
    .slider-header {
      display: flex;
      justify-content: space-between;
      font-size: 0.85rem;
      color: #94a3b8;
      margin-bottom: 8px;
    }
    .slider {
      -webkit-appearance: none;
      width: 100%;
      height: 8px;
      border-radius: 4px;
      background: #334155;
      outline: none;
    }
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 22px;
      height: 22px;
      border-radius: 50%;
      background: #00f0ff;
      cursor: pointer;
      box-shadow: 0 0 8px #00f0ff;
    }
    .instructions {
      margin-top: 16px;
      font-size: 0.72rem;
      color: #64748b;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>ROBOT WEB CONTROLLER</h1>
    <div class="status-badge">● CONNECTED (192.168.4.1)</div>
    <div class="state-box" id="stateDisplay">STATE: READY</div>

    <div class="d-pad">
      <button class="btn btn-up" id="btn-fwd" data-cmd="forward">▲<span>FRONT</span></button>
      <button class="btn btn-left" id="btn-left" data-cmd="left">◀<span>LEFT</span></button>
      <button class="btn btn-stop" id="btn-stop" data-cmd="stop">STOP</button>
      <button class="btn btn-right" id="btn-right" data-cmd="right">▶<span>RIGHT</span></button>
      <button class="btn btn-down" id="btn-bwd" data-cmd="backward">▼<span>BACK</span></button>
    </div>

    <div class="slider-container">
      <div class="slider-header">
        <span>Motor Speed (PWM)</span>
        <span id="speedVal">230</span>
      </div>
      <input type="range" min="120" max="255" value="230" class="slider" id="speedRange">
    </div>

    <div class="instructions">
      Hold button to drive • Release to stop • W/A/S/D keys
    </div>
  </div>

  <script>
    let currentCmd = 'stop';
    const stateDisplay = document.getElementById('stateDisplay');

    function sendCommand(cmd) {
      if (cmd === currentCmd && cmd !== 'stop') return;
      currentCmd = cmd;
      stateDisplay.innerText = "STATE: " + cmd.toUpperCase();
      fetch('/cmd?dir=' + cmd).catch(err => {
        stateDisplay.innerText = "ERROR SENDING: " + cmd;
      });
    }

    const buttons = [
      { id: 'btn-fwd', cmd: 'forward' },
      { id: 'btn-bwd', cmd: 'backward' },
      { id: 'btn-left', cmd: 'left' },
      { id: 'btn-right', cmd: 'right' }
    ];

    buttons.forEach(item => {
      const el = document.getElementById(item.id);
      
      const startMove = (e) => {
        e.preventDefault();
        el.classList.add('active');
        sendCommand(item.cmd);
      };
      
      const endMove = (e) => {
        e.preventDefault();
        el.classList.remove('active');
        sendCommand('stop');
      };

      el.addEventListener('mousedown', startMove);
      el.addEventListener('mouseup', endMove);
      el.addEventListener('touchstart', startMove, { passive: false });
      el.addEventListener('touchend', endMove, { passive: false });
      el.addEventListener('mouseleave', () => {
        if (el.classList.contains('active')) endMove();
      });
    });

    document.getElementById('btn-stop').addEventListener('click', () => sendCommand('stop'));

    const speedRange = document.getElementById('speedRange');
    const speedVal = document.getElementById('speedVal');
    speedRange.addEventListener('input', (e) => {
      speedVal.innerText = e.target.value;
    });
    speedRange.addEventListener('change', (e) => {
      fetch('/speed?val=' + e.target.value).catch(err => console.error(err));
    });

    const keyMap = {
      'KeyW': 'forward', 'ArrowUp': 'forward',
      'KeyS': 'backward', 'ArrowDown': 'backward',
      'KeyA': 'left', 'ArrowLeft': 'left',
      'KeyD': 'right', 'ArrowRight': 'right',
      'Space': 'stop'
    };

    window.addEventListener('keydown', (e) => {
      if (keyMap[e.code]) {
        sendCommand(keyMap[e.code]);
      }
    });

    window.addEventListener('keyup', (e) => {
      if (keyMap[e.code] && e.code !== 'Space') {
        sendCommand('stop');
      }
    });
  </script>
</body>
</html>
)rawliteral";

// ==============================================================================
// 5. HTTP HELPERS & ROUTE HANDLERS
// ==============================================================================
void setCorsAndNoCache() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
}

void handleRoot() {
  setCorsAndNoCache();
  server.send(200, "text/html", HTML_PAGE);
}

void handleCommand() {
  setCorsAndNoCache();
  if (server.hasArg("dir")) {
    String dir = server.arg("dir");
    if (dir == "forward") {
      moveForward();
    } else if (dir == "backward") {
      moveBackward();
    } else if (dir == "left") {
      turnLeft();
    } else if (dir == "right") {
      turnRight();
    } else {
      stopBot();
    }
  } else {
    stopBot();
  }
  server.send(200, "text/plain", "OK");
}

void handleSpeed() {
  setCorsAndNoCache();
  if (server.hasArg("val")) {
    int speedVal = server.arg("val").toInt();
    setMotorSpeed(speedVal);
    server.send(200, "text/plain", "Speed updated");
  } else {
    server.send(400, "text/plain", "Missing val");
  }
}

// Redirect all unknown requests (Captive portal detection: Google, Apple, Microsoft)
void handleNotFound() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
}

// ==============================================================================
// 6. SETUP & MAIN LOOP
// ==============================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n==========================================");
  Serial.println("         ESP32 ROBOT STARTING             ");
  Serial.println("==========================================");

  // Motor direction pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Setup PWM channels
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(ENA, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(ENB, PWM_FREQ, PWM_RESOLUTION);
#else
  ledcSetup(PWM_CHAN_A, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHAN_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(ENA, PWM_CHAN_A);
  ledcAttachPin(ENB, PWM_CHAN_B);
#endif

  setMotorSpeed(motorSpeed);
  stopBot();

  // Configure Wi-Fi Access Point with explicit IP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(AP_SSID, AP_PASS);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("[WIFI] SSID:     ");
  Serial.println(AP_SSID);
  Serial.print("[WIFI] Password: ");
  Serial.println(AP_PASS);
  Serial.print("[WIFI] IP:       ");
  Serial.println(myIP);

  // Start Captive Portal DNS Server (routes any URL to 192.168.4.1)
  dnsServer.start(DNS_PORT, "*", local_ip);
  Serial.println("[DNS]  Captive Portal Active");

  // Setup HTTP routes
  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.on("/speed", handleSpeed);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("[HTTP] Server ready!");
  Serial.println("==========================================");
  Serial.println("Open your browser and navigate to:");
  Serial.print("  http://");
  Serial.println(myIP);
  Serial.println("==========================================\n");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
