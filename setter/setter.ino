#include <Wire.h>
 
void setup()
{
  Wire.begin();
  Serial.begin(9600);
 
  // clear /EOSC bit
  // Sometimes necessary to ensure that the clock
  // keeps running on just battery power. Once set,
  // it shouldn't need to be reset but it's a good
  // idea to make sure.
  /*
  Wire.beginTransmission(0x68); // address DS3231
  Wire.write(0x0E); // select register
  Wire.write(0b00011100); // write register bitmap, bit 7 is /EOSC
  Wire.endTransmission();
  */
  
  //Set Time
  /*
  Wire.beginTransmission(0x68);
  Wire.write(0); // start at register 0
  uint8_t time[3] = {0b0,0b01011001,0b00010110};
  Wire.write(time,3);
  Wire.endTransmission();
  */
  
  //Set Date
  /*
  Wire.beginTransmission(0x68);
  Wire.write(0x03); // start at register 0
  uint8_t time[4] = {0b00000001,0b00000011,0b00000110,0b00010010};
  Wire.write(time,4);
  Wire.endTransmission();
  */
}
 
void loop()
{
  // send request to receive data starting at register 0
  Wire.beginTransmission(0x68); // 0x68 is DS3231 device address
  Wire.write(0); // start at register 0
  Wire.endTransmission();
  Wire.requestFrom(0x68, 3); // request three bytes (seconds, minutes, hours)
 
  while(Wire.available())
  { 
    int seconds = Wire.read(); // get seconds
    int minutes = Wire.read(); // get minutes
    int hours = Wire.read();   // get hours
 
    seconds = (((seconds & 0b11110000)>>4)*10 + (seconds & 0b00001111)); // convert BCD to decimal
    minutes = (((minutes & 0b11110000)>>4)*10 + (minutes & 0b00001111)); // convert BCD to decimal
    hours = (((hours & 0b00100000)>>5)*20 + ((hours & 0b00010000)>>4)*10 + (hours & 0b00001111)); // convert BCD to decimal (assume 24 hour mode)
 
    Serial.print(hours); Serial.print(":"); Serial.print(minutes); Serial.print(":"); Serial.println(seconds);
  }
 
  delay(1000);
}
