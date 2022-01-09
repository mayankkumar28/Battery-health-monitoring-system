#include "SPI.h"
#include <LiquidCrystal.h>
// SDO - PIN 12
// SDI - PIN 11
#define WRCFG  0x01 //Write Configuration Registers
#define RDCFG  0x02  // Read config
#define RDCV   0x04  // Read cells
 
#define STCVAD 0x10  // Start all A/D's - poll status
#define RDFLG  0x06 //Read Flags
#define RDTMP  0x08 //Read Temperatures
 
#define STCDC  0x60 //A/D converter and poll Status
#define STOWAD 0x20 //Start Test - poll status
#define STTMPAD 0x30// Temperature Reading - ALL
 
#define address 0x80
//Functions
#define TOTAL  4
#define LIMIT 0.01
byte byteTemp;
byte CFGR1 = 0x00;
byte CFGR2 = 0x00;
float cellVolt[TOTAL];
 
LiquidCrystal lcd(7,6,5,4,3,2);
 
 
void setup()
{
pinMode(10,OUTPUT);
pinMode(11,OUTPUT);
pinMode(12,INPUT);
pinMode(13,OUTPUT);
digitalWrite(10, HIGH);
SPI.setBitOrder(MSBFIRST);
SPI.setDataMode(SPI_MODE3);
SPI.setClockDivider(SPI_CLOCK_DIV16);
SPI.begin();
Serial.begin(9600);
//LCD Setup
lcd.begin(16,2);
lcd.setCursor(0,0);
lcd.print("C1: ");
lcd.setCursor(8,0);
lcd.print("C2: ");
lcd.setCursor(0,1);
lcd.print("C3: ");
lcd.setCursor(8,1);
lcd.print("C4: ");
//lcd.print("Hello world!");
writeReg(); //Initial setup
}
 
void loop()
{
readV();
//Serial.print("Lowest cell: "); Serial.print(lowCell()); Serial.print(" , Value: "); Serial.println(cellVolt[lowCell()-1]);
//Serial.print("Highest cell: "); Serial.print(highCell()); Serial.print(" , Value: "); Serial.println(cellVolt[highCell()-1]);
startBalance();
//delay(2000);
Serial.println(CFGR1,BIN);
lcd.setCursor(3,0);
lcd.print(cellVolt[0]);
lcd.setCursor(11,0);
lcd.print(cellVolt[1]);
lcd.setCursor(3,1);
lcd.print(cellVolt[2]);
lcd.setCursor(11,1);
lcd.print(cellVolt[3]);
delay(1000);
}
 
void writeReg()
{
 Serial.println("Writing config...");
  digitalWrite(10, LOW);
  SPI.transfer(address);
  SPI.transfer(WRCFG);
  SPI.transfer(0x01);//0
  SPI.transfer(CFGR1);//1
  SPI.transfer(CFGR2);//2
  SPI.transfer(0x00);//3
  SPI.transfer(0x71);//4
  SPI.transfer(0xAB);//5
  digitalWrite(10, HIGH);
}
void readReg()
{
Serial.println("Reading config...");
digitalWrite(10, LOW);
   SPI.transfer(address);
   SPI.transfer(RDCFG);
   for(int i = 0; i < 6; i++)
   {
   byteTemp = SPI.transfer(RDCFG);
   Serial.println(byteTemp, HEX);
   }
  digitalWrite(10, HIGH);
}
void readV()
{
  digitalWrite(10,LOW);
  SPI.transfer(STCVAD);
  delay(20); // wait at least 12ms as per data sheet, p.24
  digitalWrite(10,HIGH);
  byte volt[18];
  digitalWrite(10,LOW);
  SPI.transfer(0x80);
  SPI.transfer(RDCV);
  for(int j = 0; j<18;j++)
  {
  volt[j] = SPI.transfer(RDCV);
  }
  digitalWrite(10,HIGH);
  cellVolt[0] = (((volt[0] & 0xFF) | (volt[1] & 0x0F) << 8)*1.5*0.001);
  cellVolt[1] = (((volt[1] & 0xF0) >> 4 | (volt[2] & 0xFF) << 4)*1.5*0.001); 
  cellVolt[2] = (((volt[3] & 0xFF) | (volt[4] & 0x0F) << 8)*1.5*0.001);
  cellVolt[3] = (((volt[4] & 0xF0) >> 4 | (volt[5] & 0xFF) << 4)*1.5*0.001);
  Serial.println(cellVolt[0]);
  Serial.println(cellVolt[1]);
  Serial.println(cellVolt[2]);
  Serial.println(cellVolt[3]);
  Serial.println("--------------------");
}
 
int startBalance()
{
 CFGR1 = 0x00;
 CFGR2 = 0x00;
 int low = lowCell();
 for(int i = 1; i < TOTAL+1; i++)
 {
 if(i != low)
 {
 balanceCell(i);//set each cell individually; discharge if not lowest cell
 }
 }
 Serial.println(CFGR1,BIN);
 Serial.println(CFGR2,BIN);
 writeReg();
   
}
 
int balanceCell(int x)
{
  double diff = cellVolt[x-1] - cellVolt[lowCell()-1]; //check the difference between current cell and lowestCell
  if(x <= 8 && diff > 0.03)   //discharge if difference more than 0.03V
  {
  CFGR1 += 0x01<<(x-1);
  }
  if(x <= 8 && diff <= 0.03)
  {
  CFGR1 += 0x00<<(x-1);  
  }
  if(x > 8 && diff > 0.03)
  {
  CFGR2 += 0x01<<(x-9);
  }
  if(x > 8 && diff <= 0.03)
  {
  CFGR2 += 0x00<<(x-9);
  }
   
}
 
int highCell() //find highest voltage cell value
{
  int num = 0;
  float cellTemp = 0;
  for(int i = 0; i < TOTAL;)
  {
    if(cellVolt[i] > cellTemp)
    {
      cellTemp = cellVolt[i];
      num = i+1;
    }
    i++;
  }
  return num;
}
 
int lowCell() //find lowest cell value
{
  int num = 0;
  float cellTemp = 5;
  for(int i = 0; i < TOTAL;)
  {
    if(cellVolt[i] < cellTemp)
    {
      cellTemp = cellVolt[i];
      num = i+1;
    }
    i++;
  }
  return num;
}
