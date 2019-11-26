#ifndef __CC3200R1M1RGC__
// Do not include SPI for CC3200 LaunchPad
#include <SPI.h>
#endif
#include <WiFi.h>
#include <stdio.h>
#include "pitches.h"


char ssid[] = "NETGEAR65";            //variables for the network
char password[] = "littlecello367";
char destination;                     //variable for the destination specified by the server
char server[] = "192.168.1.2";   

int dest = 0;       //destination var
int pos = 0;        //current position
int dir = 1;        //current direction to dest, 0==clockwise 1==anticlockwise
int prevdir = 1;    //prev direction car was facing
int stops = 0;      //how many stops until destination

int destinations[7][2] = {  //an array of connected destinations. the first variable in each sub-array is the destination in the clockwise direction
    {4,2}, //position 0     //and the second is the next position in the anti-clockwise direction          
    {5,6}, //pos 1
    {0,3}, //pos 2
    {2,6}, //pos 3
    {6,0}, //pos 4
    {1,6}, //pos 5
    {3,4}  //pos 6
};

boolean goingto1 = false;   //used when going to position one
boolean goingto5 = false;   //used when going to position five
boolean combinationLock = false;  //used for inovation
int lightsensorPin = A1; // input pin for LDR 
int lightsensorValue = 0; // variable to store the value coming from the sensor 
int turned = 6; //previous turn direction
int motor1PWM = 39; // P2.6
int motor1Phase = 40; // P2.7
int motor2PWM = 38; // P2.6
int motor2Phase = 31; // P3.7
int AnalogValue[5] = {0,0,0,0,0}; //array for lightsensor array values
int AnalogPin[5] = {A6,A9,A11,A13,A14};  //Analog pins P4.7, P4.4, P4.2, P4.0, P6.1        
                                         //       J3    ->    28,     26,     25,     24,     23 

WiFiClient client;


void setup() {
    Serial.begin(9600);
    stopped();    //insures motors are stopped
    for (int i=0;i<5;i++) AnalogValue[i]=analogRead(AnalogPin[i]); //reads light sensor array to ignore first reading error
    stops=1;  //sets stops == 1 so car travels to first position
    while(combinationLock == false){
      combinationLock = digitalRead(35); //reads to see if combination has been input into the other board
    }
    delay(700);
    while(stops!=0){
      loopLightSensor();  //travels along line
    }
    ConnectToServer();  //Connects to the server
}

void loop() {
    dest = getdest(); //gets the destination from server
    traverse(); //travels to destination
    pos=dest;
}

void turn(int k){ //chooses the destination 
  switch(k){
    case 0:
      if(turned != 0){ //so it doesnt repeatedly reset motor speeds
        fullRight(); //turns right
        turned = 0;
      } 
      break;
    case 1:
      if(turned != 1){
        slightRight(); //veers right
        turned = 1;
      } 
      break;
    case 2:
       if(turned != 2){
        forward();//goes straight
        turned = 2;
      } 
      break;
    case 3:
       if(turned != 3){
        slightLeft();//veers left
        turned = 3;
      }
      break;
    case 4:
       if(turned != 4){
        fullLeft();//turns left
        turned=4;
      } 
      break;   
  }
}

void traverse(){
  
  if(pos!=dest){  //if not already at destination the rest of this code runs
    if(dest==1){  //sets destination to 6 (point between 4 and 3) so that we can hardcode the route from their to one
        dest=6;
        goingto1=true;  //sets true to use later to run hardcode
    }
    if(dest==5){  //does same as above
      dest=6;
      goingto5=true;
    }
    
    if(pos==1){ //everything in this if statement is hardcode to get from 1 to all other positions
        
        if((dest==2)||(dest==0)){ //going to 2 or 0
          stops = 1;//goes to end of centre line
          forward();
          delay(300);
          while(stops!=0){
          loopLightSensor();//follows line
          }
          stopped();
          
          if(dest==2) {//turns left
            forward();//forward off line
            delay(100);
            ninetyleft();//left onto perimeter line
            dir=1;//set direction
          }
          else {//turn right
            forward();
            delay(100);
            ninetyright();//right onto peremiter line
            dir=0;//set direction
          }
          stops = 1;
          while(stops!=0){//follows until at destination
            loopLightSensor();//continue to position
          }
        }

        if((dest==3)||(dest==4)||(dest==6)){//if going to 3 4 or 6
          turn180();//turn around
          stops = 1;//goes to pos 6(crossroads)
          forward();
          while(stops!=0){
            loopLightSensor();//go to end of line
          }
          stopped();
          if(dest==4){//turn left
            forward();//forward off line
            delay(250);
            ninetyleft();//turn left onto perimeter line
            dir=1;//set direction
            stops = 1;
            while(stops!=0){
              loopLightSensor();//go to next stop
            }
          }
          else if(dest==3){//turn right
            forward();
            delay(250);
            ninetyright();
            dir=0;
            stops = 1;
            while(stops!=0){
              loopLightSensor();//go to next stop
            }
          }
       }
      stopped();//stop
      prevdir=dir;//set previous direction
    }


    else{//this runs if not at position one
      getroute();//gets stops and direction
      if(dir!=prevdir){
        turn180();//if direction is opposite way turn around
        prevdir=dir;
      }
      if(stops!=0){
        forward();//forward to get off the line
        delay(200);
      }
      while(stops!=0){
        loopLightSensor();//follows line until at destination
      }
    }
    stopped();


    
    if(goingto1==true){//if destination is 1 (car will be at crossroads)
      forward();//forward off line
      delay(200);
      if(dir==0){//clockwise
        ninetyright();//turn right
      }
      else{//anti-clockwise
        ninetyleft();//turn left
      }
      stops=1; //set to travel to next stop
      while(stops!=0){
        loopLightSensor();//follows line
      }
      stopped();
      dest=1;//resets dest to 1
      goingto1=false;//resets bool
    }

    
    if(goingto5==true){//runs if destination is 5 (will be at crossroads)
      forward();//to get off line
      delay(200);
      if(pos==1){//if came from 1 do nothing
      }
      else if (dir==0){//if came from clockwise 
        ninetyleft();//turn left
      }
      else{//if came from anti-clockwisw
        ninetyright();//turn right
      }
      forward();
      delay(2400);//forward until close to line
      slowforward();//slow
      analogRead(lightsensorPin);//read pin once in case first read fault
      while(analogRead(lightsensorPin)<705){
        delay(1);//while light sensor low car will continue
      }
      stopped();//stop
      Beep();//beeps
      dest=5;//resets dest
      goingto5=false;//resets bool
    }
    }
    pos = dest;//sets position
    Serial.println("stopped");
    delay(300);
}

void forward() {
 digitalWrite(motor1Phase, LOW); //forward
 analogWrite(motor1PWM, 220); // set speed of motor
 digitalWrite(motor2Phase, HIGH); //forward
 analogWrite(motor2PWM, 203); // set speed of motor
 //Serial.println("forward"); // Display motor direction
}

void slowforward() {
 digitalWrite(motor1Phase, LOW); //forward
 analogWrite(motor1PWM, 90); // set speed of motor
 digitalWrite(motor2Phase, HIGH); //forward
 analogWrite(motor2PWM, 80); // set speed of motor
 //Serial.println("forward"); // Display motor direction
}

void slightRight() {
 digitalWrite(motor1Phase, LOW); //forward
 analogWrite(motor1PWM, 220); // set speed of motor
 digitalWrite(motor2Phase, HIGH); //forward
 analogWrite(motor2PWM, 170); // set speed of motor
 //Serial.println("slight right"); // Display motor direction
}

void fullRight() {
 digitalWrite(motor1Phase, LOW); //forward
 analogWrite(motor1PWM, 210); // set speed of motor
 digitalWrite(motor2Phase, HIGH); //forward
 analogWrite(motor2PWM, 0); // set speed of motor
 //Serial.println("full right"); // Display motor direction
}

void slightLeft() {
 digitalWrite(motor1Phase, LOW); //forward
 analogWrite(motor1PWM, 170); // set speed of motor
 digitalWrite(motor2Phase, HIGH); //forward
 analogWrite(motor2PWM, 220); // set speed of motor
 //Serial.println("slight left"); // Display motor direction
}

void fullLeft() {
 digitalWrite(motor1Phase, LOW); //forward
 analogWrite(motor1PWM, 0); // set speed of motor
 digitalWrite(motor2Phase, HIGH); //forward
 analogWrite(motor2PWM, 210); // set speed of motor
 //Serial.println("full left"); // Display motor direction
}

void backward() {
 digitalWrite(motor1Phase, HIGH); //Backward
 analogWrite(motor1PWM, 200); // set speed of motor
 digitalWrite(motor2Phase, LOW); //Backward
 analogWrite(motor2PWM, 200); // set speed of motor
 //Serial.println("Backward"); // Display motor direction
}

void turn180(){
 digitalWrite(motor1Phase, HIGH); //Backward
 analogWrite(motor1PWM, 170); // set speed of motor
 digitalWrite(motor2Phase, HIGH); //Backward
 analogWrite(motor2PWM, 170); // set speed of motor
 delay(760);//timed turn to when its roughly 180, then the line follower code straightens her up if its off centre
 //Serial.println("180"); // Display motor direction
}

void ninetyright(){
 digitalWrite(motor1Phase, HIGH); //Backward
 analogWrite(motor1PWM, 150); // set speed of motor
 digitalWrite(motor2Phase, HIGH); //Backward
 analogWrite(motor2PWM, 150); // set speed of motor
 delay(400);
 //Serial.println("90 left"); // Display motor direction
}

void ninetyleft(){
 digitalWrite(motor1Phase, LOW); //Backward
 analogWrite(motor1PWM, 150); // set speed of motor
 digitalWrite(motor2Phase, LOW); //Backward
 analogWrite(motor2PWM, 150); // set speed of motor
 delay(400);//timed turn to when its roughly 90, then the line follower code straightens her up if its off centre
 //Serial.println("90 right"); // Display motor direction
}

void stopped() {
 digitalWrite(motor1Phase, LOW); //forward
 analogWrite(motor1PWM, 0); // set speed of motor
 digitalWrite(motor2Phase, HIGH); //forward
 analogWrite(motor2PWM, 0); // set speed of motor
 //Serial.println("stopped"); // Display motor direction
}


void loopLightSensor(){//this is the code that follows the line and must loop(in a while()) to function
  int i = 0;   
  int sensor;
  boolean left=false;
  boolean right=false;
  for (i=0;i<5;i++)//reads each ldr 
     
  {     
    AnalogValue[i]=analogRead(AnalogPin[i]);//reads value
    
    if (AnalogValue[i] < 230){//if over a line
      turn(i); //turn in that direction (middle is straight, one over slight, outside is full) this keeps it on the line
    }
    if(i==4){//after all read it checks if it was over a line, this happens very often and does not ever miss a line
      if((AnalogValue[0]<220) || (AnalogValue[1]<220))left = true;//if either left ldr was over line
      if((AnalogValue[3]<220) || (AnalogValue[4]<220))right = true;//if either right was over the line
      if ((left)&&(right)&&(AnalogValue[2]<250)){//if left and right and center are over the line
              stopped();//stop
              stops--;//reduce stops to next dest
              if(stops!=0){//if not at destination
                forward();//forward to get off line
                delay(300);
              }
      }
    }
 } 
}

int getroute(){//finds the route to the destination
  int k;
  for(k=0; k<2; k++){
    if(destinations[pos][k] == dest){//goes to the current position array in destinations array
      dir = k;                        //it checks both its clockwise then anticlockwise neighbours 
      stops=1;                        //if they are the dest sets direction and stops to dest
    }
  }
  if(destinations[destinations[pos][0]][0]== dest){ //this checks its clockwise neighbours nieghbour
    dir = 0;
    stops=2;
  }
  else if(destinations[destinations[pos][1]][1]== dest){//this checks its anti-clockwise neighbours nieghbour
    dir = 1;
    stops = 2;
  }
  else if(destinations[destinations[destinations[pos][0]][0]][0] == dest){//this checks its clockwise neighbours nieghbours neighbour (say it three times fast)
    dir = 0;
    stops = 3;
  }
  else if(destinations[destinations[destinations[pos][1]][1]][1] == dest){//this checks its anti-clockwise neighbours nieghbours neighbour
    dir = 1;
    stops = 3;
  }
  //this could be tidied up with recursion but was unnecessary with our small amount of destinations
}

void Melody(){//plays a melody when done credit to energia example
  int melody[] = {
  NOTE_A3, NOTE_AS3,NOTE_B3, NOTE_C3};

  int noteDurations[] = {
  4, 4, 4, 2};

  for (int thisNote = 0; thisNote < 4; thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurations[thisNote];
    tone(A8, melody[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(A8);
  }

}

void Beep(){//plays a beep
  tone(A8, NOTE_A3, 1000/4);
  delay(1000/4*1.30);
  noTone(A8);
}


int ConnectToServer(){//connects to the server
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  //@@@@@@@@@@@@@printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
  }
}

int getdest(){ // sends message to server and gets destination as a reply
  // if there are incoming bytes available
  // from the server, read them and print them:
if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
    for(int h=0; h<7; h++){
      Beep();
    }
  client.connect(server, 80);
}
if(client.connected()){
        // Make a HTTP request:
    String messageToSend = "POST /mobilerobotics/api/position/tag/?group=9&pos=";
    String spos = String(pos);
    Serial.println(spos);
    messageToSend += spos;
    messageToSend += " HTTP/1.1";
    client.println(messageToSend);
    client.println("Host: energia.nu");
    client.println();
  
    char buffer[255] = {0}; 
    while (!client.available()){};
    if (client.available()){
      client.read((uint8_t*)buffer, client.available());
    }
    destination = buffer[178];
    String finalMessage = buffer;
    Serial.println(finalMessage);
    Serial.println(destination);
    if(destination=='T'){
      Melody();
    }
    int idestination = destination - '0';
    return idestination;
}
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
