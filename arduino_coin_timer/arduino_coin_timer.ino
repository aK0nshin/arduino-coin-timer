#include <GyverTM1637.h>

#define CLK_PIN 2
#define DIO_PIN 3
#define COIN_PIN 4
#define PlAY_BTN_PIN 5
#define RESET_BTN_PIN 6
#define RESULT_PIN 12

#define COIN_SECONDS 600
#define POINTS_PING_MILLIS 500
#define CLOCK_UPDATE_MILLIS 1000

GyverTM1637 disp(CLK_PIN, DIO_PIN);

volatile uint32_t innerTime = 0;
uint32_t lastCoinTime = 0;
uint32_t lastPingPoint = 0;
uint32_t lastClockUpdate = 0;
uint8_t brightness = 7;   // яркость, 0 - 7 (минимум - максимум)
bool pointsFlag = false;

enum state {
  STOP, 
  PLAY,
  PAUSE,
  END,
};
state myState;

void toStop() {
  Serial.println("toStop start myState: " + String(myState));
  myState = STOP;
  digitalWrite(RESULT_PIN, LOW);
  innerTime = 0;
  pointsFlag = true;
  disp.point(pointsFlag);
}

void toPlay() {
  Serial.println("toPlay start myState: " + String(myState));
  myState = PLAY;
  digitalWrite(RESULT_PIN, LOW);
}

void toPause() {
  Serial.println("toPause start myState: " + String(myState));
  myState = PAUSE;
  digitalWrite(RESULT_PIN, LOW);
}

void toEnd() {
  Serial.println("toEnd start myState: " + String(myState));
  myState = END;
  digitalWrite(RESULT_PIN, HIGH);
}
 
void pingPoints() {
  if (millis() - lastPingPoint >= POINTS_PING_MILLIS) {
    pointsFlag = !pointsFlag;
    disp.point(pointsFlag);
  }
}

void pingClock() {
  if (millis() - lastPingPoint >= POINTS_PING_MILLIS) {
    if (brightness == 7) {
      brightness = 0;
    } else {
      brightness = 7;
    }
    disp.brightness(brightness);
  }
}

void updateTime() {
  if (millis() - lastClockUpdate >= CLOCK_UPDATE_MILLIS) {
    if (innerTime < 1) {
      return toEnd();
    }
    innerTime--;
    displayClock();
  }
}

void displayClock() {
  int16_t mins = 99, secs = 99;
  mins = innerTime / 60;
  if (mins > 99) mins = 99;
  secs = innerTime % 60;
  disp.displayClock(mins, secs);
}

void coinHandler() {
  Serial.println("coinHandler start innerTime: " + String(innerTime));
  if (lastCoinTime + 300 > millis()) {
    Serial.println("coinHandler debounce lastCoinTime: " + String(lastCoinTime) + " millis: " + millis());
    return;
  }
  innerTime += COIN_SECONDS;
  lastCoinTime = millis();
  displayClock();
  Serial.println("coinHandler end innerTime: " + String(innerTime));
}

void playHandler() {
  Serial.println("playHandler start myState: " + String(myState));
  switch (myState) {
    case PLAY:
      toPause();
      break;
    case STOP:
    case PAUSE:
      toPlay();
  }
}

void resetHandler() {
  Serial.println("resetHandler start myState: " + String(myState));
  switch (myState) {
    case PLAY:
    case PAUSE:
    case END:
      toStop();
  }
}

void setup() {
  pinMode(COIN_PIN, INPUT);
  pinMode(PlAY_BTN_PIN, INPUT);
  pinMode(RESET_BTN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(COIN_PIN), coinHandler, FALLING);
  attachInterrupt(digitalPinToInterrupt(PlAY_BTN_PIN), playHandler, RISING);
  attachInterrupt(digitalPinToInterrupt(RESET_BTN_PIN), resetHandler, RISING);

  pinMode(RESULT_PIN, OUTPUT);

  disp.clear();
  disp.brightness(brightness);
  displayClock();
  myState = STOP;
  Serial.begin(115200);
}

void loop() {
  switch (myState) {
    case STOP:
      break;
    case PLAY:
      pingPoints();
      updateTime();
      break;
    case PAUSE:
      break;
    case END:
      pingClock();
      break;
    default:
      toStop();
  } 
}
