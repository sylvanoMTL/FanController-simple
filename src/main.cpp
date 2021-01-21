/* This program is a simple fan controller
 */
#include <Arduino.h>
#include <TimerOne.h>
#include <Wire.h>


#define FAN_PWM_PIN 9 //9 or 10 for UNO
#define RPM_FAN_PIN 2   //2 or 3 for UNO 


//Ventilator
volatile unsigned long t0 = micros();
volatile unsigned long fanPeriod = 300000;
volatile boolean flag_fanPeriod = false;
float averageVent = 0; // for moving average
float  actSpeed;



void getFanPeriod(void);
float speedVent();

void setup()
{
  Serial.begin(9600);
  // FAN
  Timer1.initialize(40);  // 40 us = 25 kHz
  attachInterrupt(RPM_FAN_PIN, getFanPeriod, RISING);
  Serial.println(" Start ");
}

void loop()
{
    //set duty cycles, to be replaced by my code with serial.read();
    int dutyCycle = 75;
    Timer1.pwm(FAN_PWM_PIN, (dutyCycle / 100) * 1023);
    Serial.print(" DUTY CYCLE :  ");Serial.println(dutyCycle);


    //Diplay RPM readings
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
    //averageVent = ((3.0 * averageVent) + speedVent) / 4.0; //no idea, smoothng  average on the last values?
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





