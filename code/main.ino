#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <NewPing.h>

// =====================================================
// WIFI
// =====================================================

const char* ssid = "Infinix HOT 40 Pro";
const char* password = "12345678";

// =====================================================
// PIN MAPPING
// =====================================================

#define TRIG_PIN       16
#define ECHO_PIN       17

#define SERVO1_PIN     32
#define SERVO2_PIN     33

#define LCD_SDA        21
#define LCD_SCL        22

#define BUZZER_PIN     23

#define GREEN_LED      25
#define YELLOW_LED     26
#define RED_LED        27

#define LASER_PIN      4

// =====================================================
// SETTINGS
// =====================================================

#define MAX_DISTANCE       400
#define DETECT_DISTANCE    40

// Servo 1 speed
// Higher = slower
#define SERVO_INTERVAL     45

// Ultrasonic interval
#define SENSOR_INTERVAL    60

// Buzzer speed
#define BUZZER_INTERVAL    180

// If Servo 2 is physically reversed,
// change this to true.
#define SERVO2_REVERSE     false

// =====================================================
// OBJECTS
// =====================================================

Servo servo1;
Servo servo2;

NewPing sonar(
  TRIG_PIN,
  ECHO_PIN,
  MAX_DISTANCE
);

LiquidCrystal_I2C lcd(
  0x27,
  16,
  2
);

WebServer server(80);

// =====================================================
// VARIABLES
// =====================================================

int servo1Angle = 0;
int servo1Direction = 1;

int servo2Angle = 0;

int distanceCm = MAX_DISTANCE;

int detectedAngle = 0;
int detectedDistance = 0;

bool objectDetected = false;
bool previousObjectDetected = false;

bool radarRunning = true;

bool buzzerState = false;

unsigned long lastServoMove = 0;
unsigned long lastSensorRead = 0;
unsigned long lastBuzzer = 0;


// =====================================================
// HTML DASHBOARD
// =====================================================

const char MAIN_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta name="viewport"
content="width=device-width, initial-scale=1">

<title>ESP32 Radar System</title>

<style>

* {
  box-sizing: border-box;
}

body {

  margin: 0;

  background: #020603;

  color: #8cff45;

  font-family:
  Arial,
  Helvetica,
  sans-serif;

}

.header {

  text-align: center;

  padding: 20px;

  font-size: 32px;

  font-weight: bold;

  letter-spacing: 3px;

  text-shadow:
  0 0 15px #39ff14;

}

.container {

  display: grid;

  grid-template-columns:
  1fr 300px;

  gap: 15px;

  padding: 15px;

}

.radarBox {

  background: #020803;

  border:
  1px solid #39ff14;

  border-radius: 15px;

  padding: 10px;

  box-shadow:
  0 0 20px #0b4d0b;

}

canvas {

  width: 100%;

  height: auto;

  display: block;

}

.panel {

  display: flex;

  flex-direction: column;

  gap: 15px;

}

.card {

  background:
  rgba(0,20,5,0.8);

  border:
  1px solid #39ff14;

  border-radius: 12px;

  padding: 18px;

}

.title {

  color: #5cff32;

  font-size: 16px;

  font-weight: bold;

}

.value {

  color: white;

  font-size: 36px;

  font-weight: bold;

  text-align: center;

  margin-top: 10px;

}

.status {

  font-size: 23px;

  font-weight: bold;

  margin-top: 15px;

}

.red {

  color: #ff2020;

}

.green {

  color: #39ff14;

}

.yellow {

  color: #ffff00;

}

.buttons {

  display: flex;

  gap: 10px;

}

button {

  flex: 1;

  padding: 14px;

  border: none;

  border-radius: 8px;

  font-size: 16px;

  font-weight: bold;

  cursor: pointer;

}

.start {

  background: #39ff14;

  color: black;

}

.stop {

  background: #ff2020;

  color: white;

}

.log {

  margin: 15px;

  border:
  1px solid #39ff14;

  border-radius: 12px;

  padding: 15px;

  height: 160px;

  overflow-y: auto;

  background: #010401;

}

.logTitle {

  font-size: 18px;

  margin-bottom: 10px;

}

.logItem {

  color: #ddd;

  margin: 5px 0;

  font-family: monospace;

}

@media(max-width:900px) {

  .container {

    grid-template-columns: 1fr;

  }

  .header {

    font-size: 24px;

  }

}

</style>

</head>


<body>


<div class="header">

ESP32 RADAR SYSTEM

</div>


<div class="container">


<div class="radarBox">

<canvas id="radar"
width="900"
height="600">
</canvas>

</div>


<div class="panel">


<div class="card">

<div class="title">
STATUS
</div>

<div id="status"
class="status green">
NO OBJECT
</div>

<div id="running"
class="green">
Scanning...
</div>

</div>


<div class="card">

<div class="title">
ANGLE
</div>

<div id="angle"
class="value">
0°
</div>

</div>


<div class="card">

<div class="title">
DISTANCE
</div>

<div id="distance"
class="value">
400 cm
</div>

</div>


<div class="card">

<div class="title">
DETECTED AT
</div>

<div id="detected"
class="value"
style="font-size:26px">

--°

</div>

</div>


<div class="card">

<div class="title">
CONNECTION
</div>

<div id="connection"
class="status green">

CONNECTED

</div>

</div>


<div class="card">

<div class="title">
CONTROLS
</div>

<br>

<div class="buttons">

<button
class="start"
onclick="startRadar()">

▶ START

</button>

<button
class="stop"
onclick="stopRadar()">

■ STOP

</button>

</div>

</div>


</div>

</div>


<div class="log">

<div class="logTitle">
DETECTION LOG
</div>

<div id="log"></div>

</div>


<script>

const canvas =
document.getElementById("radar");

const ctx =
canvas.getContext("2d");

let angle = 0;

let distance = 400;

let detected = false;

let detectedAngle = 0;

let detectedDistance = 0;

let targets = [];

let lastDetect = false;


// =================================================
// RADAR DRAW
// =================================================

function drawRadar() {

  const w = canvas.width;

  const h = canvas.height;

  const cx = w / 2;

  const cy = h - 40;

  const radius =
  Math.min(w * 0.45, h - 70);


  ctx.clearRect(
    0,
    0,
    w,
    h
  );


  // Background

  ctx.fillStyle =
  "#010501";

  ctx.fillRect(
    0,
    0,
    w,
    h
  );


  // Radar circles

  ctx.strokeStyle =
  "#39ff14";

  ctx.lineWidth = 2;


  for (
    let d = 100;
    d <= 400;
    d += 100
  ) {

    let r =
    radius * d / 400;

    ctx.beginPath();

    ctx.arc(
      cx,
      cy,
      r,
      Math.PI,
      2 * Math.PI
    );

    ctx.stroke();


    ctx.fillStyle =
    "#66ff33";

    ctx.font =
    "14px Arial";

    ctx.fillText(
      d + " cm",
      cx + 8,
      cy - r
    );

  }


  // Angle lines

  for (
    let a = 0;
    a <= 180;
    a += 30
  ) {

    let rad =
    Math.PI -
    a * Math.PI / 180;

    let x =
    cx + Math.cos(rad) * radius;

    let y =
    cy - Math.sin(rad) * radius;


    ctx.beginPath();

    ctx.moveTo(
      cx,
      cy
    );

    ctx.lineTo(
      x,
      y
    );

    ctx.stroke();


    let tx =
    cx + Math.cos(rad) *
    (radius + 25);

    let ty =
    cy - Math.sin(rad) *
    (radius + 25);


    ctx.fillStyle =
    "#ffffff";

    ctx.font =
    "16px Arial";

    ctx.fillText(
      a + "°",
      tx - 10,
      ty
    );

  }


  // Bottom line

  ctx.beginPath();

  ctx.moveTo(
    cx - radius,
    cy
  );

  ctx.lineTo(
    cx + radius,
    cy
  );

  ctx.stroke();


  // Target history

  targets.forEach(
  function(t) {

    let rad =
    Math.PI -
    t.angle *
    Math.PI / 180;


    let r =
    radius *
    t.distance /
    400;


    let x =
    cx +
    Math.cos(rad) * r;


    let y =
    cy -
    Math.sin(rad) * r;


    // glow

    ctx.beginPath();

    ctx.arc(
      x,
      y,
      15,
      0,
      2*Math.PI
    );

    ctx.fillStyle =
    "rgba(255,0,0,0.15)";

    ctx.fill();


    // target

    ctx.beginPath();

    ctx.arc(
      x,
      y,
      6,
      0,
      2*Math.PI
    );

    ctx.fillStyle =
    "#ff2020";

    ctx.fill();

  });


  // Scanner

  let rad =
  Math.PI -
  angle *
  Math.PI / 180;


  let sx =
  cx +
  Math.cos(rad) * radius;


  let sy =
  cy -
  Math.sin(rad) * radius;


  ctx.beginPath();

  ctx.moveTo(
    cx,
    cy
  );

  ctx.lineTo(
    sx,
    sy
  );


  ctx.strokeStyle =
  "#39ff14";

  ctx.lineWidth = 4;

  ctx.stroke();


  // Center

  ctx.beginPath();

  ctx.arc(
    cx,
    cy,
    7,
    0,
    2*Math.PI
  );

  ctx.fillStyle =
  "#39ff14";

  ctx.fill();

}


// =================================================
// UPDATE DATA
// =================================================

async function updateData() {

  try {

    const response =
    await fetch(
      "/api/status"
    );

    const data =
    await response.json();


    angle =
    data.angle;

    distance =
    data.distance;

    detected =
    data.detected;

    detectedAngle =
    data.detectedAngle;

    detectedDistance =
    data.detectedDistance;


    document.getElementById(
      "angle"
    ).innerText =
    angle + "°";


    document.getElementById(
      "distance"
    ).innerText =
    distance + " cm";


    document.getElementById(
      "detected"
    ).innerText =
    detected ?
    detectedAngle + "° / " +
    detectedDistance + " cm" :
    "--";


    const status =
    document.getElementById(
      "status"
    );


    if (detected) {

      status.innerText =
      "OBJECT DETECTED";

      status.className =
      "status red";

    }
    else {

      status.innerText =
      "NO OBJECT";

      status.className =
      "status green";

    }


    document.getElementById(
      "running"
    ).innerText =
    data.running ?
    "Scanning..." :
    "Stopped";


    // New detection

    if (
      detected &&
      !lastDetect
    ) {

      addLog(
        "OBJECT DETECTED!  Angle: " +
        detectedAngle +
        "°  Distance: " +
        detectedDistance +
        " cm"
      );


      targets.push({

        angle:
        detectedAngle,

        distance:
        detectedDistance

      });


      if (
        targets.length > 30
      ) {

        targets.shift();

      }

    }


    lastDetect =
    detected;


    document.getElementById(
      "connection"
    ).innerText =
    "CONNECTED";


  }

  catch(error) {

    document.getElementById(
      "connection"
    ).innerText =
    "DISCONNECTED";

  }

}


// =================================================
// LOG
// =================================================

function addLog(message) {

  const log =
  document.getElementById(
    "log"
  );


  const time =
  new Date().toLocaleTimeString();


  const item =
  document.createElement(
    "div"
  );


  item.className =
  "logItem";


  item.innerText =
  "[" + time + "] " +
  message;


  log.prepend(item);


  while (
    log.children.length > 20
  ) {

    log.removeChild(
      log.lastChild
    );

  }

}


// =================================================
// BUTTONS
// =================================================

function startRadar() {

  fetch(
    "/api/start"
  );

  addLog(
    "Radar STARTED"
  );

}


function stopRadar() {

  fetch(
    "/api/stop"
  );

  addLog(
    "Radar STOPPED"
  );

}


// =================================================
// LOOP
// =================================================

function loop() {

  updateData();

  drawRadar();

  requestAnimationFrame(
    loop
  );

}

loop();

</script>

</body>

</html>

)rawliteral";


// =====================================================
// GET DISTANCE
// =====================================================

int getDistance() {

  int d =
  sonar.ping_cm();

  if (d == 0) {

    return MAX_DISTANCE;

  }

  return d;
}


// =====================================================
// UPDATE LCD
// =====================================================

void updateLCD() {

  lcd.clear();

  lcd.setCursor(
    0,
    0
  );

  lcd.print(
    "RADAR SYSTEM"
  );


  lcd.setCursor(
    0,
    1
  );


  if (objectDetected) {

    lcd.print(
      "OBJ "
    );

    lcd.print(
      detectedAngle
    );

    lcd.print(
      (char)223
    );

    lcd.print(
      " "
    );

    lcd.print(
      detectedDistance
    );

    lcd.print(
      "cm"
    );

  }
  else {

    lcd.print(
      "No Object"
    );

  }

}


// =====================================================
// LED CONTROL
// =====================================================

void updateLEDs() {

  if (objectDetected) {

    digitalWrite(
      GREEN_LED,
      LOW
    );

    digitalWrite(
      YELLOW_LED,
      LOW
    );

    digitalWrite(
      RED_LED,
      HIGH
    );

  }

  else if (radarRunning) {

    digitalWrite(
      GREEN_LED,
      LOW
    );

    digitalWrite(
      YELLOW_LED,
      HIGH
    );

    digitalWrite(
      RED_LED,
      LOW
    );

  }

  else {

    digitalWrite(
      GREEN_LED,
      HIGH
    );

    digitalWrite(
      YELLOW_LED,
      LOW
    );

    digitalWrite(
      RED_LED,
      LOW
    );

  }

}


// =====================================================
// SERVO 2 MOVE
// =====================================================

void moveServo2ToDetectedAngle() {

  int targetAngle =
  detectedAngle;


  if (SERVO2_REVERSE) {

    targetAngle =
    180 - targetAngle;

  }


  targetAngle =
  constrain(
    targetAngle,
    0,
    180
  );


  servo2Angle =
  targetAngle;


  // Immediate movement

  servo2.write(
    targetAngle
  );


  Serial.print(
    "SERVO2 -> "
  );

  Serial.println(
    targetAngle
  );

}


// =====================================================
// SCAN SERVO 1
// =====================================================

void scanServo1() {

  if (!radarRunning) {

    return;

  }


  unsigned long now =
  millis();


  if (
    now - lastServoMove >=
    SERVO_INTERVAL
  ) {

    lastServoMove =
    now;


    servo1Angle +=
    servo1Direction;


    if (
      servo1Angle >= 180
    ) {

      servo1Angle =
      180;

      servo1Direction =
      -1;

    }


    if (
      servo1Angle <= 0
    ) {

      servo1Angle =
      0;

      servo1Direction =
      1;

    }


    servo1.write(
      servo1Angle
    );

  }

}


// =====================================================
// OBJECT DETECTION
// =====================================================

void checkObject() {

  unsigned long now =
  millis();


  if (
    now - lastSensorRead <
    SENSOR_INTERVAL
  ) {

    return;

  }


  lastSensorRead =
  now;


  distanceCm =
  getDistance();


  objectDetected =
  (
    distanceCm > 0 &&
    distanceCm <=
    DETECT_DISTANCE
  );


  // -----------------------------------------------
  // NEW OBJECT DETECTED
  // -----------------------------------------------

  if (
    objectDetected &&
    !previousObjectDetected
  ) {

    detectedAngle =
    servo1Angle;

    detectedDistance =
    distanceCm;


    // Servo 2 follows target

    moveServo2ToDetectedAngle();


    // Laser ON

    digitalWrite(
      LASER_PIN,
      HIGH
    );


    Serial.print(
      "DETECT,"
    );

    Serial.print(
      detectedAngle
    );

    Serial.print(
      ","
    );

    Serial.println(
      detectedDistance
    );

  }


  // -----------------------------------------------
  // NO OBJECT
  // -----------------------------------------------

  if (!objectDetected) {

    digitalWrite(
      LASER_PIN,
      LOW
    );

  }


  previousObjectDetected =
  objectDetected;


  updateLEDs();

  updateLCD();

}


// =====================================================
// BUZZER
// =====================================================

void updateBuzzer() {

  unsigned long now =
  millis();


  if (objectDetected) {

    if (
      now - lastBuzzer >=
      BUZZER_INTERVAL
    ) {

      lastBuzzer =
      now;


      buzzerState =
      !buzzerState;


      digitalWrite(
        BUZZER_PIN,
        buzzerState
      );

    }

  }

  else {

    buzzerState =
    false;


    digitalWrite(
      BUZZER_PIN,
      LOW
    );

  }

}


// =====================================================
// API STATUS
// =====================================================

void handleStatus() {

  String json = "{";


  json +=
  "\"angle\":" +
  String(servo1Angle) +
  ",";


  json +=
  "\"distance\":" +
  String(distanceCm) +
  ",";


  json +=
  "\"detected\":" +
  String(
    objectDetected ?
    "true" :
    "false"
  ) +
  ",";


  json +=
  "\"detectedAngle\":" +
  String(detectedAngle) +
  ",";


  json +=
  "\"detectedDistance\":" +
  String(detectedDistance) +
  ",";


  json +=
  "\"servo2Angle\":" +
  String(servo2Angle) +
  ",";


  json +=
  "\"running\":" +
  String(
    radarRunning ?
    "true" :
    "false"
  );


  json +=
  "}";


  server.send(
    200,
    "application/json",
    json
  );

}


// =====================================================
// START
// =====================================================

void handleStart() {

  radarRunning =
  true;


  updateLEDs();


  server.send(
    200,
    "text/plain",
    "STARTED"
  );

}


// =====================================================
// STOP
// =====================================================

void handleStop() {

  radarRunning =
  false;


  updateLEDs();


  server.send(
    200,
    "text/plain",
    "STOPPED"
  );

}


// =====================================================
// ROOT
// =====================================================

void handleRoot() {

  server.send_P(
    200,
    "text/html",
    MAIN_PAGE
  );

}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(
    115200
  );


  // -----------------------------------------------
  // Pins
  // -----------------------------------------------

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  pinMode(
    GREEN_LED,
    OUTPUT
  );

  pinMode(
    YELLOW_LED,
    OUTPUT
  );

  pinMode(
    RED_LED,
    OUTPUT
  );

  pinMode(
    LASER_PIN,
    OUTPUT
  );


  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  digitalWrite(
    LASER_PIN,
    LOW
  );


  // -----------------------------------------------
  // LCD
  // -----------------------------------------------

  Wire.begin(
    LCD_SDA,
    LCD_SCL
  );


  lcd.init();

  lcd.backlight();

  lcd.clear();

  lcd.setCursor(
    0,
    0
  );

  lcd.print(
    "RADAR SYSTEM"
  );

  lcd.setCursor(
    0,
    1
  );

  lcd.print(
    "Starting..."
  );


  // -----------------------------------------------
  // Servos
  // -----------------------------------------------

  servo1.setPeriodHertz(
    50
  );

  servo2.setPeriodHertz(
    50
  );


  servo1.attach(
    SERVO1_PIN,
    500,
    2400
  );


  servo2.attach(
    SERVO2_PIN,
    500,
    2400
  );


  servo1.write(
    0
  );

  servo2.write(
    0
  );


  // -----------------------------------------------
  // WiFi
  // -----------------------------------------------

  WiFi.begin(
    ssid,
    password
  );


  Serial.print(
    "Connecting WiFi"
  );


  int attempts = 0;


  while (
    WiFi.status() !=
    WL_CONNECTED &&
    attempts < 30
  ) {

    delay(500);

    Serial.print(".");

    attempts++;

  }


  Serial.println();


  if (
    WiFi.status() ==
    WL_CONNECTED
  ) {

    Serial.println(
      "WiFi Connected!"
    );


    Serial.print(
      "IP Address: "
    );

    Serial.println(
      WiFi.localIP()
    );


    lcd.clear();

    lcd.setCursor(
      0,
      0
    );

    lcd.print(
      "WiFi Connected"
    );

    lcd.setCursor(
      0,
      1
    );

    lcd.print(
      WiFi.localIP()
    );

  }

  else {

    Serial.println(
      "WiFi Failed!"
    );

    lcd.clear();

    lcd.setCursor(
      0,
      0
    );

    lcd.print(
      "WiFi Failed"
    );

  }


  // -----------------------------------------------
  // Web Server
  // -----------------------------------------------

  server.on(
    "/",
    handleRoot
  );


  server.on(
    "/api/status",
    handleStatus
  );


  server.on(
    "/api/start",
    handleStart
  );


  server.on(
    "/api/stop",
    handleStop
  );


  server.begin();


  Serial.println(
    "Web Server Started"
  );


  delay(1000);


  lcd.clear();

  lcd.setCursor(
    0,
    0
  );

  lcd.print(
    "RADAR SYSTEM"
  );

  lcd.setCursor(
    0,
    1
  );

  lcd.print(
    "No Object"
  );


  updateLEDs();

}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // Web server
  server.handleClient();


  // Servo 1 continuously scans
  scanServo1();


  // Ultrasonic detection
  checkObject();


  // Buzzer
  updateBuzzer();

}
