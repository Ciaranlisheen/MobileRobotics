

#include "SPI.h"
#include "OneMsTaskTimer.h"
#include "LCD_SharpBoosterPack_SPI.h"

// Variables
LCD_SharpBoosterPack_SPI myScreen;
uint8_t myOrientation = 0;
uint16_t myCount = 0;
int Val1 = 0;//first combination lock unlock-value
int Val2 = 2;//second combination lock unlock-value
int Val3 = 3;//third combination lock unlock-value
int sensorPin3 = 23;//sensor pins for each combination wheel
int sensorPin2 = 24;
int sensorPin1 = 25;

// Add setup code
void setup() {
    Serial.begin(9600);
    digitalWrite(40, LOW);//set the signal pin low
    myScreen.begin();//start the sharp screen
    myScreen.clearBuffer();
    
    myScreen.setFont(1);//prints greeting
    myScreen.text(10, 10, "Hello!");//The values are for the starting pixel in horizontal distance then verticle
    myScreen.setFont(0.5);
    myScreen.text(10, 30, "Team 9");//starts at pixel 10 horizontal in and 30 verticle down
    myScreen.setFont(0);
    myScreen.text(10, 60, "U.S.S. BR33ZY");
    myScreen.flush();
    
    for (uint8_t i=0; i<20; i++){
      delay(100);
      myScreen.reverseFlush();//flips the screen colours (for FLARE!!)
    }
    

    myScreen.clear();
}

// Add loop code
void loop()
{ 
  if(analogRead(sensorPin1)<=400){//manual map the sensors to values as the increase was unlinear
    Val1=1;//sets value to 1 2 or 3
  }
  if((analogRead(sensorPin1)>400) && (analogRead(sensorPin1)<=480)){
    Val1=2;
  }
  if(analogRead(sensorPin1)>480){
    Val1=3;
  }
  
  if(analogRead(sensorPin2)<=400){//does this for all three sensors
    Val2=1;
  }
  if((analogRead(sensorPin2)>400) && (analogRead(sensorPin2)<=460)){
    Val2=2;
  }
  if(analogRead(sensorPin2)>460){
    Val2=3;
  }

    if(analogRead(sensorPin3)<=400){
    Val3=1;
  }
  if((analogRead(sensorPin3)>400) && (analogRead(sensorPin3)<=460)){
    Val3=2;
  }
  if(analogRead(sensorPin3)>460){
    Val3=3;
  }

  if((Val1==2) && (Val2==1) && (Val3==3)){//if the correct value is input (2,1,3)
    digitalWrite(40, HIGH);//set signal pin high to start the car
  }
    
    //screen output code
    myScreen.clearBuffer();//clear the buffer
    myScreen.setFont(0);//sets font size
    myScreen.text(10, 10, "Combination:");//Print combination on top
    for (uint8_t i=10; i<LCD_HORIZONTAL_MAX-10; i++) {
        myScreen.setXY(i,20,1);//draws a line accross the screen 20 pixels down
    }
    myScreen.setFont(1);//sets font size
    myScreen.text(10,30,String(Val1,10));//Prints the value of the first combination value currently set
    myScreen.text(10,50,String(Val2,10));//prints second
    myScreen.text(10,70,String(Val3,10));//prints third
    myScreen.flush();//flushes the data to output on the screen
    delay(100);
}
