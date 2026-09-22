/*
  Joystick Test
  --------------
  Confirms your joystick module is wired correctly before using it
  in anything else.

  Wiring:
    GND -> GND
    +5V -> 5V
    VRx -> A0
    VRy -> A1
    SW  -> D3   (uses INPUT_PULLUP, so it reads LOW when pressed)

  Open Tools > Serial Monitor, set baud rate to 9600.

  What to expect:
    - With the stick left alone (centered), X and Y should both read
      somewhere around 500 (usually 480-520, rarely exactly 512).
    - Push the stick left/right, X should swing toward 0 or 1023.
    - Push the stick up/down, Y should swing toward 0 or 1023.
    - Click the stick straight down, SW should print "PRESSED".
*/

const int VRX_PIN = A0;
const int VRY_PIN = A1;
const int SW_PIN  = 3;

void setup() {
  Serial.begin(9600);
  pinMode(SW_PIN, INPUT_PULLUP);
  Serial.println("=== Joystick test starting ===");
  Serial.println("Move the stick and click it. Leave it centered to check resting values.");
}

bool lastSwState = HIGH;

void loop() {
  int xValue = analogRead(VRX_PIN);
  int yValue = analogRead(VRY_PIN);
  bool swState = digitalRead(SW_PIN);

  Serial.print("X: ");
  Serial.print(xValue);
  Serial.print("   Y: ");
  Serial.print(yValue);
  Serial.print("   SW: ");
  Serial.println(swState == LOW ? "PRESSED" : "released");

  if (swState != lastSwState) {
    lastSwState = swState;
  }

  delay(200); // slow the printing down so it's readable
}
