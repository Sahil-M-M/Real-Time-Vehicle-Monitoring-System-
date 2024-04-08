/*
Auther: Sahil Mahendra Mangaonkar
Discription: This program reads data from various sensors and uploads it onto ThingSpeak could. Critical alerts are sent from the cloud using IFTTT applets
*/

#include<ESP8266WiFi.h>
#include "DHT.h"

//Pin declarations
#define AlcoholSensorPin D0     // Digital pin connected to the Alcohol sensor
#define CNGSensorPin D1         // Digital pin connected to the CNG gas sensor
#define DHTPin D2               // Digital pin connected to the DHT sensor
#define FlameSensorPin D3       // Digital pin connected to the Flame sensor
#define IRSensorPin D5          // Digital pin connected to the IR sensor
#define BuzzerPin D6            // Digital pin connected to the Buzzer
#define VibrationSensorPin D7   // Digital pin connected to the Vibration sensor
#define PIRSensorPin D8         // Digital pin connected to the PIR sensor
#define SmoakeSensorPin A0      // Analog pin connected to the Smoke gas sensor
#define DHTTYPE DHT11           // DHT 11/22

// Wifi & server details
#define SSID "Lenovo K8 Note 9382"  
#define PASS "1234567890"
#define server "api.thingspeak.com"

DHT dht(DHTPin, DHTTYPE);  // Creating an object

//Variable declarations
bool AlcoholState = 0;                //Varible to store Alcohol State; 0 when detected
bool CNGState = 0;                    //Varible to store CNG State; 1 when detected
bool FlameState = 0;                  //Varible to store Flame State; 0 when detected
bool IRState = 0;                     //Varible to store IR State; 0 when detected
bool PIRState = 0;                    //Varible to store PIR State; 1 when detected
bool VibrationState = 0;              //Varible to store Vibration State; 1 when detected
int SmokeVal = 0;                     //Variable to store the Smoke Value
const int SmokeThreshold = 600;       //Threshold for Smoke Value
int TemperatureVal = 0;               //Variable to store the Temperature Value
const int TemperatureThreshold = 50;  //Threshold for Humidity Value
int HumidityVal = 0;                  //Variable to store the Temperature Value
const int HumidityMinThreshold = 30;  //Lower Threshold for Humidity Value
const int HumidityMaxThreshold = 50;  //Higher Threshold for Humidity Value

WiFiClient client;

void setup() {
  
   // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();

  Serial.print("Conneting to %s");
  Serial.println(SSID);

  WiFi.begin(SSID,PASS);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  
  }
  Serial.println("");
  Serial.println("WiFi conneted");

  pinMode(AlcoholSensorPin, INPUT);
  pinMode(CNGSensorPin,INPUT);
  pinMode(DHTPin,INPUT);
  pinMode(FlameSensorPin,INPUT);
  pinMode(IRSensorPin, INPUT);
  pinMode(VibrationSensorPin,INPUT);
  pinMode(BuzzerPin,OUTPUT);
}

void loop() {

  //Detect Alcohol using MQ-3 Gas sensor
  AlcoholState = digitalRead(AlcoholSensorPin);     //Reads the digital value from the MQ-3 Gas sensor's DOUT pin

  if (AlcoholState == LOW)    //Reads LOW (0) when Alcohol is detected
  {
    Serial.println("Alchol Detected ");
  }


  //Detect CNG leakage using MQ-4 Gas sensor
  CNGState = digitalRead(CNGSensorPin);   //Reads the digital value from the MQ-4 Gas sensor's DOUT pin; 

  if (CNGState == HIGH)   //Reads HIGH (1) when Alcohol is detected
  {
    Serial.println("CNG Leakage Detected ");
  }


  //Take Temperature readings from DHT11 sensor
  TemperatureVal = dht.readTemperature();

  if (TemperatureVal >= TemperatureThreshold)  //Buzzer rings when Temperature Value more than the set threshold
  { 
    Serial.print("Temperature: ");
    Serial.println(TemperatureVal);
    Serial.println("Temperature is to high");
    digitalWrite(BuzzerPin, HIGH);
    delay(3000);
    digitalWrite(BuzzerPin, LOW);
  }

  //Take Humidity readings from DHT11 sensor
  HumidityVal = dht.readHumidity();

  if (HumidityMinThreshold <= HumidityVal >= HumidityMaxThreshold)  //Buzzer rings when Humidity Value is outside the normal range
  {
    Serial.print("Humidity: ");
    Serial.println(HumidityVal);
    Serial.println("Humidity Value is out of normal range");
    digitalWrite(BuzzerPin, HIGH);
    delay(3000);
    digitalWrite(BuzzerPin, LOW);
  }


  //Detect Flame using Flame sensor
  FlameState = digitalRead(FlameSensorPin);   //Reads the digital value from the Flame sensor's DOUT pin

  if (FlameState == LOW)    //Reads LOW (0) when Flame is detected & buzzer rings
  {
    Serial.println("Flame Detected");
    digitalWrite(BuzzerPin, HIGH);
    delay(3000);
    digitalWrite(BuzzerPin, LOW);
  }

  //Detect proximity using IR sensor
  //If a driver can come near the steering wheel on appling brake then the driver isn't wearing seat belt
  IRState = digitalRead(IRSensorPin);   //Reads the digital value from the IR sensor's DOUT pin

  if (IRState ==LOW)    //Reads LOW (0) when an object is detected in the proximity
  {
    Serial.println("Proximity Detected");
  }

  //Detect motion using PIR sensor
  PIRState = digitalRead(PIRSensorPin);   //Reads the digital value from the PIR sensor's DOUT pin

  if( PIRState == HIGH )    //Reads HIGH (1) when an Motion is detected
  {
    Serial.print("Driver is present\n");
  }

  //If a driver can come near the steering wheel on appling brake then the driver isn't wearing seat belt
  if( PIRState == HIGH && IRState ==LOW)      //Motion is detected with close proximity
  {
    Serial.print("Driver is not wearing a seat belt\n");
  }

  //Detect Vibrations using Vibration sensor in order to check if the car is on or not
  VibrationState = digitalRead(VibrationSensorPin);   //Reads the digital value from the Vibrations sensor's DOUT pin

  if (VibrationState == HIGH)    //Reads HIGH (1) when an Motion is detected
  { 
    Serial.println("Vibrations Detected: Car is On");
  }
  //Take Smoke readings from MQ-5 Gas sensor
  SmokeVal = analogRead(SmoakeSensorPin);     //Reads the analog value from the Smoke sensor's AOUT pin


  if (SmokeVal>=SmokeThreshold)     //Buzzer rings when Smoke Value more than the set threshold
  {
    Serial.print("Smoke value : ");
    Serial.println(SmokeVal);
    Serial.println("Smoke Detected");
    digitalWrite(BuzzerPin, HIGH);
    delay(3000);
    digitalWrite(BuzzerPin, LOW);
  }
  
  //Send data onto the server
  Serial.println("Sending......");

  if (client.connect(server, 80));          //Connect to server api.thingspeak.com
  {
    client.print("GET https://api.thingspeak.com/update?api_key=RO7DZ78P1NW588CU&field1="+ String(TemperatureVal)+"&field2="+ String(!FlameState)+"&field3="+ String(SmokeVal)+"&field4="+ String(!AlcoholState)+"&field5="+ String(CNGState)+"&field6="+ String(VibrationState)+"&field7="+ String(PIRState)+"&field8="+ String(!IRState)+"\n");
  }
  client.stop();                            //Close server connection

  delay(1000);
}