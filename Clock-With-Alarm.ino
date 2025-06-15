#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int buttonPin = 2;
const int buzzerPin = 3;

// Alarm config
int alarmHour = 7;
int alarmMinute = 0;
bool alarmTriggered = false;
bool settingAlarm = false;
bool settingHour = true;

unsigned long buttonPressTime = 0;
bool lastButtonState = HIGH;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  lcd.init();
  lcd.backlight();
  rtc.begin();

  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alarm Clock Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  // --- Button Handling ---
  bool buttonState = digitalRead(buttonPin);
  unsigned long currentTime = millis();

  // Long press detection (enter set mode)
  if (buttonState == LOW && lastButtonState == HIGH) {
    buttonPressTime = currentTime;
  }

  if (buttonState == LOW && !settingAlarm && (currentTime - buttonPressTime > 1000)) {
    settingAlarm = true;
    settingHour = true;
    lcd.clear();
  }

  // Short press in set mode
  if (buttonState == HIGH && lastButtonState == LOW) {
    if (settingAlarm) {
      if (settingHour) {
        alarmHour = (alarmHour + 1) % 24;
      } else {
        alarmMinute = (alarmMinute + 1) % 60;
        settingAlarm = false; // Done setting
        lcd.clear();
      }
    }
  }

  lastButtonState = buttonState;

  // --- Display ---
  lcd.setCursor(0, 0);
  char timeBuf[9];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  lcd.print("Time: ");
  lcd.print(timeBuf);
  lcd.print(" ");

  lcd.setCursor(0, 1);
  if (settingAlarm) {
    if (settingHour) {
      lcd.print("Set Hour: ");
      lcd.print((alarmHour < 10) ? "0" : "");
      lcd.print(alarmHour);
      lcd.print("        ");
    } else {
      lcd.print("Set Minute: ");
      lcd.print((alarmMinute < 10) ? "0" : "");
      lcd.print(alarmMinute);
      lcd.print("      ");
    }
  } else {
    lcd.print("Alarm: ");
    lcd.print((alarmHour < 10) ? "0" : "");
    lcd.print(alarmHour);
    lcd.print(":");
    lcd.print((alarmMinute < 10) ? "0" : "");
    lcd.print(alarmMinute);
    lcd.print(alarmTriggered ? " *TRIG*" : "      ");
  }

  // Automatically switch from hour to minute after delay
  if (settingAlarm && settingHour && (currentTime - buttonPressTime > 3000)) {
    settingHour = false;
    lcd.clear();
  }

  // --- Alarm Logic ---
  if (!settingAlarm &&
      now.hour() == alarmHour &&
      now.minute() == alarmMinute &&
      now.second() == 0 &&
      !alarmTriggered) {

    alarmTriggered = true;

    for (int i = 0; i < 10; i++) {
      tone(buzzerPin, 1000);
      delay(300);
      noTone(buzzerPin);
      delay(300);
    }
  }

  // Reset alarm trigger so it works again tomorrow
  if (now.minute() != alarmMinute) {
    alarmTriggered = false;
  }

  delay(200);
}
