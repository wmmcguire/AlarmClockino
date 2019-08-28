/*Alarm Clock using Adafruit LED Backpack
 * and 7-Segment Display
 * 
 * by William McGuire
 * Clock written using examples by adafruit
 * 
 * 
 * Serial monitor inputs written using examples
 * by Robin2 on Arduino.cc forums
 * 
 * 
 * 
 * 
 * 
 */
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

//defining object to function
Adafruit_7segment sevSeg = Adafruit_7segment();

//Alarm Time
int alarmTime; 

//variables for debounce
int btnState; // current reading from button pin
int lastBtnState = LOW; // last reading from button pin

unsigned long lastDbnc = 0; //last time button was pressed
unsigned long dbncDelay = 50;//interval of time digital high can be read

//Is Alarm on?
bool alarmOn = false;

//Define time variables here
int hr = 0;
int mins = 0;
int sec = 0;

char collectedChar = 0;
String collectedStr = "";

//Use "unsigned long" for time variables
//Stores last time LED was updated
unsigned long prevMillis = 0;

//interval of 1000ms, or 1 second
const long interval = 1000;

//Blinker will constantly be changed between T and F
bool blinker = false; 




//pin variables
const int ledG = 10;
const int ledR = 9;
const int buzzer = 8;

const int potPin = A1;
const int btnPin = 2;



//Variables for when the time is set through serial monitor
int setHr = 0;
int setMin = 0;

int ampmState = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Started Clock.");

  //Start up the Display.
  //0x70 is the i2c address.
  sevSeg.begin(0x70);


  //initialize pinss
  pinMode(ledG, OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(btnPin, INPUT_PULLUP);

}

void loop() {
  //Checks the current millisecond, which 
  //Tells the controller when to increment a second
  unsigned long curMillis = millis();
  
  if (curMillis - prevMillis >= interval){
    sec++;
    prevMillis = curMillis;

    //Blink colon every second
    blinker = !blinker;
  }

  switch(ampmState){
    //Green LED on at AM
    case 0: 
      digitalWrite(ledG, HIGH);
      digitalWrite(ledR, LOW);
    //Red LED on at PM
    case 1:
      digitalWrite(ledG, LOW);
      digitalWrite(ledR, HIGH);  
  }
  
  //time value to display
  int dispVal = hr*100 + mins;
  //If when hours are past 12, subtract 12 hours
  if (hr > 12) {
    dispVal -= 1200;
    ampmState++;
  }
  //When it hits midnight
  else if (hr == 0) {
    dispVal += 1200;
    ampmState++;
  }

  //Print display in decimal
  sevSeg.print(dispVal, DEC);

  //Write value to show the value on display
  sevSeg.drawColon(blinker);
  sevSeg.writeDisplay();
  
  if (sec > 59){
    sec = 0;
    mins++;
    if (mins > 59){
      mins = 0;
      hr++;
      if (hr > 23){
        hr = 0;
      }
    }
  }
  
  //Setting Time
  
  inputNums();
  if (alarmTime == dispVal){
    alarmOn = true;
  }
  if (alarmOn == true){
    int potVal = analogRead(potPin);
    //shift from 10-bit to 8-bit
    int vol = potVal >> 2;
    analogWrite(buzzer, vol);

    bool pressed = digitalRead(btnPin);

    //If switch is changed
    if (pressed != lastBtnState){
      lastDbnc = curMillis;
    }
    
    if (curMillis - lastDbnc >= dbncDelay){
      //time intervals for debouncing
      if (pressed != btnState){
        btnState = pressed;

        //turn off alarm only when input is HIGH
        if (btnState == HIGH) {
          alarmOn = false;
        }
      }
      }
    
    lastBtnState = pressed;
    
  }
  else if (alarmOn == false){
    //turn off buzzer
    analogWrite(buzzer, 0);
  }

}


void inputNums() {
  if (Serial.available() > 0 ){
      
      collectedChar  = Serial.read();
      String hrStr = "";                 //empty string for hours
      String minStr = "";                //empty string for minutes
      String alarmStr = "";              //empty string for alarm

      
      //Serial monitor collects data character by character
      if ((collectedChar >= '0') && (collectedChar <= '9')){
        //put numbers into one string
        collectedStr += collectedChar;
       
        
        if (collectedStr.length() > 8){
          //Total of 8 inputs: hr|min|alarmtime
          //clear out the string for another input
          collectedStr = "";
        }
      }
      else {
        //turn the string of numbers into an integer
        //and store it in our alarm varaible
         for (int i = 0; i < 2; i++){ // first two indexes for hour
          hrStr += collectedStr.charAt(i);
        }
        for (int i = 2; i <= 3; i++){ //second two for minute
          minStr += collectedStr.charAt(i);
        }
        for (int i = 4; i <= 8; i++){ // last four for alarm
          alarmStr += collectedStr.charAt(i);
        }
        
        hr = hrStr.toInt();
        mins = minStr.toInt();
        alarmTime = alarmStr.toInt();
        Serial.println(hr); // print hour
        Serial.println(mins); // print minute
        Serial.println(alarmTime); // print alarm time
        
        
      }
       }
}



  