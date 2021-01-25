/* This program is a simple fan controller
 */
#include <Arduino.h>
#include <TimerOne.h>
#include <Wire.h>


#define FAN_PWM_PIN 9 //9 or 10 for UNO
#define RPM_FAN_PIN 2   //2 or 3 for UNO 
#define MIN_FAN_PERIOD 12000 //10000us for our settings, to be modified by a stall.


#define PULSE_PER_REV 2
#define MIN_DUTY_CYCLE 0
#define MAX_DUTY_CYCLE 100
#define DEFAULT_DUTY_CYCLE 20
#define RPM_READING_TIMEOUT 150  //in ms will set fan speed to 0 if timeout

// fan command
float dutyCycle = DEFAULT_DUTY_CYCLE;

//fan reading
volatile unsigned long t0 = micros();
volatile unsigned long fanPeriod;// = 300000;
volatile boolean flag_fanPeriod = false;

float instantFanSpeed = 0;
float averageFanSpeed = 0; // for moving average
float  alpha = 0.8;//0.8; // 0 = no filter 1 = filter with 0 Hz frequency cut



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

  //set duty cycles, to default value
    Timer1.pwm(FAN_PWM_PIN, (dutyCycle / 100) * 1023);
    Serial.print(" DUTY CYCLE :  ");Serial.println(dutyCycle);

}

void loop()
{
    if(Serial.available()>0){
      dutyCycle = dutyFromSerial();
      Serial.print("DUTY = ");Serial.println(dutyCycle);
    }
    Timer1.pwm(FAN_PWM_PIN, (dutyCycle / 100) * 1023);

    Serial.println(getFanSpeed());
    //getFanSpeed();
    //Serial.println(100*abs(averageFanSpeed-instantFanSpeed)/averageFanSpeed);  

    //DUBUGING Diplay RPM readings
    /**if (flag_fanPeriod == 1){
      Serial.print("fan period: ");Serial.print(fanPeriod);Serial.println("  us");
      //Serial.print("fan frequency: ");Serial.print((float) 1000000/ (PULSE_PER_REV * fanPeriod));Serial.println("  Hz");
      //Serial.print("fan speed: ");Serial.print((float) 60*1000000/ (PULSE_PER_REV * fanPeriod));Serial.println("  RPM");
      flag_fanPeriod = false;
    }*/

    //delay(10);
}




float getFanSpeed(void) {
  long timestamp = millis();
  while ((millis() - timestamp) < RPM_READING_TIMEOUT && (flag_fanPeriod == false)) ; //wait for both timeout and update
  //timeout, check update
  if (flag_fanPeriod == 1)  {
    instantFanSpeed = 60*1000000/ (PULSE_PER_REV * fanPeriod);
    
    /**if(instantFanSpeed > 500 && averageFanSpeed > 100 && (100*abs(averageFanSpeed-instantFanSpeed)/averageFanSpeed)>50.0)
    {
      instantFanSpeed = averageFanSpeed;
    }*/
    // Does not work together with the leaky integrator 

    //averageFanSpeed = instantFanSpeed;

    // Leaky integrator to smooth data (= low pass filter alpha=0 no filter alpha = 1 all data are filtered)
    averageFanSpeed = alpha * averageFanSpeed + (1-alpha) *  instantFanSpeed;
    flag_fanPeriod = false;
  } 
  else{
    averageFanSpeed =  0.0;
  } 
  return averageFanSpeed;
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
      flag_fanPeriod = false;//stall alarm
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