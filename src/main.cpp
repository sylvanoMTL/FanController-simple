/* This program is a simple fan controller
 */
#include <Arduino.h>
#include <TimerOne.h>
#include <Wire.h>


#define VENT_PWM 10 //9 or 10 for UNO
#define RPM_RAW 3   //2 or 3 for UNO 


//Ventilator
volatile unsigned long t0 = micros();
volatile unsigned long fanPeriod = 300000;
volatile boolean flag_fanPeriod = false;
float averageVent = 0;
int ventPwm = 500;

boolean keyb = false;
float  actSpeed;
long lastEntry = 0;


void getFanPeriod(void);
float speedVent();

void setup()
{
  Serial.begin(9600);
  Serial.println("Start Setup");

  // FAN
  Timer1.initialize(40);  // 40 us = 25 kHz
  //pinMode(RPM_RAW,INPUT_PULLUP);
  attachInterrupt(RPM_RAW, getFanPeriod, RISING);

 // while (speedVent() > 0.0);
  Serial.println(" Start ");
  lastEntry = millis();
}

void loop()
{
    int dutyCycle = 75;
    Timer1.pwm(VENT_PWM, (dutyCycle / 100) * 1023);
    Serial.print(" DUTY CYCLE :  ");Serial.println(dutyCycle);

    if (flag_fanPeriod == 1){
      Serial.print("fan period: ");Serial.print(fanPeriod);Serial.println("  us");
      Serial.print("fan frequency: ");Serial.print((float) 1000000/fanPeriod);Serial.println("  Hz");
      
      flag_fanPeriod = false;
    }
    
    
    //actSpeed = speedVent();   
    //Serial.print(" actSpeed : ");Serial.print(actSpeed);
    
  
}




float speedVent() {
  long timestamp = millis();
  while ((millis() - timestamp) < 150 && (flag_fanPeriod == false)) ; //wait for both timeout and update
  //timeout, check update
  if (flag_fanPeriod == 1)  {
    float speedVent = 30000000.0 / fanPeriod;
    //averageVent = ((3.0 * averageVent) + speedVent) / 4.0; //no idea
    flag_fanPeriod = false;
  } 
  else{
    averageVent = 0.0;
  } 
  return averageVent;
}



void getFanPeriod(void)
{
    long delta_t = micros() - t0;
    fanPeriod = delta_t;
    t0 = micros();
    flag_fanPeriod = true;
}





