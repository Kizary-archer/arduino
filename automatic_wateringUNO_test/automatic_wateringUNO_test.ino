//да прибудут со мной Git технологии заморские
#include <EEPROM.h>
#include <MsTimer2.h>
#include <iarduino_RTC.h>
#define Wetlavel 0
iarduino_RTC time(RTC_DS1307);
unsigned short addr = 3,chas; // первый байт памяти для времени начала
void EEPROMread();
void EEPROMwrite();
void EEPROMclear();
void switchTimer();
void help();

/////////////////////////////////////

void setup() 
{
  pinMode(Wetlavel,INPUT);
  pinMode(8,OUTPUT);
  Serial.begin(9600);
  MsTimer2::set(2,switchTimer); // задаем период прерывания по таймеру 2 мс
  MsTimer2::start();              // разрешаем прерывание по таймеру
  time.begin();
  Serial.println(time.gettime("d-m-Y, H:i:s, D")); // выводим время
  Serial.println("enter h for help");
  time.gettime();
  EEPROM.write(0,time.Hours);
  EEPROM.write(1,time.day);
  EEPROM.write(2,time.month);  
  chas = time.Hours;
  digitalWrite(8,LOW);
}
////////////////////////////////////
void loop() 
{    
  time.gettime();          
  Serial.print("Wetvalue-now = ");
  Serial.println(analogRead(Wetlavel));
  delay(10000);  
  if (time.Hours>chas ||time.Hours<chas)
  {
  digitalWrite(8,HIGH);
  delay(5000);
  chas = time.Hours;
  EEPROMwrite();
  digitalWrite(8,LOW);
  }
}
void switchTimer()
{
  int val;
  if (Serial.available() > 0)
  {
  val = Serial.read();
  if (val=='w') EEPROMwrite();
  else
  if (val=='r') EEPROMread();
  else
  if (val=='c') EEPROMclear();
  else
  if (val=='h') help();
  else
  {
    Serial.println("this command does not exist");
    Serial.println("enter h for help");
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
return 0;
}
void EEPROMwrite()
{
     time.gettime(); 
     Serial.println("****** Write start ******"); 
     Serial.print("value"); 
     Serial.print("["); 
     Serial.print(addr); 
     Serial.print("] = ");
     Serial.println(analogRead(Wetlavel));  
     EEPROM.write(addr,time.Hours);
     addr++;
     EEPROM.write(addr,analogRead(Wetlavel) / 4);
     addr++;
return 0;
}
                   
void EEPROMread()
{ 
 byte j = 1;
 Serial.println("****** Read start ******");
 Serial.print(EEPROM.read(0));
 Serial.print(" Hours : ");
 Serial.print(EEPROM.read(1));
 Serial.print(" Days : ");
 Serial.print(EEPROM.read(2));
 Serial.println(" Month");
  for(unsigned short i = 3;i<1023;i++)
    { 
       if (i%2==0)
       {
       Serial.print("Wetvalue");
       Serial.print("["); 
       Serial.print(j); 
       Serial.print("] = "); 
       Serial.println(EEPROM.read(i)* 4);
       j++;
       }
       else
       {
        Serial.print(EEPROM.read(i));
        Serial.println(" Hours");
       }
    }
return 0;
}
    
void EEPROMclear()
{
  Serial.println("****** clear start ******");
    for(int i=3;i<=1023;i++) 
    {
     Serial.print("value = ");
     Serial.println(i);
     EEPROM.write(i,0);  
                    
    }
addr = 3;  
return 0;
}
