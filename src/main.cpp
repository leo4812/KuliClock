#include <Arduino.h>
// *************************** DHT22 ******************************
#include <Adafruit_Sensor.h>
#include "DHT.h"
// *************************** LCD1602 ****************************
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// **************************** DS3231 ****************************
#include <microDS3231.h>
// **************************** BMP180 ****************************
#include <SPI.h>
#include <Adafruit_BMP085.h>
// ***************************** Noty *****************************
#include "pitches.h"
// **************************** EEPROM ****************************
#include <EEPROM.h>

#define DHTPIN 2      // пин DHT22
#define DHTTYPE DHT22 // тип датчика DHT 22  (AM2302), AM2321

#define PinBuzzer 3 // Пин пассивного зуммера

int16_t Hum = 0; // Переменная влажности
float Temp = 0;  // Переменная температуры

uint8_t Minuty_so_starta = 0; // Минуты со старта программы
uint8_t Chasy_so_starta = 0;  // Часы со старта программы (В EEPROMе 0 байт)
uint16_t Dni_so_starta = 0;   // Дни со старта программы (В EEPROMе 1-2 байт)

uint16_t STUCK_DHT22 = 0;       // Количество раз которое датчик DHT22 завис (В EEPROMе 3-4 байт)
uint32_t STUCK_DHT22_Timer = 0; // Таймер для цикла KuliClockWork

float Temp_BMP180 = 0;     // Переменная температуры с датчика BMP180
uint32_t Press_BMP180 = 0; // Переменная давления с датчика BMP180

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
uint32_t TimerBMP180 = 0;
uint32_t Timer_minut = 0;
uint16_t MyPeriod = 25000;

uint32_t Iterations = 0;      // Счетчик количества итераций (сбрасывается при смене флажков)
uint32_t IterationsError = 0; // Счетчик количества итераций циклов ошибки (сбрасывается при смене флажков)

bool Flag_Time = true; // Флажки
bool Flag_HumTemp = false;
bool Flag_RealTime = false;
bool Flag_Pressure = false;
bool Flag_soStarta = false;

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

bool StartWorkingTime = true; // Флажок для фанфар при старте рабочего дня

bool ErrorDHT22 = false;     // Флаг ошибки датчика температуры и влажности
bool ErrorDS3231 = false;    // Флаг ошибки часов реального времени
bool ErrorBMP180 = false;    // Флаг ошибки датчика давления
bool BackLightNight = false; // Подсветка ночь
bool BackLightDay = false;   // Подсветка день

int16_t TMP_Hum = 0;          // Промежуточная переменная влажности для ошибки датчика
float TMP_Temp = 0;           // Промежуточная переменная температуры для ошибки датчика
uint32_t TimerErrorDHT22 = 0; // Таймер ошибки датчика DHT22

uint32_t TMPPress_BMP180 = 0;  // Промежуточная переменная давления для ошибки датчика
uint32_t TimerErrorBMP180 = 0; // Таймер ошибки датчика BMP180

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
MicroDS3231 rtc; // по умолчанию адрес 0x68
Adafruit_BMP085 bmp;

void Start();                // Приветствие
void ReadDHT();              // Чтение DHT22
void Time();                 // Left to work (Осталось работать)
void HumTemp();              // HumTemp (влажность и температура)
void ReadTime();             // Опрос датчика DS3231
void MathTime();             // Математика оставшегося рабочего времени
void WorkingTime();          // Рабочее время
void Weekend();              // Выходные
void PreWorkingDay();        // Рабочий день еще не начался
void PostWorkingDay();       // Рабочий день закончен
void Countdown();            // Обратный осчет секунд до конца рабочего дня
void ShortSignal();          // Короткий звуковой сигнал
void LongSignal();           // Длинный звуковой сигнал
void GoHome();               // Вызывается ровно в 17:00 после длинного звукового сигнала
void Music();                // Функция музыки (управляет выбором музыки)
void RealTime();             // Функция отрисовки реального времени и даты
void ReadPressure();         // Функция опроса датчика давления
void Pressure();             // Функция вызова математики и отрисовки давления
void YaSvoboden();           // Песня Я Свободен!
void Fanfary();              // Приветственные фанфары
void PoraDomoy();            // Песня Пора домой!
void ReadMinuty();           // Цикл счетчика минут со старта программы
void KuliClockWork();        // Цикл количества дней и часов со старта часов
void (*resetFunc)(void) = 0; // объявляем функцию reset

void setup()
{
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  rtc.begin();
  bmp.begin();
  EEPROM.get(0, Chasy_so_starta);
  EEPROM.get(1, Dni_so_starta);
  EEPROM.get(3, STUCK_DHT22);
  Start();
  TimerFlag = millis();
}

void loop()
{
  ReadTime();
  ReadPressure();
  ReadMinuty();
  //*************************************************
  if ((ErrorDHT22 == true) || (ErrorDS3231 == true) || (ErrorBMP180 == true))
  {
    lcd.backlight();
    BackLightNight = false;
    BackLightDay = false;
  }
  else
  {
    if ((Hours >= 1) && (Hours <= 7))
    {
      if (BackLightNight == false)
      {
        lcd.noBacklight();
        BackLightNight = true;
        BackLightDay = false;
      }
    }
    else
    {
      if (BackLightDay == false)
      {
        lcd.backlight();
        BackLightNight = false;
        BackLightDay = true;
      }
    }
  }
  //************************************************
  if ((millis() - TimerFlag) >= MyPeriod)
  {
    TimerFlag = millis(); // ЕСЛИ ДОБАВЛЯТЬ СЮДА НОВЫЕ ФЛАГИ, ТО ДОБАВИТЬ ЕЩЕ И В GoHome() в состояние false
    Iterations = 0;
    IterationsError = 0;
    if (Flag_Time == true) // Период Темп Влаж
    {
      Flag_Time = false;
      Flag_HumTemp = true;
      Flag_RealTime = false;
      Flag_Pressure = false;
      Flag_soStarta = false;
      if ((Day <= 5) && (Hours >= 9) && (Hours < 17))
      {
        MyPeriod = 20000;
      }
      else
      {
        MyPeriod = 25000;
      }
    }
    else if (Flag_HumTemp == true) // Период Даты Время
    {
      Flag_Time = false;
      Flag_HumTemp = false;
      Flag_RealTime = true;
      Flag_Pressure = false;
      Flag_soStarta = false;
      if ((Day <= 5) && (Hours >= 9) && (Hours < 17))
      {
        MyPeriod = 10000;
      }
      else
      {
        MyPeriod = 15000;
      }
    }
    else if (Flag_RealTime == true) // Период Давления
    {
      Flag_Time = false;
      Flag_HumTemp = false;
      Flag_RealTime = false;
      Flag_Pressure = true;
      Flag_soStarta = false;
      if ((Day <= 5) && (Hours >= 9) && (Hours < 17))
      {
        MyPeriod = 10000;
      }
      else
      {
        MyPeriod = 15000;
      }
    }
    else if (Flag_Pressure == true) // Период отсчета сколько работает программа
    {
      Flag_Time = false;
      Flag_HumTemp = false;
      Flag_RealTime = false;
      Flag_Pressure = false;
      Flag_soStarta = true;
      if ((Day <= 5) && (Hours >= 9) && (Hours < 17))
      {
        MyPeriod = 14000;
      }
      else
      {
        MyPeriod = 14000;
      }
    }
    else if (Flag_soStarta == true) // Период Рабочего времени
    {
      Flag_Time = true;
      Flag_HumTemp = false;
      Flag_RealTime = false;
      Flag_Pressure = false;
      Flag_soStarta = false;
      if ((Day <= 5) && (Hours >= 9) && (Hours < 17))
      {
        MyPeriod = 25000;
      }
      else
      {
        MyPeriod = 10000;
      }
    }
  }
  //*************************************************
  if ((Day <= 5) && (Hours == 16) && (Minutes == 59))
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
    else if (Flag_HumTemp == true)
    {
      HumTemp();
    }
    else if (Flag_RealTime == true)
    {
      RealTime();
    }
    else if (Flag_Pressure == true)
    {
      Pressure();
    }
    else if (Flag_soStarta == true)
    {
      KuliClockWork();
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
  Fanfary();
  delay(14000);
  lcd.clear();
  delay(1000);
}
void ReadDHT()
{
  if ((millis() - TimerDHT) >= 6000) // Каждые 6 секунды
  {
    TimerDHT = millis();
    Hum = dht.readHumidity();
    Temp = dht.readTemperature();
    if ((Temp == TMP_Temp) && (Hum == TMP_Hum))
    {
      if ((Day <= 5) && (Hours >= 9) && (Hours < 18))
      {
        if ((millis() - TimerErrorDHT22) >= 5400000)
        {
          ErrorDHT22 = true;
        }
      }
      else
      {
        if ((millis() - TimerErrorDHT22) >= 10800000)
        {
          ErrorDHT22 = true;
        }
      }
    }
    else
    {
      TimerErrorDHT22 = millis();
      TMP_Temp = Temp;
      TMP_Hum = Hum;
      ErrorDHT22 = false;
    }
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
  ReadDHT();
  if (ErrorDHT22 == true)
  {
    if (IterationsError == 0)
    {
      lcd.backlight(); // включаем подсветку дисплея
      lcd.clear();
      delay(300);
      lcd.setCursor(0, 0);
      lcd.print("Sensor DHT22");
      lcd.setCursor(0, 1);
      lcd.print("STUCK");
    }
    IterationsError++;
    LongSignal();
    STUCK_DHT22++;
    delay(10000);
    EEPROM.put(3, STUCK_DHT22);
    delay(5000);
    resetFunc(); // вызываем reset
  }
  else
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
    Iterations++;
  }
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
    if (Year <= 2022)
    {
      ErrorDS3231 = true;
    }
    else
    {
      ErrorDS3231 = false;
    }
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
  if (StartWorkingTime == false)
  {
    StartWorkingTime = true;
    Fanfary();
  }
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
  StartWorkingTime = false;
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
  StartWorkingTime = false;
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
  delay(1500);
  TimerFlag = millis();
  MyPeriod = 25000;
  Iterations = 0;
  Flag_Time = false;
  Flag_HumTemp = true; // *************** НОВЫЕ ФЛАГИ СЮДА!!!!!!! *********************
  Flag_RealTime = false;
  Flag_Pressure = false;
  Flag_soStarta = false;
}
void Music()
{
  // ****************** ТУТ БУДЕТ ВЫЗЫВАТЬСЯ РАЗНАЯ МУЗЫКИ ************************
  // YaSvoboden();
  PoraDomoy();
}
void RealTime()
{
  if (Iterations == 0)
  {
    lcd.clear();
    delay(300);

    lcd.setCursor(0, 0);
    if (Date < 10) // Дата
    {
      lcd.setCursor(0, 0);
      lcd.print("0");
      lcd.print(Date);
    }
    else
    {
      lcd.setCursor(0, 0);
      lcd.print(Date);
    }

    lcd.setCursor(3, 0);

    switch (Month)
    {
    case 1:
      lcd.print("Jan"); // Январь
      break;
    case 2:
      lcd.print("Feb"); // Февраль
      break;
    case 3:
      lcd.print("Mar"); // Март
      break;
    case 4:
      lcd.print("Apr"); // Апрель
      break;
    case 5:
      lcd.print("May"); // Май
      break;
    case 6:
      lcd.print("Jun"); // Июнь
      break;
    case 7:
      lcd.print("Jul"); // Июль
      break;
    case 8:
      lcd.print("Aug"); // Август
      break;
    case 9:
      lcd.print("Sep"); // Сентябрь
      break;
    case 10:
      lcd.print("Oct"); // Октябрь
      break;
    case 11:
      lcd.print("Nov"); // Ноябрь
      break;
    case 12:
      lcd.print("Dec"); // Декабрь
      break;
    }
    lcd.setCursor(7, 0);
    lcd.print(Year);
    lcd.print(",");
    lcd.setCursor(13, 0);
    switch (Day)
    {
    case 1:
      lcd.print("Mon"); // Понедельник
      break;
    case 2:
      lcd.print("Tue"); // Вторник
      break;
    case 3:
      lcd.print("Wed"); // Среда
      break;
    case 4:
      lcd.print("Tho"); // Четверг
      break;
    case 5:
      lcd.print("Fri"); // Пятница
      break;
    case 6:
      lcd.print("Sat"); // Суббота
      break;
    case 7:
      lcd.print("Sun"); // Воскресенье
      break;
    }
    lcd.setCursor(6, 1);
    lcd.print(":");
    lcd.setCursor(9, 1);
    lcd.print(":");
  }
  if (ErrorDS3231 == true)
  {
    lcd.setCursor(0, 1);
    lcd.print("Check battery!");
  }
  else
  {
    if (Hours < 10) // Часы
    {
      lcd.setCursor(4, 1);
      lcd.print("0");
      lcd.print(Hours);
    }
    else
    {
      lcd.setCursor(4, 1);
      lcd.print(Hours);
    }

    if (Minutes < 10) // Минуты
    {
      lcd.setCursor(7, 1);
      lcd.print("0");
      lcd.print(Minutes);
    }
    else
    {
      lcd.setCursor(7, 1);
      lcd.print(Minutes);
    }

    if (Seconds < 10) // Секунды
    {
      lcd.setCursor(10, 1);
      lcd.print("0");
      lcd.print(Seconds);
    }
    else
    {
      lcd.setCursor(10, 1);
      lcd.print(Seconds);
    }
  }
  Iterations++;
}
void ReadPressure()
{
  if ((millis() - TimerBMP180) >= 30000)
  {
    TimerBMP180 = millis();
    Temp_BMP180 = bmp.readTemperature();
    Press_BMP180 = bmp.readPressure();
    if (Press_BMP180 == TMPPress_BMP180)
    {
      if ((millis() - TimerErrorBMP180) >= 5400000)
      {
        ErrorBMP180 = true;
      }
    }
    else
    {
      TimerErrorBMP180 = millis();
      TMPPress_BMP180 = Press_BMP180;
      ErrorBMP180 = false;
    }
  }
}
void Pressure()
{
  if (ErrorBMP180 == true)
  {
    if (IterationsError == 0)
    {
      lcd.clear();
      delay(300);
      lcd.setCursor(0, 0);
      lcd.print("Sensor BMP180");
      lcd.setCursor(0, 1);
      lcd.print("STUCK");
    }
    IterationsError++;
  }
  else
  {
    float PressMM = ((float)Press_BMP180 * 0.00750);
    uint16_t PressMMprint = PressMM;
    if (Iterations == 0)
    {
      lcd.clear();
      delay(300);

      lcd.setCursor(0, 0);
      lcd.print("Press:");
      lcd.setCursor(14, 0);
      lcd.print("mm");

      lcd.setCursor(0, 1);
      lcd.print("Press:");
      lcd.setCursor(14, 1);
      lcd.print("Pa");
    }
    lcd.setCursor(7, 0);
    lcd.print(PressMMprint);
    lcd.setCursor(7, 1);
    lcd.print(Press_BMP180);
    Iterations++;
  }
}
void YaSvoboden()
{
  tone(PinBuzzer, NOTE_DS4, 300); // Ре#
  delay(500);
  tone(PinBuzzer, NOTE_DS4, 180); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 180); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_G4, 400); // Соль
  delay(750);

  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_D4, 220); // Ре
  delay(300);
  tone(PinBuzzer, NOTE_C4, 180); // До
  delay(300);
  tone(PinBuzzer, NOTE_AS3, 180); // Ля#3
  delay(300);
  tone(PinBuzzer, NOTE_C4, 450); // До
  delay(800);

  tone(PinBuzzer, NOTE_DS4, 300); // Ре#
  delay(500);
  tone(PinBuzzer, NOTE_DS4, 180); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 180); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_G4, 400); // Соль
  delay(750);

  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_D4, 220); // Ре
  delay(300);
  tone(PinBuzzer, NOTE_C4, 210); // До
  delay(300);
  tone(PinBuzzer, NOTE_AS3, 210); // Ля#3
  delay(300);
  tone(PinBuzzer, NOTE_C4, 450); // До
  delay(800);

  tone(PinBuzzer, NOTE_DS4, 300); // Ре#
  delay(500);
  tone(PinBuzzer, NOTE_DS4, 180); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 180); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_G4, 400); // Соль
  delay(750);

  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_D4, 220); // Ре
  delay(300);
  tone(PinBuzzer, NOTE_C4, 180); // До
  delay(300);
  tone(PinBuzzer, NOTE_AS3, 180); // Ля#3
  delay(300);
  tone(PinBuzzer, NOTE_C4, 450); // До
  delay(800);

  tone(PinBuzzer, NOTE_DS4, 300); // Ре#
  delay(500);
  tone(PinBuzzer, NOTE_DS4, 180); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 180); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_G4, 400); // Соль
  delay(750);

  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_DS4, 200); // Ре#
  delay(300);
  tone(PinBuzzer, NOTE_D4, 220); // Ре
  delay(300);
  tone(PinBuzzer, NOTE_C4, 210); // До
  delay(300);
  tone(PinBuzzer, NOTE_AS3, 210); // Ля#3
  delay(300);
  tone(PinBuzzer, NOTE_C4, 700); // До
  delay(1000);
  noTone(PinBuzzer);
  delay(1000);
}
void Fanfary()
{
  int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};

  int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};

  for (int thisNote = 0; thisNote < 8; thisNote++)
  {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(PinBuzzer, melody[thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(PinBuzzer);
  }
}
void PoraDomoy()
{
  tone(PinBuzzer, NOTE_D4); // Ре 4
  delay(150);
  noTone(PinBuzzer);
  delay(70);

  tone(PinBuzzer, NOTE_D4); // Ре 4
  delay(150);
  noTone(PinBuzzer);
  delay(70);

  tone(PinBuzzer, NOTE_D4); // Ре 4
  delay(150);
  noTone(PinBuzzer);
  delay(70);

  tone(PinBuzzer, NOTE_D4); // Ре 4
  delay(150);
  noTone(PinBuzzer);
  delay(70);

  tone(PinBuzzer, NOTE_D4); // Ре 4
  delay(250);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_C4); // До 4
  delay(250);
  noTone(PinBuzzer);
  delay(150);

  tone(PinBuzzer, NOTE_B3); // Си 3
  delay(150);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_A3); // Ля 3
  delay(250);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_G3); // Соль 3
  delay(350);
  noTone(PinBuzzer);
  delay(150);

  //******************************************

  tone(PinBuzzer, NOTE_G3); // Соль 3
  delay(150);
  noTone(PinBuzzer);
  delay(70);

  tone(PinBuzzer, NOTE_A3); // Ля 3
  delay(150);
  noTone(PinBuzzer);
  delay(70);

  tone(PinBuzzer, NOTE_B3); // Си 3
  delay(150);
  noTone(PinBuzzer);
  delay(70);

  tone(PinBuzzer, NOTE_C4); // До 4
  delay(400);
  noTone(PinBuzzer);
  delay(150);

  //*************************************

  tone(PinBuzzer, NOTE_C4); // До 4
  delay(150);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_C4); // До 4
  delay(150);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_B3); // Си 3
  delay(300);
  noTone(PinBuzzer);
  delay(200);

  tone(PinBuzzer, NOTE_B3); // Си 3
  delay(150);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_A3); // Ля 3
  delay(200);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_A3); // Ля 3
  delay(350);
  noTone(PinBuzzer);
  delay(250);

  //*****************************************

  tone(PinBuzzer, NOTE_B3); // Си 3
  delay(150);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_A3); // Ля 3
  delay(450);
  noTone(PinBuzzer);
  delay(450);

  tone(PinBuzzer, NOTE_A3); // Ля 3
  delay(200);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_B3); // Си 3
  delay(450);
  noTone(PinBuzzer);
  delay(300);

  tone(PinBuzzer, NOTE_B3); // Си 3
  delay(200);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_C4); // До 4
  delay(200);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_B3); // Си 3
  delay(200);
  noTone(PinBuzzer);
  delay(100);

  tone(PinBuzzer, NOTE_A3); // Ля 3
  delay(1000);
  noTone(PinBuzzer);
  delay(100);
}
void ReadMinuty()
{
  if ((millis() - Timer_minut) >= 60000)
  {
    Timer_minut = millis();
    Minuty_so_starta++;
    if (Minuty_so_starta >= 60)
    {
      Minuty_so_starta = 0;
      Chasy_so_starta++;
      EEPROM.put(0, Chasy_so_starta);
      EEPROM.put(1, Dni_so_starta);
    }
    if (Chasy_so_starta >= 24)
    {
      Chasy_so_starta = 0;
      Dni_so_starta++;
      EEPROM.put(0, Chasy_so_starta);
      EEPROM.put(1, Dni_so_starta);
    }
  }
}
void KuliClockWork()
{
  if (Iterations == 0)
  {
    STUCK_DHT22_Timer = millis();
    lcd.clear();
    delay(300);

    lcd.setCursor(0, 0);
    lcd.print("KuliClock Work:");
    lcd.setCursor(0, 1);
    lcd.print("Day:");
    if (Dni_so_starta > 99)
    {
      lcd.setCursor(4, 1);
      lcd.print(Dni_so_starta);
    }
    else
    {
      lcd.setCursor(5, 1);
      lcd.print(Dni_so_starta);
    }
    lcd.setCursor(8, 1);
    lcd.print("Hour:");
    lcd.setCursor(14, 1);
    lcd.print(Chasy_so_starta);
  }
  if (STUCK_DHT22 != 0)
  {
    if ((millis() - STUCK_DHT22_Timer) >= 8000)
    {
      STUCK_DHT22_Timer = millis();
      lcd.clear();
      delay(300);

      lcd.setCursor(2, 0);
      lcd.print("Sensor DHT22");
      lcd.setCursor(0, 1);
      lcd.print("stuck:");
      lcd.setCursor(7, 1);
      lcd.print(STUCK_DHT22);
      if (STUCK_DHT22 < 10)
      {
        lcd.setCursor(9, 1);
        lcd.print("onse");
      }
      else if (STUCK_DHT22 > 99)
      {
        lcd.setCursor(11, 1);
        lcd.print("onse");
      }
      else
      {
        lcd.setCursor(10, 1);
        lcd.print("onse");
      }
    }
  }
  Iterations++;
}