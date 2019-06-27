
#include <WiFi.h>
#include <AWS_IOT.h>

/* defines wifi parameters
 *  define WIFI_SSID "Mywifi"
 *  define WIFI_PASSWORD "Mypassword"
 */
#if __has_include("seesamconfig.h")
#include "seesamconfig.h"
#else
#include "bilgeconfig.h"
#endif

/* Board specific definitions */
#define LED_BUILTIN 13

/* Wifi definitions */
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

/* AWS definitions */
#define TOPIC_NAME AWS_TOPIC_NAME
char* awsEndpoint = AWS_ENDPOINT;
AWS_IOT bilgeMonitor;
char* clientid = AWS_CLIENT_ID;
const int httpPort = 443;

int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];


/* Echo sensor */
// HC-SR04
const int trigPin = A1;
const int echoPin = A2;
long duration;
float distance;

/* Tuning parameters */

/* in order to minimize power consumption we do not connect to the wifi all the time.
 * we only send if a minimum distances is reached or force send timer is reached
 * 
 */

/* minimum distance in cm that is necessary to trigger a wifi upload of the data, -1 means we always send the data */

/* Deep sleep feature */
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

/* we need to save the boot count in order to support the FORCE_SEND_EVERY_X_WAKEUPS feature */
#ifdef FORCE_SEND_EVERY_X_WAKEUPS
RTC_DATA_ATTR int bootCount = 0;
#endif

int shouldIsend(){
  
#ifdef FORCE_SEND_EVERY_X_WAKEUPS
  if (bootCount %  FORCE_SEND_EVERY_X_WAKEUPS == 0){
      return 1;
  }
#endif
  if (distance < MIN_DISTANCE_TO_SEND){
      return 1;
  }

  return 0;
}

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
    strncpy(rcvdPayload,payLoad,payloadLen);
    rcvdPayload[payloadLen] = 0;
    msgReceived = 1;
}


void turnonled(){
  digitalWrite(LED_BUILTIN, HIGH);
}

void turnoffled(){
  digitalWrite(LED_BUILTIN, LOW);
}

/* blink the led twice - when transfer was ok  */
void blinkledok(){
  turnoffled();
  delay(500);
  turnonled();
  delay(500);
  turnoffled();
  delay(500);
  turnonled();
  delay(500);
}

/* blink the led three times - when transfer failed */
void blinkledfail(){
  turnoffled();
  delay(500);
  turnonled();
  delay(500);
  turnoffled();
  delay(500);
  turnonled();
  delay(500); 
  turnoffled();
  delay(500);
  turnonled();
  delay(500); 
}

void connectToWifi(){
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
  if(bilgeMonitor.connect(awsEndpoint,clientid)== 0)
    {
        Serial.println("Connected to AWS");
        delay(1000);

        if(0==bilgeMonitor.subscribe(TOPIC_NAME,mySubCallBackHandler))
        {
            Serial.println("Subscribe Successfull");
        }
        else
        {
            Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
            while(1);
        }
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address");
        while(1);
    }
}

void setup() {
  
#ifdef FORCE_SEND_EVERY_X_WAKEUPS
  ++bootCount;
#endif

  pinMode(LED_BUILTIN, OUTPUT); // Sets the LED as an Output
  turnonled();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600); // Starts the serial communication
}

void sendToHost(){
  sprintf(payload,"{\"distance\":%f}",distance);
  if(bilgeMonitor.publish(TOPIC_NAME,payload) == 0){        
            Serial.print("Publish Message:");
            Serial.println(payload);
            blinkledok();
  }
  else
  {
      Serial.println("Publish failed");
      blinkledfail();
  }
}

void setupPins() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);  
}

float measureDistance() {
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculating the distance
  distance= duration*0.034/2.0;
  
  Serial.print("Distance: ");
  Serial.println(distance);
  
  return distance;
}


void deepsleep() {
  Serial.println("Entering Deep Sleep");
  
#ifndef FORCE_SEND_EVERY_X_WAKEUPS
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
#endif

  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

/* we put some setup functions in the loop, as we do deep sleep at the end of the loop*/
void loop() {
  setupPins();
  measureDistance();
  
  if(shouldIsend() == 1){
     connectToWifi();
     sendToHost();
  }
  turnoffled();
  deepsleep();
  //delay(5000);
}
