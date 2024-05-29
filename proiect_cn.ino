#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// Initializare LCD la adresa I2C 0x27, dimensiune 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initializare modul RTC DS1307
RTC_DS1307 rtc;

// Definire pini pentru butoane si buzzer
const int setSaveButton = 2;    
const int modeButton = 3;       
const int incrementButton = 4; 
const int switchSettingButton = 5; 
const int buzzerPin = 36;       

// Variabile pentru ora curenta si ora alarmei
int hour = 0, minute = 0;
int alarmHour = 0, alarmMinute = 0;

// Variabile pentru modul de setare a orei si alarmei
bool settingTime = false;
bool settingAlarm = false;
int currentSetting = 0;

// Variabile pentru debouncingul butoanelor
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pinMode(buzzerPin, OUTPUT);
  
  // Verifica daca RTC este gasit
  if (!rtc.begin()) {
    lcd.print("Couldn't find RTC");
    while (1);
  }

  // Verifica daca RTC functioneaza
  if (!rtc.isrunning()) {
    lcd.print("RTC lost power, setting time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcd.clear();
  lcd.print("Clock Started");
  delay(2000);
  lcd.clear();
}

void loop() {
  static unsigned long lastUpdate = 0;
  unsigned long currentMillis = millis();

  DateTime now = rtc.now();

  // Actualizeaza ora curenta doar daca nu suntem in modul de setare
  if (!settingTime && !settingAlarm) {
    hour = now.hour();
    minute = now.minute();
  }

  // Actualizeaza minutul o data la fiecare minut
  if (currentMillis - lastUpdate >= 60000) {
    lastUpdate = currentMillis;
    if (!settingTime && !settingAlarm) {
      minute++;
      if (minute >= 60) {
        minute = 0;
        hour++;
        if (hour >= 24) {
          hour = 0;
        }
      }
    }
  }

  // Activeaza buzzer-ul daca ora si minutul curent corespund cu alarma
  if (hour == alarmHour && minute == alarmMinute) {
    digitalWrite(buzzerPin, LOW);
    delay(200);
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(buzzerPin, HIGH);
  }

  // Verifica butoanele cu debouncing
  if (currentMillis - lastDebounceTime > debounceDelay) {
    //Intarea in modul de Set/Save
    if (digitalRead(setSaveButton) != HIGH) {
      lastDebounceTime = currentMillis;
      if (settingTime) {
        settingTime = false;
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute));
      } else if (settingAlarm) {
        settingAlarm = false;
      } else {
        settingTime = true;
        currentSetting = 0;
      }
    }
    //Intrarea in modul de schimbare set ora/alarma
    if (digitalRead(modeButton) != HIGH) {
      lastDebounceTime = currentMillis;
      if (settingTime) {
        settingAlarm = true;
        settingTime = false;
        currentSetting = 0;
      } else if (settingAlarm) {
        settingAlarm = false;
        settingTime = true;
        currentSetting = 0;
      }
    }
    //Incrementarea variabilei afisate
    if (digitalRead(incrementButton) != HIGH) {
      lastDebounceTime = currentMillis;
      if (settingTime) {
        if (currentSetting == 0) {
          hour = (hour + 1) % 24;
        } else if (currentSetting == 1) {
          minute = (minute + 1) % 60;
        }
      } else if (settingAlarm) {
        if (currentSetting == 0) {
          alarmHour = (alarmHour + 1) % 24;
        } else if (currentSetting == 1) {
          alarmMinute = (alarmMinute + 1) % 60;
        }
      }
    }
    //Intrarea in modul de schimbare minut/ora
    if (digitalRead(switchSettingButton) != HIGH) {
      lastDebounceTime = currentMillis;
      currentSetting = (currentSetting + 1) % 2;
    }
  }
  
  // Afiseaza ora
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  if (hour < 10) lcd.print('0');
  lcd.print(hour);
  lcd.print(":");
  if (minute < 10) lcd.print('0');
  lcd.print(minute);
  //Afisare Set Ora/Alarma
  lcd.setCursor(0, 1);
  if (settingTime) {
    lcd.print("Set Time: ");
    if (currentSetting == 0) {
      if (hour < 10) lcd.print('0');
      lcd.print(hour);
      lcd.print(":");
      lcd.print("  ");
    } else if (currentSetting == 1) {
      lcd.print("  ");
      lcd.print(":");
      if (minute < 10) lcd.print('0');
      lcd.print(minute);
    }
  } else if (settingAlarm) {
    lcd.print("Set Alarm: ");
    if (currentSetting == 0) {
      if (alarmHour < 10) lcd.print('0');
      lcd.print(alarmHour);
      lcd.print(":");
      lcd.print("  ");
    } else if (currentSetting == 1) {
      lcd.print("  ");
      lcd.print(":");
      if (alarmMinute < 10) lcd.print('0');
      lcd.print(alarmMinute);
    }
  //Afisare alarma
  } else {
    lcd.print("Alarm: ");
    if (alarmHour < 10) lcd.print('0');
    lcd.print(alarmHour);
    lcd.print(":");
    if (alarmMinute < 10) lcd.print('0');
    lcd.print(alarmMinute);
    lcd.print("                ");
  }

  delay(100);
}
