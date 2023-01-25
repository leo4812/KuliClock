#include <Arduino.h>
// *************************** DHT22 ******************************
#include <Adafruit_Sensor.h>
#include "DHT.h"
// *************************** LCD1602 ****************************
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 2      // пин DHT22
#define DHTTYPE DHT22 // тип датчика DHT 22  (AM2302), AM2321

uint8_t Hum = 0;
float Temp = 0;
uint32_t TimerDHT = 0;
uint32_t TimerFlag = 0;
uint16_t MyPeriod = 30000;

uint32_t Iterations = 0; // Счетчик количества итераций (сбрасывается при смене флажков)

bool Flag_Time = true;
bool Flag_HumTemp = false;

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void Start();   // Приветствие
void ReadDHT(); // Чтение DHT22
void Time();    // Left to work (Осталось работать)
void HumTemp(); // HumTemp (Влажность и температура)

void setup()
{
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  Start();
  TimerFlag = millis();
}

void loop()
{
  ReadDHT();
  if ((millis() - TimerFlag) >= MyPeriod)
  {
    TimerFlag = millis();
    Iterations = 0;
    if (Flag_Time == true)
    {
      Flag_Time = false;
      Flag_HumTemp = true;
    }
    else if (Flag_HumTemp == true)
    {
      Flag_Time = true;
      Flag_HumTemp = false;
    }
  }
  //*************************************************
  if (Flag_Time == true)
  {
    Time();
  }
  if (Flag_HumTemp == true)
  {
    HumTemp();
  }
}
void Start()
{
  lcd.backlight(); // включаем подсветку дисплея
  delay(300);

  lcd.cursor();
  lcd.setCursor(0, 0);

  lcd.print("H");
  delay(100);
  lcd.print("e");
  delay(100);
  lcd.print("l");
  delay(100);
  lcd.print("l");
  delay(100);
  lcd.print("o");
  delay(100);
  lcd.print(" ");
  delay(100);
  lcd.print("f");
  delay(100);
  lcd.print("r");
  delay(100);
  lcd.print("o");
  delay(100);
  lcd.print("m");
  delay(100);

  lcd.setCursor(7, 1);
  lcd.print("K");
  delay(100);
  lcd.print("u");
  delay(100);
  lcd.print("l");
  delay(100);
  lcd.print("i");
  delay(100);
  lcd.print("C");
  delay(100);
  lcd.print("l");
  delay(100);
  lcd.print("o");
  delay(100);
  lcd.print("c");
  delay(100);
  lcd.print("k");

  lcd.noCursor();
  delay(19000);
  lcd.clear();
  delay(1000);
}
void ReadDHT()
{
  if ((millis() - TimerDHT) >= 3000)
  {
    TimerDHT = millis();
    Hum = dht.readHumidity();
    Temp = dht.readTemperature();
    if (Flag_HumTemp == true)
    {
      lcd.setCursor(7, 0);
      lcd.print(Hum);

      lcd.setCursor(7, 1);
      lcd.print(Temp);
    }
  }
}
void Time()
{
  if (Iterations == 0)
  {
    lcd.clear();
    delay(300);

    lcd.setCursor(2, 0);
    lcd.print("Left to work");
  }
  // Цикл математики времени ???
  // Тут будут часы которые постоянно обновляются
  Iterations++;
}
void HumTemp()
{
  if (Iterations == 0)
  {
    lcd.clear();
    delay(300);

    lcd.setCursor(0, 0);
    lcd.print("Hum: ");
    lcd.setCursor(14, 0);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.setCursor(14, 1);
    lcd.print("C");

    lcd.setCursor(7, 0);
    lcd.print(Hum);

    lcd.setCursor(7, 1);
    lcd.print(Temp);
  }
  // Serial.print("Важность: ");
  // Serial.print(Hum);
  // Serial.print("   Температура: ");
  // Serial.println(Temp);

  Iterations++;
}