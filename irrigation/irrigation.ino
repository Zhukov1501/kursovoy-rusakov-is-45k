// Автополив Irrigation, Arduino Uno (Русаков А.А., ИС-45к)
#include <Adafruit_LiquidCrystal.h>

const int SOIL = A3, RAIN = A2;
const int LED_G = 3, LED_Y = 5, LED_R = 10, PUMP = 9, BUZ = 11, MUTE = 6, MODE = 8;
const int DRY_T = 30, OK_T = 55;
const int OK = 0, MID = 1, DRY = 2;
int st = OK, last = OK, page = 0;
bool muted = false, pMute = HIGH, pMode = HIGH;
unsigned long tBeep = 0, tLcd = 0;
Adafruit_LiquidCrystal lcd(0);

int pct(int pin) {
  return constrain(map(analogRead(pin), 0, 1023, 0, 100), 0, 100);
}

int classify(int soil) {
  if (soil < DRY_T) return DRY;
  if (soil < OK_T) return MID;
  return OK;
}

const char* name(int s) {
  if (s == DRY) return "DRY";
  if (s == MID) return "MID";
  return "OK";
}

void setLeds(int s) {
  digitalWrite(LED_G, s == OK); digitalWrite(LED_Y, s == MID); digitalWrite(LED_R, s == DRY);
}

void show(int soil, int rain) {
  char a[17], b[17];
  if (page == 0) {
    snprintf(a, 17, "Soil:%3d%%      ", soil);
    snprintf(b, 17, muted ? "Mode:%-4s  M    " : "Mode:%-4s       ", name(st));
  } else {
    snprintf(a, 17, "Rain:%3d%% Pump%c", rain, digitalRead(PUMP) ? '+' : '-');
    snprintf(b, 17, "Irrigat    v1.0");
  }
  lcd.setCursor(0, 0); lcd.print(a);
  lcd.setCursor(0, 1); lcd.print(b);
}

bool edge(bool now, bool& prev) {
  bool e = (prev == HIGH && now == LOW); prev = now; return e;
}

void setup() {
  pinMode(LED_G, OUTPUT); pinMode(LED_Y, OUTPUT); pinMode(LED_R, OUTPUT);
  pinMode(PUMP, OUTPUT); pinMode(BUZ, OUTPUT);
  pinMode(MUTE, INPUT_PULLUP); pinMode(MODE, INPUT_PULLUP);
  lcd.begin(16, 2); lcd.setBacklight(1); lcd.print("Irrigat v1.0");
  digitalWrite(LED_G, 1); delay(150); digitalWrite(LED_G, 0);
  digitalWrite(LED_Y, 1); delay(150); digitalWrite(LED_Y, 0);
  digitalWrite(LED_R, 1); delay(150); digitalWrite(LED_R, 0);
  tone(BUZ, 1000, 80); delay(200); lcd.clear(); setLeds(OK);
}

void loop() {
  int soil = pct(SOIL), rain = pct(RAIN);
  st = classify(soil);
  if (edge(digitalRead(MUTE), pMute)) muted = !muted;
  if (edge(digitalRead(MODE), pMode)) page = 1 - page;
  if (st != last) last = st;
  setLeds(st);
  // насос: сухая почва и нет сильного «дождя»
  bool pumpOn = (st == DRY && rain < 60);
  digitalWrite(PUMP, pumpOn);
  if (muted || st != DRY) noTone(BUZ);
  else if (millis() - tBeep >= 400) { tone(BUZ, 1800, 120); tBeep = millis(); }
  if (millis() - tLcd >= 250) { show(soil, rain); tLcd = millis(); }
}
