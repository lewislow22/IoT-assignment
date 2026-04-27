// assignment.cpp
// Sensor: TMP36
// This program reads temperature from a TMP36 sensor
// connected to the ESP32 and send the data to a flask
// server via JSON. 
// It also displays a series of LED patterns that are
// controlled by buttons on the flask server.

// include the time for the animations
#include <time.h>
#include <string>
#include <iostream>
using namespace std;

// Include WiFi library for network connection
#include <WiFi.h>

// JSON
#include <HTTPClient.h>

// MUST DO - update with you own computer's IP address (found with 'ipconfig' command)
const String IP_ADDRESS = "192.168.1.92";

// Client used to connect to the external server
WiFiClient client;

// ---------------- Wi-Fi credentials ----------------
// SSID and password of the wireless network.
// These must match the Wi-Fi network you want the ESP32 to connect to.
const char* ssid = "BTB-3ZWF2K";
const char* password = "Rx6gYJN4hRAk3nTc"; // CHANGE THIS TO YOUR WIFI PASSWORD

// ---------------- TMP36 Sensor Pin ----------------
// GPIO36 is an analog input pin on the ESP32
// where the TMP36 temperature sensor output is connected.
#define TEMP_PIN 13
#define LED0_PIN 12
#define LED1_PIN 11
#define LED2_PIN 10
#define LED3_PIN 9
#define LED4_PIN 6
#define LED5_PIN 5

// all of the LEDs used to turn them all of before new patetrn is shown
const int ALL_LEDS[6] = {LED0_PIN, LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN, LED5_PIN};

// LED pattern, each array is what should be on in the current "frame"
const int LED_PATTERN_WAVE[6][2] = {
  {0, 1},
  {1, 2},
  {2, 3},
  {3, 4},
  {4, 5},
  {5, 0}
};

const int LED_PATTERN_CURTAIN[4][2] = {
  {0, 5},
  {1, 4},
  {2, 3},
  {-1, -1}
};

const int LED_PATTERN_CHECKER[2][3] = {
  {0, 2, 4},
  {1, 3, 5}
};

// Loop counter for looping the LED patterns
unsigned long loopIteration = 0;

// keeps track of what frame of the loop patetrn should be shown
int frame = 0;

// set up the timer for the animations
time_t seconds;

// store current LED pattern. "wave","curtain","checker" or "temp"
String pattern = "checker";

// ---------------- ADC Settings ----------------

// Maximum ADC value when using 12-bit resolution
// 2^12 − 1 = 4095
#define ADC_MAX 4095.0

// Reference voltage of ESP32 analog input (approx. 3.3V)
#define VREF 3.3


// Variable to store the calculated temperature
float temperatureC;


// ---------------------------------------------------
// Function: readTemperature()
// Reads the TMP36 sensor and converts the analog value
// into a temperature value in degrees Celsius
// ---------------------------------------------------
float readTemperature()
{
  // Read the analog value from the sensor pin
  int adcValue = analogRead(TEMP_PIN);

  // Convert ADC value to voltage
  // Formula: voltage = (ADC value / ADC max) * reference voltage
  float voltage = adcValue * VREF / ADC_MAX;

  // Convert voltage to temperature using TMP36 formula
  // TMP36 output = 500 mV offset + 10 mV per °C
  // Temperature (°C) = (Voltage - 0.5) * 100
  float tempC = (voltage - 0.5) * 100.0;

  temperatureC = tempC;
  return tempC;
}


// handles the led patterns
void handleLEDs(int frame){
  for (int LED : ALL_LEDS){
    digitalWrite(LED, LOW);
  }
  if (pattern == "wave") {
    for (int index : LED_PATTERN_WAVE[frame]){
      if (index != -1){
        digitalWrite(ALL_LEDS[index], HIGH);
      }
    }
  }
  else if (pattern == "curtain") {
    for (int index : LED_PATTERN_CURTAIN[frame]){
      if (index != -1){
        digitalWrite(ALL_LEDS[index], HIGH);
      }
    }
  }
  else {
    for (int index : LED_PATTERN_CHECKER[frame]){
      if (index != -1){
        digitalWrite(ALL_LEDS[index], HIGH);
      }
    }
  }
}

void handleLEDsTemp(float temp){
  for (int LED : ALL_LEDS){
    digitalWrite(LED, LOW);
  }
  for (int i = 0; i < 5; i++) {
    if ((i+1)*5 < temp) {
      digitalWrite(ALL_LEDS[5-i], HIGH);
    }
  }
}


// ---------------------------------------------------
// Function: setup()
// Runs once when the ESP32 starts
// ---------------------------------------------------
void setup()
{

  // Start serial communication for debugging
  Serial.begin(115200);

  // Set ADC resolution to 12 bits (0-4095)
  analogReadResolution(12);

  // Increase ADC input range for better measurement
  analogSetPinAttenuation(TEMP_PIN, ADC_11db);

    // Start connecting to WiFi
  WiFi.begin(ssid,password);

  Serial.print("Connecting to WiFi");

  // Wait until connection is established
  while(WiFi.status()!=WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Once connected, print IP address
  Serial.println("");
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // Configure GPIO pin connected to the external LED
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LED5_PIN, OUTPUT);
}

// ---------------------------------------------------
// Function: sendData()
// Posts sensor readings and current LED pattern to flask server via JSON
// ---------------------------------------------------
void sendData() {
  
  HTTPClient http;
  http.begin("http://"+IP_ADDRESS+":5000/data");      // MAKE SURE TO UPDATE WITH YOUR IP
  http.addHeader("Content-Type", "application/json");

  // The 1 in String() limits it to 1 d.p.
  String json = ("{\"temperature\":"+String(temperatureC,1)+",\"led\":\""+ pattern +"\"}");
  
  int httpCode = http.POST(json);
  if (httpCode != 200) {
    Serial.println("HTTP POST failed: " + String(httpCode));
  }
  http.end();
}


// ---------------------------------------------------
// Function: getPattern()
// Receives updated pattern from flask server via JSON
// ---------------------------------------------------
void getPattern() {
  HTTPClient http;
  http.begin("http://"+IP_ADDRESS+":5000/get_led");    // MAKE SURE TO UPDATE WITH YOUR IP

  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    if (payload.indexOf("Wave") != -1) pattern = "wave";
    else if (payload.indexOf("Curtain") != -1) pattern = "curtain";
    else if (payload.indexOf("Checker") != -1) pattern = "checker";
    else if (payload.indexOf("Temperature") != -1) pattern = "temp";
  }
  else {
    Serial.println("HTTP GET failed: " + String(httpCode));
  }
  http.end();
}


// ---------------------------------------------------
// Function: loop()
// Runs continuously after setup()
// ---------------------------------------------------
void loop()
{
  loopIteration++;
  
  if (loopIteration % 500 == 0){
    getPattern(); // Update pattern variable from server buttons
    float temp = readTemperature();

    if (pattern == "temp") {
      handleLEDsTemp(temp);
    }
    else {
      seconds = time (NULL);

      int frames = sizeof(LED_PATTERN_CHECKER) / sizeof(LED_PATTERN_CHECKER[0]);
      if (pattern == "wave") {
        frames = sizeof(LED_PATTERN_WAVE) / sizeof(LED_PATTERN_WAVE[0]);
      }
      else if (pattern == "curtain") {
        frames = sizeof(LED_PATTERN_CURTAIN) / sizeof(LED_PATTERN_CURTAIN[0]);
      }
      handleLEDs(seconds % frames);
    }
    
    // Display info on server
    sendData();
  }
}
