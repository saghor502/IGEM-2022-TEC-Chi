/*
This is the code made for the centrifuge from Tecnológico de Monterrey IGEM's 2022 team Agrocapsi.
For more information visit the Agrocapsi's Wiki right here: https://2022.igem.wiki/tec-chihuahua/index.html
*/

/*Buzzer*/
//D7

/*Driver*/
//D8 IN3
//D9 IN4
//D10 ENB

/*Encoder*/
//D2 OUT_B
//D4 OUT_A
//D3 SW

/*LCD*/
//A4 SDA
//A5 SCL

/*Lid Microswitch*/
//D11

/*Toggle Switch*/
//D12  

#include <ezButton.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <string.h>

ezButton VelSwitch(12);
//ezButton LidSwitch(11);
LiquidCrystal_I2C lcd(0x27,16,4);
#define LidSwitch 11
#define ENB 10
#define Motor1 9
#define Motor2 8
#define Buzzer 7
#define KnobA 4
#define KnobButton 3
#define KnobB 2

float a = 0;
int vel = 0;
int time = 0;
bool firstTimeChange = false;
volatile int knobPos = 0;
int prevKnobPos = 0;
int timerInSec = 0;
int startingTimer = 0;

bool knobPress = false; //check if knob is pressed
bool timeOver = false; //time is over
bool Beep = false;
bool firstTimeMotor = true;

void setup() {
  VelSwitch.setDebounceTime(50);
  //LidSwitch.setDebounceTime(50);
  pinMode (LidSwitch, INPUT_PULLUP);
  pinMode (ENB,OUTPUT);
  pinMode (Motor1,OUTPUT);
  pinMode (Motor2,OUTPUT);
  pinMode (Buzzer,OUTPUT);
  pinMode (KnobA,INPUT);
  pinMode (KnobButton,INPUT_PULLUP);
  pinMode (KnobB,INPUT);

  digitalWrite(Motor1, HIGH); //direction of centrifuge's spinning
  digitalWrite(Motor2, LOW);

  prevKnobPos = digitalRead(KnobA);

  //show welcoming message
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("Tec Chihuahua");
  lcd.setCursor(4,1);
  lcd.print("IGEM 2022");
  delay(3000);
  lcd.clear();
}

void loop() {
  knobPos = digitalRead(KnobA);

  VelSwitch.loop();
  //LidSwitch.loop();
  int isGs = VelSwitch.getState(); //check if switch g is pressed
  int lidOn = !digitalRead(LidSwitch); //LidSwitch.getState(); //check if lid is on

  if(!digitalRead(KnobButton)){
    knobPress = !knobPress;
    delay(250);
  }
  
  /**Prints from the LCD**/

  printVel(vel);
  printTime(time);

  if(lidOn){
    //block time and velocity knob
    //start motor on indicated speed
    speedControl(vel, isGs);
    //start counting down timer til timerOver is true
    if(time > 0){
      timeOver = false;
      time = time - 1;
      delay(1000);
    }else{
      timeOver = true;
    }

    if(timeOver){
      //velocity and time reset to 0
      vel = 0;
      //show finishing text with "open the lid"
      if(Beep){
        //motor off
        delay(3000); //time for the motor to fully start
        //sound on
        beeps();
        Beep = false;
      }
    }
  }else{
    digitalWrite(ENB, LOW);
    firstTimeMotor = true;
    Beep = true;
    if(isGs){
      if(vel > 12000){
        vel = 12000;
      }
      //show time and vel in gs
      lcd.setCursor(13,0);
      lcd.print("g's");
      if(knobPress){
        //edit time
        knobPos = digitalRead(KnobA);
        if (knobPos != prevKnobPos){
          if(time >= 60){
            if (digitalRead(KnobB) != prevKnobPos) {
              time += 60;
            }else{
              time -= 60;
            }
          }else{
            if (digitalRead(KnobB) != prevKnobPos) {
              time += 5;
            }else{
              time -= 5;
            }
          }
          
        }
        time = min(6039, max(0, time));

        prevKnobPos = knobPos;
      }else{
        //edit vel
        knobPos = digitalRead(KnobA);
        if (knobPos != prevKnobPos){
          if (digitalRead(KnobB) != prevKnobPos) {
            vel += 500;
          }else{
            vel -= 500;
          }
        }
        vel = min(12000, max(0, vel));

        prevKnobPos = knobPos;
      }
    }else{
      //show time and vel in rpm
      lcd.setCursor(13,0);
      lcd.print("rpm");
       if(knobPress){
        //edit time
        knobPos = digitalRead(KnobA);
        if (knobPos != prevKnobPos){
          if(time >= 60){
            if (digitalRead(KnobB) != prevKnobPos) {
              time += 60;
            }else{
              time -= 60;
            }
          }else{
            if (digitalRead(KnobB) != prevKnobPos) {
              time += 5;
            }else{
              time -= 5;
            }
          }
        }
        time = min(3600, max(0, time));
        
        prevKnobPos = knobPos;
      }else{
        //edit vel
        knobPos = digitalRead(KnobA);
        if (knobPos != prevKnobPos){
          if (digitalRead(KnobB) != prevKnobPos) {
            vel += 500;
          }else{
            vel -= 500;
          }
        }
        vel = min(13000, max(0, vel));
        prevKnobPos = knobPos;
      }
    }
  }
}

void beeps(){
  int repeat = 5; //times repeated
  while(repeat > 0){
    digitalWrite(Buzzer, HIGH);
    delay(100);
    digitalWrite(Buzzer, LOW);
    delay(300);
    repeat -= 1;
  }
}

void speedControl(int velocity, bool isGs) {
  //max rpm: 13,000
  //max gs: 12,000
  //max motor vel: 255
  if(isGs){
    //is on gravities
    a = velocity;
    a = a*255/12000;
  }else{
    //is on rpm
    a = velocity;
    a = a*255/13000;
  }
  if(firstTimeMotor){ 
    analogWrite(ENB, 150);
    delay(250);
  }
	analogWrite(ENB, a);
  firstTimeMotor = false;
}

void printTime(int time){
  int min = time/60;
  int sec = time - (min*60);

  if(time == 0){
    lcd.setCursor(0,1);
    lcd.print("00:00");
    lcd.setCursor(12,1);
    lcd.print("time");

  }else if((time < 6039) && (time != 0)){ //6039 are the máximun minuts the centrifuge can take because the lcd would print "99:99"
    if(min == 0){
      lcd.setCursor(0,1);
      lcd.print("00");
    }else if(min < 10){
      lcd.setCursor(0,1);
      lcd.print("0");
      lcd.setCursor(1,1);
      lcd.print(min);

    }else{
      lcd.setCursor(0,1);
      lcd.print(min);

    }
    lcd.setCursor(2,1);
    lcd.print(":");
    if(sec < 10){
      lcd.setCursor(3,1);
      lcd.print("0");
      lcd.setCursor(4,1);
      lcd.print(sec);

    }else{
      lcd.setCursor(3,1);
      lcd.print(sec);

    }

    lcd.setCursor(12,1);
    lcd.print("time");
  }
}

void printVel(int vel){
  if(vel == 0){
    lcd.setCursor(0,0);
    lcd.print("00000");

  }else if(vel < 1000){
    lcd.setCursor(0,0);
    lcd.print("00");
    lcd.setCursor(2,0);
    lcd.print(vel);

  }else if(vel < 10000){
    lcd.setCursor(0,0);
    lcd.print("0");
    lcd.setCursor(1,0);
    lcd.print(vel);

  }else{
    lcd.setCursor(0,0);
    lcd.print(vel);

  }
}
