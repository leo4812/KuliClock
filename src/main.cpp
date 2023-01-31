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

#define PinBuzzer 3 // Пин пассивного зуммера

uint8_t Hum = 0; // Переменная влажности
float Temp = 0;  // Переменная температуры

uint8_t Hours = 0;   // Часы
uint8_t Minutes = 0; // Минуты
uint8_t Seconds = 0; // Секунды
uint8_t Date = 0;    // Число
uint8_t Month = 0;   // Месяц
uint16_t Year = 0;   // Год
uint8_t Day = 0;     // День недели пн...пт (1...7)

uint8_t LeftHourse = 0;  // Осталось работать часов
uint8_t LeftMinutes = 0; // Осталось работать минут
uint8_t LeftSeconds = 0; // Осталось работать секунд

uint32_t TimerDHT = 0;
uint32_t TimerFlag = 0;
uint32_t TimerDS3231 = 0;
uint16_t MyPeriod = 25000;

uint32_t Iterations = 0; // Счетчик количества итераций (сбрасывается при смене флажков)

bool Flag_Time = true; // Флажки
bool Flag_HumTemp = false;

bool CountdownFlag = false; // Флажок для функции обратного отсчета
bool Second10 = false;
bool Second9 = false;
bool Second8 = false;
bool Second7 = false;
bool Second6 = false;
bool Second5 = false;
bool Second4 = false;
bool Second3 = false;
bool Second2 = false;
bool Second1 = false;

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
MicroDS3231 rtc; // по умолчанию адрес 0x68

void Start();          // Приветствие
void ReadDHT();        // Чтение DHT22
void Time();           // Left to work (Осталось работать)
void HumTemp();        // HumTemp (влажность и температура)
void ReadTime();       // Опрос датчика DS3231
void MathTime();       // Математика оставшегося рабочего времени
void WorkingTime();    // Рабочее время
void Weekend();        // Выходные
void PreWorkingDay();  // Рабочий день еще не начался
void PostWorkingDay(); // Рабочий день закончен
void Countdown();      // Обратный осчет секунд до конца рабочего дня
void ShortSignal();    // Короткий звуковой сигнал
void LongSignal();     // Длинный звуковой сигнал
void GoHome();         // Вызывается ровно в 17:00 после длинного звукового сигнала
void Music();          // Функция музыки (управляет выбором музыки)

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
    TimerFlag = millis(); // ЕСЛИ ДОБАВЛЯТЬ СЮДА НОВЫЕ ФЛАГИ, ТО ДОБАВИТЬ ЕЩЕ И В GoHome() в состояние false
    Iterations = 0;
    if (Flag_Time == true) // Условие включает темп. влаж.
    {
      Flag_Time = false;
      Flag_HumTemp = true;
      // Тут условие времени меняющее MyPeriod
    }
    else if (Flag_HumTemp == true) // Условие включает Таймер раб. времени
    {
      Flag_Time = true;
      Flag_HumTemp = false;
      // Тут условие времени меняющее MyPeriod
    }
  }
  //*************************************************
  if ((Hours == 16) && (Minutes == 59))
  {
    Countdown();
  }
  else
  {
    if (CountdownFlag == true) // Сброс флагов обратного отсчета
    {
      CountdownFlag = false;
      Second10 = false;
      Second9 = false;
      Second8 = false;
      Second7 = false;
      Second6 = false;
      Second5 = false;
      Second4 = false;
      Second3 = false;
      Second2 = false;
      Second1 = false;
    }

    if (Flag_Time == true)
    {
      Time();
    }
    if (Flag_HumTemp == true)
    {
      HumTemp();
    }
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
  }
}
void Time()
{
  if (Day <= 5) // День недели (с понедельника по пятницу)
  {
    if ((Hours >= 0) && (Hours < 9)) // с 0:00 до 9:00
    {
      PreWorkingDay();
    }
    else if ((Hours >= 9) && (Hours < 17)) // Рабочее время
    {
      WorkingTime();
    }
    else if ((Hours >= 17) && (Hours < 24)) // с 17:00 до 24:00
    {
      PostWorkingDay();
    }
  }
  else // Выходные
  {
    Weekend();
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
  lcd.setCursor(7, 0);
  lcd.print(Hum);

  lcd.setCursor(7, 1);
  lcd.print(Temp);
  // Serial.print("Влажность: ");
  // Serial.print(Hum);
  // Serial.print("   “Температура: ");
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

    lcd.setCursor(6, 1);
    lcd.print(":");
    lcd.setCursor(9, 1);
    lcd.print(":");
  }
  MathTime();
  Iterations++;
}
void MathTime()
{
  LeftHourse = 17 - (Hours + 1);
  LeftMinutes = 59 - Minutes;
  LeftSeconds = 59 - Seconds;
  if (LeftHourse < 10) // Часы
  {
    lcd.setCursor(4, 1);
    lcd.print("0");
    lcd.print(LeftHourse);
  }
  else
  {
    lcd.setCursor(4, 1);
    lcd.print(LeftHourse);
  }

  if (LeftMinutes < 10) // Минуты
  {
    lcd.setCursor(7, 1);
    lcd.print("0");
    lcd.print(LeftMinutes);
  }
  else
  {
    lcd.setCursor(7, 1);
    lcd.print(LeftMinutes);
  }

  if (LeftSeconds < 10) // Секунды
  {
    lcd.setCursor(10, 1);
    lcd.print("0");
    lcd.print(LeftSeconds);
  }
  else
  {
    lcd.setCursor(10, 1);
    lcd.print(LeftSeconds);
  }
}
void Weekend()
{
  if (Iterations == 0)
  {
    lcd.clear();
    delay(300);

    lcd.setCursor(4, 0);
    lcd.print("Weekend");
    if (Day == 6) // Суббота
    {
      lcd.setCursor(4, 1);
      lcd.print("Saturday");
    }
    else // Воскресенье
    {
      lcd.setCursor(5, 1);
      lcd.print("Sunday");
    }
  }
  Iterations++;
}
void PreWorkingDay()
{
  if (Iterations == 0)
  {
    lcd.clear();
    delay(300);

    lcd.setCursor(0, 0);
    lcd.print("The work day");
    lcd.setCursor(0, 1);
    lcd.print("has not Started");
  }
  Iterations++;
}
void PostWorkingDay()
{
  if (Iterations == 0)
  {
    lcd.clear();
    delay(300);

    lcd.setCursor(0, 0);
    lcd.print("Working day");
    lcd.setCursor(8, 1);
    lcd.print("is Over!");
  }
  Iterations++;
}
void Countdown()
{
  if (CountdownFlag == false)
  {
    lcd.clear();
    delay(300);

    lcd.setCursor(0, 0);
    lcd.print("Over of work in:");
  }
  LeftSeconds = 59 - Seconds;
  if (LeftSeconds < 10) // Секунды
  {
    lcd.setCursor(7, 1);
    lcd.print("0");
    lcd.print(LeftSeconds);
  }
  else
  {
    lcd.setCursor(7, 1);
    lcd.print(LeftSeconds);
  }
  switch (LeftSeconds)
  {
  case 10:
    if (Second10 == false)
    {
      ShortSignal();
    }
    Second10 = true;
    break;
  case 9:
    if (Second9 == false)
    {
      ShortSignal();
    }
    Second9 = true;
    break;
  case 8:
    if (Second8 == false)
    {
      ShortSignal();
    }
    Second8 = true;
    break;
  case 7:
    if (Second7 == false)
    {
      ShortSignal();
    }
    Second7 = true;
    break;
  case 6:
    if (Second6 == false)
    {
      ShortSignal();
    }
    Second6 = true;
    break;
  case 5:
    if (Second5 == false)
    {
      ShortSignal();
    }
    Second5 = true;
    break;
  case 4:
    if (Second4 == false)
    {
      ShortSignal();
    }
    Second4 = true;
    break;
  case 3:
    if (Second3 == false)
    {
      ShortSignal();
    }
    Second3 = true;
    break;
  case 2:
    if (Second2 == false)
    {
      ShortSignal();
    }
    Second2 = true;
    break;
  case 1:
    if (Second1 == false)
    {
      LongSignal();
      GoHome();
    }
    Second1 = true;
    break;
  }
  CountdownFlag = true;
}
void ShortSignal()
{
  tone(PinBuzzer, 1000);
  delay(200);
  noTone(PinBuzzer);
}
void LongSignal()
{
  tone(PinBuzzer, 1000);
  delay(900);
  noTone(PinBuzzer);
}
void GoHome()
{
  lcd.clear();
  delay(300);

  lcd.setCursor(0, 0);
  lcd.print("Working day");
  lcd.setCursor(8, 1);
  lcd.print("is Over!");
  delay(1500);
  Music();
  TimerFlag = millis();
  Flag_Time = false;
  Flag_HumTemp = true; // *************** НОВЫЕ ФЛАГИ СЮДА!!!!!!! *********************
}
void Music()
{
  // ****************** ТУТ БУДЕТ ВЫЗЫВАТЬСЯ РАЗНАЯ МУЗЫКИ ************************
}