#include <Arduino.h>
// *************************** DHT22 ******************************
#include <Adafruit_Sensor.h>
#include "DHT.h"
// *************************** LCD1602 ****************************
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// **************************** DS3231 *****************************
#include <microDS3231.h>

#define DHTPIN 2      // пин DHT22
#define DHTTYPE DHT22 // тип датчика DHT 22  (AM2302), AM2321

uint8_t Hum = 0; // Переменная влажности
float Temp = 0;  // Переменная температуры

uint8_t Hours = 0;   // Часы
uint8_t Minutes = 0; // Минуты
uint8_t Seconds = 0; // Секунды
uint8_t Date = 0;    // Число
uint8_t Month = 0;   // Месяц
uint16_t Year = 0;   // Год
uint8_t Day = 0;     // День недели Пн...Вс (1...7)

uint8_t LeftHourse = 0;  // Осталось работать часов
uint8_t LeftMinutes = 0; // Осталось работать минут
uint8_t LeftSeconds = 0; // Осталось работать секунд

uint32_t TimerDHT = 0;
uint32_t TimerFlag = 0;
uint32_t TimerDS3231 = 0;
uint16_t MyPeriod = 30000;

uint32_t Iterations = 0; // Счетчик количества итераций (сбрасывается при смене флажков)

bool Flag_Time = true; // Флажки
bool Flag_HumTemp = false;

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
MicroDS3231 rtc; // по умолчанию адрес 0x68

void Start();       // Приветствие
void ReadDHT();     // Чтение DHT22
void Time();        // Left to work (Осталось работать)
void HumTemp();     // HumTemp (Влажность и температура)
void ReadTime();    // Опрос датчика DS3231
void MathTime();    // Математика оставшегося рабочего времени
void WorkingTime(); // Рабочее время

void setup()
{
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  rtc.begin();
  Start();
  TimerFlag = millis();
}

void loop()
{
  ReadTime();
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
  if ((Hours >= 9) && (Hours < 17)) // Рабочее время
  {
    WorkingTime();
  }
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
void ReadTime()
{
  if ((millis() - TimerDS3231) >= 200)
  {
    TimerDS3231 = millis();
    Hours = rtc.getHours();
    Minutes = rtc.getMinutes();
    Seconds = rtc.getSeconds();
    Date = rtc.getDate();
    Month = rtc.getMonth();
    Year = rtc.getYear();
    Day = rtc.getDay();
  }
}
void WorkingTime()
{
  if (Iterations == 0)
  {
    lcd.clear();
    delay(300);

    lcd.setCursor(2, 0);
    lcd.print("Left to work");
  }
  MathTime();
  Iterations++;
}
void MathTime()
{
  LeftHourse = 17 - (Hours + 1);
  LeftMinutes = 59 - Minutes;
  LeftSeconds = 59 - Seconds;
  // Принты на дисплей будут тут
}