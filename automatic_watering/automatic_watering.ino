#include <EEPROM.h>
#include "TM1637.h"
#include <iarduino_RTC.h>
#include <MsTimer2.h>
////////////Settings///////////////
//analog pin
#define Wetlavelnow map (analogRead(0), 280, 580, 100, 0) // датчик влажности
//digit pin
#define DispPower 4 //питание дисплея
#define pomp 5 //помпа
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

#define CLK 3     
#define DIO 2
TM1637 tm1637(CLK, DIO);
iarduino_RTC time(RTC_DS1307);

void setup()
{
  pinMode(Wetlavelnow, INPUT);
  pinMode(WetsensorPower, OUTPUT);
  pinMode(pomp, OUTPUT);
  pinMode(DispPower, OUTPUT);
  digitalWrite(DispPower, HIGH);
  Serial.begin(9600);
  MsTimer2::set(500, SerialReadTimer); // задаем период прерывания по таймеру 100 мс
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
  Serial.print("Current date ");
  Serial.println(time.gettime("d-m-Y, H:i:s, D"));
  Serial.print("Required humidity ");
  Serial.println(EEPROM.read(Wetlavelmin));
  help();
  MsTimer2::start();
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

void watering ()
{
  digitalWrite(pomp, HIGH);
  timerDelay(4000);
  digitalWrite(pomp, LOW);
  Serial.println("=====");
}

void info()
{
  Serial.println(time.gettime("d-m-Y, H:i:s, D")); // выводим время
  Serial.print("Required humidity ");
  Serial.println(EEPROM.read(Wetlavelmin));
  Serial.print("Wetlavelnow ");
  Serial.println(EEPROM.read(countlog));

}
void MinWetLavelUSB()
{
  int val = Serial.parseInt();
  EEPROM.update(Wetlavelmin, constrain(val,0,100));
  Serial.println();
  info();

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
  if (Serial.available() > 0)
  {
    int event = Serial.read();
    switch (event)
    {
      case 49:
       help(); //help();
        break;
      case 50:
        EEPROMread(countlog);
        break;
      case 51:
        EEPROMread(1023);
        break;
      case 52:
        EEPROMclear(countlog);
        break;
      case 53:
        EEPROMclear(1023);
        break;
      case 54:
        reStart();
        break;
      case 55:
        info();
        break;
      case 56:
        resetFunc();
        break;
      case 57:
        MinWetLavelUSB();
        break;
      case 10:
        Serial.println("**********************");
        break;
      default:
        Serial.println("this command does not exist\n enter 1 for help");
    }
  }
}
void help()
{
  Serial.println("****** HELP ******");
  Serial.println("1 - help");
  Serial.println("2 - reading water lavel log");
  Serial.println("3 - read all");
  Serial.println("4 - clear water lavel log");
  Serial.println("5 - clear all the memory");
  Serial.println("6 - Restart"); //WARNING!!! CLEAR THE MEMORY
  Serial.println("7 - info");
  Serial.println("8 - Reset");
  Serial.println("9 - Required humidity");
}
