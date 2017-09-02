#include <EEPROM.h>
#include <MsTimer2.h>
#include <iarduino_RTC.h>
#define Wetlavel 0
iarduino_RTC time(RTC_DS1307);
byte Hr; //Hours
void EEPROMread();
void EEPROMwrite();
void EEPROMclear();
void switchTimer();
void help();

void setup() 
{
  pinMode(Wetlavel,INPUT);
  pinMode(8,OUTPUT);
  Serial.begin(9600);
  MsTimer2::set(2,switchTimer); // задаем период прерывания по таймеру 2 мс
  MsTimer2::start();
  time.begin();
  Serial.println(time.gettime("d-m-Y, H:i:s, D"));
  Serial.println("enter h for help");
  time.gettime();
  if (EEPROM.read(503) == 0)
  {
    EEPROM.write(500,time.Hours);
    EEPROM.write(501,time.day);
    EEPROM.write(502,time.month);  
    EEPROM.write(503,1);
    EEPROM.write(504,0);
  }
  digitalWrite(8,LOW);
  Hr = time.seconds;
}
void loop() 
{    
  time.gettime();          
  Serial.print("Wetvalue-now = ");
  Serial.println(analogRead(Wetlavel));
  if (time.seconds!= Hr)
  {
  digitalWrite(8,HIGH);
   EEPROMwrite();
  delay(5000);
  Hr = time.seconds;
  digitalWrite(8,LOW);
  }
}

void EEPROMwrite()
{
  unsigned short addr = EEPROM.read(504);  
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
     EEPROM.write(4,addr);
return 0;
}
                   
void EEPROMread()
{ 
 byte j = 1;
 Serial.println("****** Read start ******");
 Serial.print(EEPROM.read(500));
 Serial.print(" Hours : ");
 Serial.print(EEPROM.read(501));
 Serial.print(" Days : ");
 Serial.print(EEPROM.read(502));
 Serial.println(" Month");
  for(unsigned short i = 3;i<=EEPROM.read(504);i++)
    { 
       if (i%2==0) //чередование времени и значений 
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
        Serial.println("--------------");
       }
    }
return 0;
}
    
void EEPROMclear()
{
  Serial.println("****** clear start ******");
    for(unsigned short i=0;i<=1023;i++) 
    {
     Serial.print("value = ");
     Serial.println(i);
     EEPROM.write(i,0);               
    } 
  EEPROM.write(500,time.Hours);
  EEPROM.write(501,time.day);
  EEPROM.write(502,time.month);
  EEPROM.write(503,1);  
  EEPROM.write(504,0);
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
