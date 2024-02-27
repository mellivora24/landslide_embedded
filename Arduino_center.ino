#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

SoftwareSerial lora(10, 11);

const int den = 3;
const int coi = 7;

LiquidCrystal_I2C lcd(0x27, 16, 2);

int count = 0;

unsigned long long time_disconnect = millis();

void warning(byte state) {
  digitalWrite(den, state);
  digitalWrite(coi, state);
}

void setup() {
  lora.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(den, OUTPUT);
  pinMode(coi, OUTPUT);

  while (!lora.available()) delay(100);
}

void loop() {

  while (lora.available()) {
    char income_data = (char)lora.read();
    /*switch (income_data) {
      case '+':
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Canh bao !");

        count++;
        if (count == 10) {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Canh bao !");
          lcd.setCursor(2, 1);
          lcd.print("Mat ket noi!");
          warning(1);
        }

        break;
      case 'A':
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Sat lo doi 1 !");
        warning(1);
        break;
      case 'B':
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Sat lo doi 2 !");
        warning(1);
        break;
      case '-':
        warning(0);
        lcd.clear();
      default:
        break;
    }*/
    if (income_data == 'A') {
      lcd.setCursor(0, 0);
      lcd.print("Ket noi A!      ");
      warning(0);
    }
    if (income_data == 'B') {
      lcd.setCursor(0, 1);
      lcd.print("Ket noi B!      ");
      warning(0);
    }
    if (income_data == '+') {
      lcd.setCursor(0, 0);
      lcd.print("Canh bao sat lo!");
      lcd.setCursor(0, 1);
      lcd.print("Tai diem A      ");
      warning(1);
    }
    if (income_data == '-') {
      lcd.setCursor(0, 0);
      lcd.print("Canh bao sat lo!");
      lcd.setCursor(0, 1);
      lcd.print("Tai diem B      ");
      warning(1);
    }
  }

  if (!lora.available()) {
    if (millis() - time_disconnect > 1000) {
      count++;
      if (count == 6) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Khong co ket noi");
        warning(1);
      }
      time_disconnect = millis();
    }
  }
}