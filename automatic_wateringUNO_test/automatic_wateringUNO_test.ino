#include <EEPROM.h>
#include <MsTimer2.h>
#include <iarduino_RTC.h>
#define WetsensorPower 8 //подача питания на датчик влажности
#define TimeSensorHours 1000 //Час в памяти
#define TimeSensorDays 1001 //День в памяти
#define TimeSensorMonth 1002 // Месяц в памяти
#define keeper 1003 //Сторож перезаписи данных
#define countlog 1004 // размер лога
#define Hour 1005 // Час последней записи 
#define Wetlavelnow 0
#define Wetlavelmin 1006 // минимальный уровень влажности
#define WetlavelEditPower 7
#define Button 6
iarduino_RTC time(RTC_DS1307);
void EEPROMread();
void EEPROMwrite();
void EEPROMclear();
void switchTimer();
void help();
void WetlavelEdit();

void setup()
{
  pinMode(Wetlavelnow, INPUT);
  pinMode(WetsensorPower, OUTPUT);
  pinMode(WetlavelEditPower, OUTPUT);
  pinMode(Button, INPUT_PULLUP);
  pinMode(5, OUTPUT);
 // digitalWrite(Button, HIGH);
  //digitalWrite(5, HIGH);
  Serial.begin(9600);
  MsTimer2::set(100, switchTimer); // задаем период прерывания по таймеру 100 мс
  MsTimer2::start();
  time.begin();
  time.gettime();
  Serial.println("v1.5");
  Serial.println(time.gettime("d-m-Y, H:i:s, D"));
  Serial.println("enter h for help");
  if (EEPROM.read(keeper) == 0)
  {
    EEPROM.write(TimeSensorHours, time.Hours);
    EEPROM.write(TimeSensorDays, time.day);
    EEPROM.write(TimeSensorMonth, time.month);
    EEPROM.write(keeper, 1);
    EEPROM.write(countlog, 0);
    EEPROM.write(Hour, time.Hours);
  }
}
void(* resetFunc) (void) = 0;//объявляем функцию reset с адресом 0
void loop()
{
  time.gettime();
  if (time.Hours != EEPROM.read(Hour)) EEPROMwrite();
  delay(1800000);
}

void EEPROMwrite()
{
  byte addr = EEPROM.read(countlog), full = EEPROM.read(256);

  if (addr >= 255) //Переполнение
  {
    EEPROM.write(keeper, 0);
    full++;
    EEPROM.write(256, full);
    resetFunc();
  }
  time.gettime();
  Serial.println("****** Write start ******");
  Serial.print("value");
  Serial.print("[");
  Serial.print(addr);
  Serial.print("] = ");
  digitalWrite(WetsensorPower, HIGH);
  delay(5000);
  Serial.println(map (analogRead(Wetlavelnow), 0, 1023, 0, 255));
  EEPROM.write(addr, time.Hours);
  addr++;
  EEPROM.write(addr, map (analogRead(Wetlavelnow), 0, 1023, 0, 255));
  addr++;
  EEPROM.write(countlog, addr);
  EEPROM.write(Hour, time.Hours);
  digitalWrite(WetsensorPower, LOW);
  return 0;
}

void EEPROMread(unsigned short ind)
{
  unsigned short j = 1;
  Serial.println("****** Read start ******");
  Serial.print(EEPROM.read(TimeSensorHours));
  Serial.print(" Hours : ");
  Serial.print(EEPROM.read(TimeSensorDays));
  Serial.print(" Days : ");
  Serial.print(EEPROM.read(TimeSensorMonth));
  Serial.println(" Month");
  for (unsigned short i = 0; i < ind; i++)
  {
    if (i <= 255) {
      if (i % 2 != 0) //чередование времени и значений
      {
        Serial.print("Wetvalue");
        Serial.print("[");
        Serial.print(j);
        Serial.print("] = ");
        Serial.println(EEPROM.read(i));
        j++;
      }
      else
      {
        Serial.println("--------------");
        Serial.println("Hours");
        Serial.println(EEPROM.read(i));
        Serial.println("--------------");
      }
    }
    else
    {
      Serial.print("value");
      Serial.print("[");
      Serial.print(j);
      Serial.print("] = ");
      Serial.println(EEPROM.read(i));
      j++;
    }
  }
  return 0;
}

void EEPROMclear()
{
  Serial.println("****** clear start ******");
  for (unsigned short i = 0; i <= 255; i++)
  {
    Serial.print("value = ");
    Serial.println(i);
    EEPROM.write(i, 0);
  }
  EEPROM.write(keeper, 0);
  resetFunc();
  return 0;
}
void WetlavelEdit ()
{
  digitalWrite(WetlavelEditPower, HIGH);
  while (digitalRead(Button) == LOW)
  {
    Serial.print("LevelEdit = ");
    Serial.println(map (analogRead(1), 0, 1023, 0, 255));
  }
  EEPROM.write(Wetlavelmin, map (analogRead(1), 0, 1023, 0, 255));
  Serial.print("LevelEdit****** = ");
  Serial.println(map (analogRead(1), 0, 1023, 0, 255));
  digitalWrite(WetlavelEditPower, LOW);
  delay(500);
  return 0;
}
void watering ()
{

}
void help()
{
  Serial.println("****** HELP ******");
  Serial.println("w - the entry in memory");
  Serial.println("r - reading data in the memory");
  Serial.println("c - clear the memory");
  Serial.println("h - help");
  Serial.println("R - Restart");
  Serial.println("a - Read all");
  Serial.println("n - WetsensorActivate");

  return 0;
}
void switchTimer()
{

  byte val;
  if (Serial.available() > 0)
  {
    val = Serial.read();
    if (val == 'w') EEPROMwrite();
    else if (val == 'r') EEPROMread(EEPROM.read(countlog));
    else if (val == 'a') EEPROMread(1024);
    else if (val == 'n') digitalWrite(WetsensorPower , ! digitalRead(WetsensorPower));
    else if (val == 'c') EEPROMclear();
    else if (val == 'R') resetFunc();
    else if (val == 'm')  digitalWrite(5 , ! digitalRead(5));
    else if (val == 'h') help();
    else
    {
      Serial.println("this command does not exist");
      Serial.println("enter h for help");
    }
  }
  if (digitalRead(Button) == LOW) WetlavelEdit();
  return 0;
}
