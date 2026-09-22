/*
  Joystick Test
  --------------
  Confirms your joystick module is wired correctly before using it
  in anything else.

  Wiring (Arduino Uno / Nano):
    GND -> GND
    +5V -> 5V
    VRx -> A0
    VRy -> A1
    SW  -> D3   (uses INPUT_PULLUP, so it reads LOW when pressed)

  Open Tools > Serial Monitor, set baud rate to 9600.

  What to expect:
    - At startup the sketch measures the resting position of the stick,
      so leave it centered while the board resets.
    - With the stick left alone, X and Y should both read somewhere
      around 500 (usually 480-520, rarely exactly 512) and the
      direction should say CENTER.
    - Push the stick left/right, X should swing toward 0 or 1023.
    - Push the stick up/down, Y should swing toward 0 or 1023.
    - Click the stick straight down, SW should print "PRESSED" and a
      ">> Click #n" line appears once per click.

  Serial commands (type in the Serial Monitor and press Enter):
    p  toggle Serial Plotter mode (Tools > Serial Plotter)
    c  re-measure the center position (leave the stick alone first)
    h  show this help again
*/

const int VRX_PIN = A0;
const int VRY_PIN = A1;
const int SW_PIN  = 3;

const unsigned long PRINT_INTERVAL_MS = 200; // how often a reading is printed
const unsigned long DEBOUNCE_MS       = 30;  // ignore switch bounce shorter than this
const int DEADZONE                    = 100; // movement from center that still counts as CENTER
const int CALIBRATION_SAMPLES         = 32;

int centerX = 512;
int centerY = 512;

bool plotterMode = false;

bool swStable = HIGH;          // debounced switch state
bool swLastRaw = HIGH;         // last raw reading, used for debouncing
unsigned long swLastChange = 0;
unsigned long clickCount = 0;

unsigned long lastPrint = 0;

void printHelp() {
  Serial.println(F("Commands: p = toggle plotter mode, c = recalibrate center, h = help"));
}

// Averages a few readings with the stick at rest so direction detection
// works even if your module doesn't rest at exactly 512.
void calibrateCenter() {
  long sumX = 0;
  long sumY = 0;
  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    sumX += analogRead(VRX_PIN);
    sumY += analogRead(VRY_PIN);
    delay(5);
  }
  centerX = sumX / CALIBRATION_SAMPLES;
  centerY = sumY / CALIBRATION_SAMPLES;

  if (plotterMode) return;

  Serial.print(F("Center measured at X: "));
  Serial.print(centerX);
  Serial.print(F("   Y: "));
  Serial.println(centerY);

  // A resting value near either end usually means a wiring problem.
  if (centerX < 100 || centerX > 923) {
    Serial.println(F("WARNING: X is far from center at rest. Check VRx -> A0 and +5V/GND."));
  }
  if (centerY < 100 || centerY > 923) {
    Serial.println(F("WARNING: Y is far from center at rest. Check VRy -> A1 and +5V/GND."));
  }
}

const char *directionName(int x, int y) {
  int dx = x - centerX;
  int dy = y - centerY;
  bool left  = dx < -DEADZONE;
  bool right = dx >  DEADZONE;
  bool up    = dy < -DEADZONE;
  bool down  = dy >  DEADZONE;

  if (up && left)    return "UP-LEFT";
  if (up && right)   return "UP-RIGHT";
  if (down && left)  return "DOWN-LEFT";
  if (down && right) return "DOWN-RIGHT";
  if (up)            return "UP";
  if (down)          return "DOWN";
  if (left)          return "LEFT";
  if (right)         return "RIGHT";
  return "CENTER";
}

// Reads the switch every loop (not just when printing) so quick clicks
// between prints aren't missed.
void updateSwitch() {
  bool raw = digitalRead(SW_PIN);
  if (raw != swLastRaw) {
    swLastRaw = raw;
    swLastChange = millis();
  }

  if (raw != swStable && millis() - swLastChange >= DEBOUNCE_MS) {
    swStable = raw;
    if (swStable == LOW) {
      clickCount++;
      if (!plotterMode) {
        Serial.print(F(">> Click #"));
        Serial.println(clickCount);
      }
    }
  }
}

void handleSerialCommands() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    switch (c) {
      case 'p':
      case 'P':
        plotterMode = !plotterMode;
        if (!plotterMode) Serial.println(F("Plotter mode off."));
        break;
      case 'c':
      case 'C':
        if (!plotterMode) Serial.println(F("Recalibrating, leave the stick centered..."));
        calibrateCenter();
        break;
      case 'h':
      case 'H':
        printHelp();
        break;
      default:
        break; // ignore newlines and anything else
    }
  }
}

void printReading(int x, int y) {
  if (plotterMode) {
    // label:value pairs are understood by the Arduino IDE 2 Serial Plotter
    Serial.print(F("X:"));
    Serial.print(x);
    Serial.print(F(",Y:"));
    Serial.print(y);
    Serial.print(F(",SW:"));
    Serial.println(swStable == LOW ? 1023 : 0);
    return;
  }

  Serial.print(F("X: "));
  Serial.print(x);
  Serial.print(F("   Y: "));
  Serial.print(y);
  Serial.print(F("   SW: "));
  Serial.print(swStable == LOW ? F("PRESSED ") : F("released"));
  Serial.print(F("   Dir: "));
  Serial.println(directionName(x, y));
}

void setup() {
  Serial.begin(9600);
  pinMode(SW_PIN, INPUT_PULLUP);

  Serial.println(F("=== Joystick test starting ==="));
  Serial.println(F("Leave the stick centered for a moment..."));
  calibrateCenter();
  Serial.println(F("Move the stick and click it."));
  printHelp();
}

void loop() {
  handleSerialCommands();
  updateSwitch();

  unsigned long now = millis();
  if (now - lastPrint >= PRINT_INTERVAL_MS) {
    lastPrint = now;
    printReading(analogRead(VRX_PIN), analogRead(VRY_PIN));
  }
}
