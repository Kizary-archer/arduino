#include <EEPROM.h>
#include <MsTimer2.h>
#include <iarduino_RTC.h>
unsigned short Wetlavel = 0;
iarduino_RTC time(RTC_DS1307);
byte Hr; //Hours
void EEPROMread();
void EEPROMwrite();
void EEPROMclear();
void switchTimer();
void help();
void ReadAll();

void setup()
{
  pinMode(Wetlavel, INPUT);
  pinMode(8, OUTPUT);
  Serial.begin(9600);
  MsTimer2::set(2, switchTimer); // задаем период прерывания по таймеру 2 мс
  MsTimer2::start();
  time.begin();
  Serial.println(time.gettime("d-m-Y, H:i:s, D"));
  Serial.println("enter h for help");
  time.gettime();
  if (EEPROM.read(1003) == 0)
  {
    EEPROM.write(1000, time.Hours);
    EEPROM.write(1001, time.day);
    EEPROM.write(1002, time.month);
    EEPROM.write(1003, 1);
    EEPROM.write(1004, 0);
  }
  Hr = time.Hours;
}
void(* resetFunc) (void) = 0;//объявляем функцию reset с адресом 0
void loop()
{

  /*Serial.print("Wetvalue-now = ");
    Serial.println(analogRead(Wetlavel));*/
  time.gettime();
  if (time.Hours != Hr)
    EEPROMwrite();
  delay(5000);
}

void EEPROMwrite()
{
  byte addr = EEPROM.read(1004), full = EEPROM.read(256);

  if (addr >= 254) //Переполнение
  {
    EEPROM.write(1003, 0);
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
  digitalWrite(8, HIGH);
  delay(5000);
  Serial.println(analogRead(Wetlavel));
  EEPROM.write(addr, time.Hours);
  addr++;
  EEPROM.write(addr, analogRead(Wetlavel) / 4);
  addr++;
  EEPROM.write(1004, addr);
  Hr = time.Hours;
  digitalWrite(8, LOW);
  return 0;
}

void EEPROMread()
{
  byte j = 1;
  Serial.println("****** Read start ******");
  Serial.print(EEPROM.read(1000));
  Serial.print(" Hours : ");
  Serial.print(EEPROM.read(1001));
  Serial.print(" Days : ");
  Serial.print(EEPROM.read(1002));
  Serial.println(" Month");
  for (unsigned short i = 0; i < EEPROM.read(1004); i++)
  {
    if (i % 2 != 0) //чередование времени и значений
    {
      Serial.print("Wetvalue");
      Serial.print("[");
      Serial.print(j);
      Serial.print("] = ");
      Serial.println(EEPROM.read(i) * 4);
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
  return 0;
}

void EEPROMclear()
{
  Serial.println("****** clear start ******");
  for (unsigned short i = 0; i <= 254; i++)
  {
    Serial.print("value = ");
    Serial.println(i);
    EEPROM.write(i, 0);
  }
  EEPROM.write(1003, 0);
  resetFunc();
  return 0;
}
void ReadAll() //!!!!временная функция (реализовать 1й)
{
  byte j = 1;
  Serial.println("****** Read start ******");
  Serial.print(EEPROM.read(1000));
  Serial.print(" Hours : ");
  Serial.print(EEPROM.read(1001));
  Serial.print(" Days : ");
  Serial.print(EEPROM.read(1002));
  Serial.println(" Month");
  for (unsigned short i = 0; i <= 1023; i++)
  {
    if (i % 2 != 0) //чередование времени и значений
    {
      Serial.print("Wetvalue");
      Serial.print("[");
      Serial.print(j);
      Serial.print("] = ");
      Serial.println(EEPROM.read(i) * 4);
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
  return 0;

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

  return 0;
}
void switchTimer()
{
  int val;
  if (Serial.available() > 0)
  {
    val = Serial.read();
    if (val == 'w') EEPROMwrite();
    else if (val == 'r') EEPROMread();
    else if (val == 'a') ReadAll();
    else if (val == 'c') EEPROMclear();
    else if (val == 'R') {
      EEPROM.write(1003, 0);
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
