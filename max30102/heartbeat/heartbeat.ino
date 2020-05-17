/*
  This code is Based on :
  Optical Heart Rate Detection (PBA Algorithm) using the MAX30105 Breakout
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout
  
  Modifications Done by Girish S Kumar
  
  1. This uses Spark Fun Electronics MAX30105 Library to process the IR signals
  2. Make use of FREERTOS API calls.
  3. Tested on ESP32F using MAX30102 Sensor
  4. Uses some simple statistics methods to pick data with least variance
  
  This should not be used for any Medical or mission critical application. This is right now
  in experimental stage.
*/

#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"
#define ONBOARD_LED 2
#define SAMPLE_SIZE 10 
#define SD_LIMIT 2.0 
MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg = 0 ;
int prevBeatAvg = 100 ;
TaskHandle_t   Task1 ;
int listOfTenSamples[SAMPLE_SIZE];
int idx =0 ;
void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");
  pinMode(ONBOARD_LED,OUTPUT);
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30102 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
   xTaskCreate(ReadSensor, /* Pointer to the function that implements the task. */
             "TaskOne",/* Text name for the task. This is to facilitate debugging only. */
             10000, /* Stack depth - small microcontrollers will use much less stack than this. */
             NULL, /* This example does not use the task parameter. */
             1, /* This task will run at priority 1. */
             &Task1); /* This example does not use the task handle. */

}
void loop() { }

void ReadSensor(void *params)
{
  long irValue ;
  float SumForSD ;
  float avg ;
  while(1)
  {
     irValue = particleSensor.getIR();

    if (checkForBeat(irValue) == true)
    {
        digitalWrite(ONBOARD_LED,HIGH);
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);
    
    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
      
    }
  }

//  Serial.print("IR=");
//  Serial.print(irValue);
//  Serial.print(", BPM=");
//  Serial.print(beatsPerMinute);
//  Serial.print(", Avg BPM=");
      if (prevBeatAvg != beatAvg)
     {
       int i ;
       int sum ;
       
       listOfTenSamples[idx] = beatAvg ;
       idx++ ;
       for (i=0; i< SAMPLE_SIZE;i++)
       {
        //Serial.printf("%d, ", listOfTenSamples[i]);
       }
       //Serial.printf("\n");
       if (idx >(SAMPLE_SIZE -1))
       { 
           sum = 0 ;
           for(i=0;i < SAMPLE_SIZE; i++)
           {
               sum = sum + listOfTenSamples[i] ;     
           }
           avg  = sum/SAMPLE_SIZE ;
           idx = 0;
           //Calculate Standard Deviation 
           SumForSD = 0.0 ;
           for(i=0;i < SAMPLE_SIZE; i++)
           {
              float diff ;
              diff = listOfTenSamples[i] - avg ;
              diff = diff * diff ;
              SumForSD = SumForSD + diff ;   
           }
           SumForSD = SumForSD/SAMPLE_SIZE ;
           SumForSD = sqrt(SumForSD) ;
           if (SumForSD < SD_LIMIT)
           {
             Serial.printf("Avg = %f, SD = %f\n",avg, SumForSD);
             for(i=0;i<SAMPLE_SIZE;i++)
             {
                Serial.printf("%d, ", listOfTenSamples[i]);
                listOfTenSamples[i] = 0 ;
             }
             Serial.printf("\n");
           }
       }
       prevBeatAvg = beatAvg;
     }
      vTaskDelay(10); 
      digitalWrite(ONBOARD_LED,LOW);
      if (irValue < 50000)
      {
       /* This indicate Finger is removed from sensor
        Since Finger is removed all the values so far
        Collected has to ignored 
        */
       for (idx = 0 ; idx < 10; idx++)
           listOfTenSamples[idx] = 0;
       idx = 0;    
       beatAvg = -1;
      }


  }
}
