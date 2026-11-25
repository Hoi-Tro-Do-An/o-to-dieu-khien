#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ================== CAU HINH WIFI ==================
// Sua 2 dong duoi thanh ten/mat khau WiFi nha ban.
// Neu de nguyen "TEN_WIFI_CUA_BAN", xe se tu phat WiFi rieng: RobotCar / 12345678
const char *WIFI_SSID = "TEN_WIFI_CUA_BAN";
const char *WIFI_PASS = "MAT_KHAU_WIFI";

const char *AP_SSID = "RobotCar";
const char *AP_PASS = "12345678";
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
const char *MDNS_NAME = "robotcar";

// ================== CHAN ESP32 -> L298N ==================
// Chi dung 4 chan: moi dong co 2 chan IN (vua dieu khien chieu, vua PWM toc do).
// => Phai GIU NGUYEN jumper 5V-EN tren L298N (ENA/ENB luon bat).
//
//   Motor trai : IN1 = GPIO25, IN2 = GPIO26
//   Motor phai : IN3 = GPIO32, IN4 = GPIO33
//   GND ESP32  --- GND L298N  (bat buoc noi chung)
const int IN1_PIN = 25;
const int IN2_PIN = 26;
const int IN3_PIN = 32;
const int IN4_PIN = 33;

// Moi chan can mot KENH PWM rieng. ledcWrite() nhan KENH, khong phai so chan.
const int CH_IN1 = 0;
const int CH_IN2 = 1;
const int CH_IN3 = 2;
const int CH_IN4 = 3;

const int MAX_SPEED = 255;
const int SPEED_STEP = 25;
const unsigned long CMD_TIMEOUT_MS = 1200;  // mat ket noi -> tu dung

const int PWM_FREQ = 1000;  // L298N chi theo kip ~1-5 kHz (20 kHz se mat luc)
const int PWM_BITS = 8;     // 0..255

WebServer server(80);

int speed = 160;
char currentDir = 's';
bool moving = false;
unsigned long lastCmdMs = 0;

// ================== DIEU KHIEN DONG CO ==================
int clampSpeed(int value) {
  if (value < 0) return 0;
  if (value > MAX_SPEED) return MAX_SPEED;
  return value;
}

// value > 0: tien, value < 0: lui, 0: tha (coast)
void setMotor(int chForward, int chBackward, int value) {
  if (value > 0) {
    ledcWrite(chBackward, 0);
    ledcWrite(chForward, clampSpeed(value));
  } else if (value < 0) {
    ledcWrite(chForward, 0);
    ledcWrite(chBackward, clampSpeed(-value));
  } else {
    ledcWrite(chForward, 0);
    ledcWrite(chBackward, 0);
  }
}

void setLeftMotor(int value) { setMotor(CH_IN1, CH_IN2, value); }
void setRightMotor(int value) { setMotor(CH_IN3, CH_IN4, value); }

void carStop() {
  setLeftMotor(0);
  setRightMotor(0);
  moving = false;
  currentDir = 's';
}

const char *dirName(char dir) {
  switch (dir) {
    case 'f': return "TIEN";
    case 'b': return "LUI";
    case 'l': return "QUAY TRAI";
    case 'r': return "QUAY PHAI";
    default: return "DUNG";
  }
}

bool applyCommand(char dir) {
  switch (dir) {
    case 'f': setLeftMotor(speed);   setRightMotor(speed);  moving = true;  break;
    case 'b': setLeftMotor(-speed);  setRightMotor(-speed); moving = true;  break;
    case 'l': setLeftMotor(-speed);  setRightMotor(speed);  moving = true;  break;
    case 'r': setLeftMotor(speed);   setRightMotor(-speed); moving = true;  break;
    case 's': carStop();                                    break;
    default: return false;
  }
  currentDir = dir;
  lastCmdMs = millis();
  return true;
}

// ================== WEB INTERFACE ==================
static const char PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>Robot Car - ESP32</title>
<style>
  * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
  body {
    margin: 0; min-height: 100vh; display: flex; flex-direction: column;
    align-items: center; justify-content: center; gap: 18px;
    background: radial-gradient(circle at 50% 0%, #1e2a44 0%, #0d1220 65%);
    color: #e8eefc; font-family: system-ui, -apple-system, "Segoe UI", sans-serif;
    user-select: none; padding: 20px;
  }
  h1 { margin: 0; font-size: 20px; font-weight: 600; letter-spacing: .5px; }
  .sub { font-size: 12px; color: #7f8ca8; margin-top: -12px; }
  .pad {
    display: grid; grid-template-columns: repeat(3, 96px); grid-template-rows: repeat(3, 96px);
    gap: 12px; touch-action: none;
  }
  button.ctl {
    border: 1px solid #2f3f60; border-radius: 22px; background: linear-gradient(180deg, #1c2740, #141c2e);
    color: #cfe0ff; font-size: 34px; display: flex; align-items: center; justify-content: center;
    cursor: pointer; transition: transform .06s, background .12s, box-shadow .12s;
    box-shadow: 0 6px 14px rgba(0,0,0,.35);
    touch-action: none; -webkit-user-select: none; user-select: none;
  }
  button.ctl:active, button.ctl.on {
    background: linear-gradient(180deg, #2f7bff, #1b52c4);
    color: #fff; transform: scale(.94); box-shadow: 0 0 22px rgba(64,132,255,.55);
  }
  .stop { font-size: 26px !important; grid-column: 2; grid-row: 2; border-color: #6d2436 !important; }
  .stop:active { background: linear-gradient(180deg, #ff3b5c, #b3122f) !important; box-shadow: 0 0 22px rgba(255,59,92,.5) !important; }
  .up    { grid-column: 2; grid-row: 1; }
  .left  { grid-column: 1; grid-row: 2; }
  .right { grid-column: 3; grid-row: 2; }
  .down  { grid-column: 2; grid-row: 3; }

  .status-box {
    width: 100%; max-width: 320px; background: #131b2b; border: 1px solid #223050;
    border-radius: 16px; padding: 12px 16px; display: flex; flex-direction: column;
    align-items: center; gap: 6px;
  }
  .status-main { font-size: 16px; color: #e8eefc; display: flex; align-items: center; gap: 8px; }
  .status-main b { color: #4f9bff; font-weight: 700; letter-spacing: .5px; }
  .status-sub { font-size: 12px; color: #7f8ca8; }
  .status-sub b { color: #cfe0ff; font-weight: 600; }
  .dot { width: 9px; height: 9px; border-radius: 50%; background: #2ee06a; display: inline-block; box-shadow: 0 0 8px #2ee06a; }
  .dot.err { background: #ff3b5c; box-shadow: 0 0 8px #ff3b5c; }
</style>
</head>
<body>
  <h1>ESP32 ROBOT CAR</h1>
  <div class="sub">Dieu khien qua WiFi</div>

  <div class="pad">
    <button class="ctl up"    data-dir="f">&#9650;</button>
    <button class="ctl left"  data-dir="l">&#9664;</button>
    <button class="ctl stop"  data-dir="s">&#9632;</button>
    <button class="ctl right" data-dir="r">&#9654;</button>
    <button class="ctl down"  data-dir="b">&#9660;</button>
  </div>

  <div class="status-box">
    <div class="status-main">
      <span class="dot" id="connDot"></span>
      <span>Trang thai: <b id="dirLabel">DUNG</b></span>
    </div>
    <div class="status-sub">IP: <b id="ipLabel">...</b></div>
  </div>

<script>
  const names = { f: 'TIEN', b: 'LUI', l: 'QUAY TRAI', r: 'QUAY PHAI', s: 'DUNG' };
  let holding = null, keepAlive = null;

  const dirLabel = document.getElementById('dirLabel');
  const ipLabel = document.getElementById('ipLabel');
  const connDot = document.getElementById('connDot');

  function setActiveButton(dir) {
    document.querySelectorAll('button.ctl').forEach(b => {
      b.classList.toggle('on', b.dataset.dir === dir && dir !== 's');
    });
  }

  function showDir(dir) {
    dirLabel.textContent = names[dir] || 'DUNG';
    setActiveButton(dir);
  }

  function send(dir) {
    showDir(dir);
    fetch('/cmd?d=' + dir)
      .then(r => {
        if (!r.ok) throw new Error();
        connDot.classList.remove('err');
      })
      .catch(() => {
        connDot.classList.add('err');
      });
  }

  function startHold(dir) {
    holding = dir;
    send(dir);
    clearInterval(keepAlive);
    keepAlive = setInterval(() => { if (holding) send(holding); }, 400);
  }

  function endHold() {
    if (!holding) return;
    holding = null;
    clearInterval(keepAlive);
    keepAlive = null;
    send('s');
  }

  document.querySelectorAll('button.ctl').forEach(btn => {
    const dir = btn.dataset.dir;
    if (dir === 's') {
      btn.addEventListener('pointerdown', e => { e.preventDefault(); endHold(); send('s'); });
      return;
    }
    btn.addEventListener('pointerdown', e => {
      e.preventDefault();
      if (btn.hasPointerCapture && btn.hasPointerCapture(e.pointerId)) return;
      try { btn.setPointerCapture(e.pointerId); } catch (_) {}
      startHold(dir);
    });
    ['pointerup', 'pointercancel'].forEach(ev =>
      btn.addEventListener(ev, () => endHold()));
  });

  window.addEventListener('blur', endHold);
  document.addEventListener('visibilitychange', () => { if (document.hidden) endHold(); });
  document.addEventListener('contextmenu', e => e.preventDefault());

  function refreshStatus() {
    fetch('/status').then(r => r.json()).then(s => {
      connDot.classList.remove('err');
      ipLabel.textContent = s.ip || '...';
      if (!holding) showDir(s.dir || 's');
    }).catch(() => {
      connDot.classList.add('err');
    });
  }

  refreshStatus();
  setInterval(refreshStatus, 500);
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", PAGE_HTML);
}

void handleCmd() {
  String dirArg = server.arg("d");
  if (server.hasArg("spd")) {
    speed = clampSpeed(server.arg("spd").toInt());
  }
  char dir = dirArg.length() ? dirArg[0] : 's';
  if (!applyCommand(dir)) {
    server.send(400, "text/plain", "bad command");
    return;
  }
  Serial.printf("[WEB] %s | speed=%d\n", dirName(dir), speed);
  server.send(200, "text/plain", "ok");
}

void handleStatus() {
  String ip = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
  String json = "{\"dir\":\"";
  json += currentDir;
  json += "\",\"speed\":";
  json += speed;
  json += ",\"ip\":\"";
  json += ip;
  json += "\"}";
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// ================== KET NOI WIFI ==================
bool hasWifiConfig() {
  return strlen(WIFI_SSID) > 0 && strcmp(WIFI_SSID, "TEN_WIFI_CUA_BAN") != 0;
}

bool connectWifi() {
  if (!hasWifiConfig()) {
    Serial.println("Chua cau hinh WiFi trong code.");
    return false;
  }
  Serial.printf("Dang ket noi WiFi: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void startAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.printf("Che do AP: SSID=%s PASS=%s\n", AP_SSID, AP_PASS);
  Serial.printf("Mo trinh duyet: http://%s\n", WiFi.softAPIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Gan 4 chan vao 4 kenh PWM rieng
  ledcSetup(CH_IN1, PWM_FREQ, PWM_BITS);
  ledcAttachPin(IN1_PIN, CH_IN1);
  ledcSetup(CH_IN2, PWM_FREQ, PWM_BITS);
  ledcAttachPin(IN2_PIN, CH_IN2);
  ledcSetup(CH_IN3, PWM_FREQ, PWM_BITS);
  ledcAttachPin(IN3_PIN, CH_IN3);
  ledcSetup(CH_IN4, PWM_FREQ, PWM_BITS);
  ledcAttachPin(IN4_PIN, CH_IN4);

  carStop();

  Serial.println("Chan: IN1=25 IN2=26 IN3=32 IN4=33 (ENA/ENB phai co jumper 5V-EN)");

  if (connectWifi()) {
    Serial.print("WiFi OK. IP: ");
    Serial.println(WiFi.localIP());
    Serial.printf("Mo trinh duyet: http://%s  hoac http://%s.local\n",
                  WiFi.localIP().toString().c_str(), MDNS_NAME);
    if (MDNS.begin(MDNS_NAME)) {
      MDNS.addService("http", "tcp", 80);
    }
  } else {
    startAccessPoint();
  }

  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Web server da san sang.");
}

void loop() {
  server.handleClient();

  if (moving && millis() - lastCmdMs > CMD_TIMEOUT_MS) {
    Serial.println("Mat lenh dieu khien -> tu dung.");
    carStop();
  }
}
