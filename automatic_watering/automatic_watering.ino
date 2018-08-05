#include <EEPROM.h>
#include "TM1637.h"
#include <iarduino_RTC.h>
#include <MsTimer2.h>
////////////Settings///////////////
//analog pin
#define Wetlavelnow map (analogRead(0), 280, 580, 100, 0) // датчик влажности
#define WetlavelEdit map (analogRead(1), 0, 1023, 0, 100) //потенциометр
//digit pin
#define DispPower 4 //питание дисплея
#define pomp 5 //помпа
#define Button 6 //кнопка режима настройки
#define WetlavelEditPower 7 //питание патенциометра
#define WetsensorPower 8 //подача питания на датчик влажности
//EEPROM memory
#define keeper 1008 //Сторож первого запуска
#define countlog word(EEPROM.read(1010),EEPROM.read(1011)) // размер лога
#define Wetlavelmin 1009 // минимальный уровень влажности
//дата начала измерений
#define TimeSensorHourStart 1000 //Час в памяти
#define TimeSensorDaysStart 1001 //День в памяти
#define TimeSensorMonthStart 1002 // Месяц в памяти
#define TimeSensorYearStart 1003 // Год в памяти
#define TimeSensorHourLast 1004 //Час последней записи в памяти

TM1637 tm1637(3, 2); //Создаём объект класса TM1637, в качестве параметров передаём номера пинов подключения
iarduino_RTC time(RTC_DS1307);
void setup()
{
  pinMode(Wetlavelnow, INPUT);
  pinMode(WetsensorPower, OUTPUT);
  pinMode(WetlavelEditPower, OUTPUT);
  pinMode(Button, INPUT_PULLUP);
  pinMode(pomp, OUTPUT);
  pinMode(DispPower, OUTPUT);
  digitalWrite(DispPower, HIGH);
  Serial.begin(9600);
  MsTimer2::set(100, SerialReadTimer); // задаем период прерывания по таймеру 100 мс
  MsTimer2::start();
  time.begin();
  time.period(1);
  tm1637.init();
  tm1637.set(BRIGHT_DARKEST);
  int val = EEPROM.read(countlog);
  tm1637.display(val);
  if (!EEPROM.read(Wetlavelmin)) EEPROM.update(Wetlavelmin, 60);//минимальный уровень влажности при не установленном вручную
  if (!EEPROM.read(keeper))
  {
    time.gettime();
    EEPROM.update(TimeSensorHourStart, time.Hours);
    EEPROM.update(TimeSensorDaysStart, time.day);
    EEPROM.update(TimeSensorMonthStart, time.month);
    EEPROM.update(TimeSensorYearStart, time.year);

    EEPROM.update(keeper, 1);
    CountLogValue(-1); //начальное значение
    digitalWrite(WetsensorPower, HIGH);
    timerDelay(5000);
    EEPROMwrite();
    digitalWrite(WetsensorPower, LOW);
  }
}

void(* resetFunc) (void) = 0;//перезагрузка

void loop()
{
  time.gettime();
  if (time.Hours != EEPROM.read(TimeSensorHourLast)) {
    MsTimer2::stop();
    digitalWrite(WetsensorPower, HIGH);
    timerDelay(5000);
    if (Wetlavelnow < EEPROM.read(Wetlavelmin))
      if (analize())
        watering (); //полив
    EEPROMwrite();
    digitalWrite(WetsensorPower, LOW);
    MsTimer2::start();
  }
}
void EEPROMwrite()
{
  time.gettime();
  unsigned short addr = countlog + 1;
  if (addr > 999) reStart(); //Переполнение памяти EEPROM
  EEPROM.update(addr, Wetlavelnow);
  CountLogValue(addr);
  EEPROM.update(TimeSensorHourLast, time.Hours);
  int val = EEPROM.read(addr);
  tm1637.display(val);
}
void EEPROMread(unsigned short ind)
{
  Serial.println("****** Read start ******");
  Serial.print(EEPROM.read(TimeSensorHourStart));
  Serial.print(" Hours : ");
  Serial.print(EEPROM.read(TimeSensorDaysStart));
  Serial.print(" Days : ");
  Serial.print(EEPROM.read(TimeSensorMonthStart));
  Serial.println(" Month");
  for (unsigned short i = 0; i <= ind; i++)
  {
    Serial.print("value");
    Serial.print("[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(EEPROM.read(i));
  }
}
void reStart()
{
  EEPROM.update(keeper, 0);
  resetFunc();
}
void EEPROMclear(unsigned short ind)
{
  for (unsigned short i = 0; i < ind; i++)
    EEPROM.update(i, 0);
  reStart();
}
void WetlavelEditor ()
{
  digitalWrite(WetlavelEditPower, HIGH);
  digitalWrite(WetsensorPower, HIGH);
  digitalWrite(pomp, LOW);
  tm1637.point(true);
  while (digitalRead(Button) == LOW)
  {
    byte sensVal = constrain(WetlavelEdit, 0, 100);
    tm1637.display(sensVal);
    delay(10000);
  }
  byte sensVal = constrain(WetlavelEdit, 0, 100);
  EEPROM.update(Wetlavelmin, sensVal );
  tm1637.point(false);
  digitalWrite(WetlavelEditPower, LOW);
  int val = EEPROM.read(countlog);
  tm1637.display(val);
}
void watering ()
{
  digitalWrite(pomp, HIGH);
  timerDelay(4000);
  digitalWrite(pomp, LOW);
}
bool analize ()
{
  if (Wetlavelnow <= 100)
    return 1;
  else return 0;
}
void timerDelay(unsigned short t)
{
  unsigned long ts = millis();
  while (1) {
    unsigned long currentMillis = millis();
    if (currentMillis - ts > t)break;
  }
}
void CountLogValue (short value)
{
  value = constrain(value, -1, 999); //диапазон лога
  EEPROM.update(1010, highByte(value));
  EEPROM.update(1011, lowByte(value));
}
void SerialReadTimer()
{
  if (digitalRead(Button) == LOW) WetlavelEditor();
  if (Serial.available() > 0)
  {
    byte event = Serial.read();
    if (event == 'h') help();
    else if (event == 'R') resetFunc();
    else if (event == 'r') EEPROMread(countlog);
    else if (event == 'a') EEPROMread(1023);
    else if (event == 'c') EEPROMclear(countlog);
    else if (event == 'C') EEPROMclear(1023);
    else if (event == 's') reStart();
    else
    {
      Serial.println("this command does not exist");
      Serial.println("enter h for help");
    }
  }
}
void help()
{
  Serial.println("****** HELP ******");
  Serial.println("r - reading data in the memory");
  Serial.println("a - read all");
  Serial.println("c - clear the memory");
  Serial.println("C - clear all the memory");
  Serial.println("h - help");
  Serial.println("s - Reset");
  Serial.println("R - Restart");
}
