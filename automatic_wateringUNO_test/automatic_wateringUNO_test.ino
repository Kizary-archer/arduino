#include <EEPROM.h>
#include <MsTimer2.h>
#include <iarduino_RTC.h>
#define WetsensorPower 8
#define TimeSensorHours 1000 //Часы в памяти
#define TimeSensorDays 1001 //Дни в памяти
#define TimeSensorMonth 1002 // Месяц в памяти
#define keeper 1003 //Сторож перезаписи данных
#define countlog 1004 // размер лога
#define Wetlavel  0
iarduino_RTC time(RTC_DS1307);
byte Hr; //Hours
void EEPROMread();
void EEPROMwrite();
void EEPROMclear();
void switchTimer();
void help();

void setup()
{
  pinMode(Wetlavel, INPUT);
  pinMode(WetsensorPower, OUTPUT);
  pinMode(9, INPUT_PULLUP);
  pinMode(12, OUTPUT);
  Serial.begin(9600);
  MsTimer2::set(2, switchTimer); // задаем период прерывания по таймеру 2 мс
  MsTimer2::start();
  time.begin();
  Serial.println("v1.3");
  Serial.println(time.gettime("d-m-Y, H:i:s, D"));
  Serial.println("enter h for help");
  time.gettime();
  if (EEPROM.read(keeper) == 0)
  {
    EEPROM.write(TimeSensorHours, time.Hours);
    EEPROM.write(TimeSensorDays, time.day);
    EEPROM.write(TimeSensorMonth, time.month);
    EEPROM.write(keeper, 1);
    EEPROM.write(countlog, 0);
  }
  Hr = time.Hours;
}
void(* resetFunc) (void) = 0;//объявляем функцию reset с адресом 0
void loop()
{
  time.gettime();
  if (time.Hours != Hr) EEPROMwrite();
  delay(5000);
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
  Serial.println(map (analogRead(Wetlavel), 0, 1023, 0, 255));
  EEPROM.write(addr, time.Hours);
  addr++;
  EEPROM.write(addr, map (analogRead(Wetlavel), 0, 1023, 0, 255));
  addr++;
  EEPROM.write(countlog, addr);
  Hr = time.Hours;
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
  /////////////////////////////////////////
}
void watering ()
{
  //////////////////////////////////////
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
  digitalWrite(12, ! digitalRead(9) );
  byte val;
  if (Serial.available() > 0)
  {
    val = Serial.read();
    if (val == 'w') EEPROMwrite();
    else if (val == 'r') EEPROMread(EEPROM.read(countlog));
    else if (val == 'a') EEPROMread(1024);
    else if (val == 'n') {
     boolean b = digitalRead(WetsensorPower) ;
      digitalWrite(WetsensorPower , !b);
    }
    else if (val == 'c') EEPROMclear();
    else if (val == 'R') {
      EEPROM.write(countlog, 0);
      Serial.println("*** RESTART ***");
      delay(2000);
      resetFunc();
    }
    else if (val == 'h') help();
    else

    {
      Serial.println("this command does not exist");
      Serial.println("enter h for help");
    }
  }
  return 0;
}
