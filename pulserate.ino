#define USE_ARDUINO_INTERRUPTS false //using ESP-32

//install all the modules
#include <PulseSensorPlayground.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <Arduino.h>

const char ssid[] = ""; //write wifi name
const char password[] = ""; // write wifi password
WiFiClient client;

//ThingSpeak
const long CHANNEL = ; //In this field, enter the Channel ID
const char *WRITE_API = "";// Enter the Write API key 

long prevMillisThingSpeak = 0;
int intervalThingSpeak = 20000; // 15 seconds to send data to the dashboard

const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = 32;   
const int PULSE_BLINK = 21;    
const int PULSE_FADE = 5;
const int THRESHOLD = 550;   

byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 10;

PulseSensorPlayground pulseSensor;

//const int PIN_SPEAKER = 35;


void setup() 
{
  Serial.begin(115200);

  ledcSetup(0,1E5,12);
  ledcAttachPin(25,0);
  
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);
  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
  
  if (!pulseSensor.begin()) 
  {
    for(;;)
    {
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  
  
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, password);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  } 
}

void loop() 
{
  if (pulseSensor.sawNewSample()) 
  {
    if (--samplesUntilReport == (byte) 0) 
    {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
      pulseSensor.outputSample();
      if (pulseSensor.sawStartOfBeat()) 
      {
        pulseSensor.outputBeat();
//        tone(PIN_SPEAKER,1047);
//        ledcWriteTone(0,800);
      }
    }
    int myBPM = pulseSensor.getBeatsPerMinute();

    if (myBPM < 100 && myBPM > 50){
      if (millis() - prevMillisThingSpeak > intervalThingSpeak) 
      {
      ThingSpeak.setField(1, myBPM);
      int x = ThingSpeak.writeFields(CHANNEL, WRITE_API);
      if (x == 200) 
      {
        Serial.println("Channel update successful.");
      }
      else 
      {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
        prevMillisThingSpeak = millis();
      }        
    }
//   if(pulseSensor.isInsideBeat() == false){
//    uint8_t octave = 1;
//    ledcWriteNote(0,NOTE_C,octave);
//   }     
  }
}
