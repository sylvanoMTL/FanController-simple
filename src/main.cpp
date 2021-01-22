/* This program is a simple fan controller
 */
#include <Arduino.h>
#include <TimerOne.h>
#include <Wire.h>


#define FAN_PWM_PIN 9 //9 or 10 for UNO
#define RPM_FAN_PIN 2   //2 or 3 for UNO 
#define MIN_FAN_PERIOD 000 //8000us for our settings, to be modified by a stall.


#define PULSE_PER_REV 2
#define MIN_DUTY_CYCLE 0
#define MAX_DUTY_CYCLE 100
#define DEFAULT_DUTY_CYCLE 20


// fan command
float dutyCycle = DEFAULT_DUTY_CYCLE;

//fan reading
volatile unsigned long t0 = micros();
volatile unsigned long fanPeriod = 300000;
volatile boolean flag_fanPeriod = false;
float averageVent = 0; // for moving average
float  actSpeed;



//---Serial input
const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;
            


//forward devlaration
void recvWithEndMarker(void);
int dutyFromSerial(void);
void getFanPeriod(void);
float getFanSpeed(void);



void setup()
{
  Serial.begin(9600);
  // FAN
  Timer1.initialize(40);  // 40 us = 25 kHz

  pinMode(RPM_FAN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(RPM_FAN_PIN), getFanPeriod, FALLING);//RISING);
  Serial.println(" Start ");

  //set duty cycles, to be replaced by my code with serial.read();
   /*   int dutyCycle = 75;
    Timer1.pwm(FAN_PWM_PIN, (dutyCycle / 100) * 1023);
    Serial.print(" DUTY CYCLE :  ");Serial.println(dutyCycle);*/

}

void loop()
{
    
    if(Serial.available()>0){
      dutyCycle = dutyFromSerial();
      Serial.print("DUTY = ");Serial.println(dutyCycle);
    }
    Timer1.pwm(FAN_PWM_PIN, (dutyCycle / 100) * 1023);




    //Diplay RPM readings
    if (flag_fanPeriod == 1){
      Serial.print("fan period: ");Serial.print(fanPeriod);Serial.println("  us");
      //Serial.print("fan frequency: ");Serial.print((float) 1000000/ (PULSE_PER_REV * fanPeriod));Serial.println("  Hz");
      //Serial.print("fan speed: ");Serial.print((float) 60*1000000/ (PULSE_PER_REV * fanPeriod));Serial.println("  RPM");
      flag_fanPeriod = false;
    }




}




float getFanSpeed(void) {
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
{   //update the period between two pulse
    long delta_t = micros() - t0;
    fanPeriod = delta_t;
    t0 = micros();

    if(fanPeriod>MIN_FAN_PERIOD){  //MIN_FAN_PERIOD = 8000us for our settings
      flag_fanPeriod = true;
    }
    else{
      ;//stall alarm
    }
}





void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc; 
    if (Serial.available() > 0) {
      while(Serial.available()>0){
          rc = Serial.read();
          if (rc != endMarker) {
              receivedChars[ndx] = rc;
              ndx++;
              if (ndx >= numChars) {
                  ndx = numChars - 1;
              }
          }
          else {
              receivedChars[ndx] = '\0'; // terminate the string
              ndx = 0;
              newData = true;
          }
      }
    }
}

int dutyFromSerial(void){
  recvWithEndMarker();
    if (newData == true) {
            
        int dutyCycle = atoi(receivedChars);      
        newData = false;
        
        //bound the duty cycle
        if(dutyCycle>MAX_DUTY_CYCLE){
          return MAX_DUTY_CYCLE;
        }
        else if(dutyCycle<MIN_DUTY_CYCLE){
          return MIN_DUTY_CYCLE;
        }
        else
        {
          return dutyCycle;
        }
    }
    else
    {
      return 0;
    }
}