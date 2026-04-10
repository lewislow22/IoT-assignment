// Ex11.cpp
// COM3505 – Exercise 11
// Exercise 11: ESP32 Temperature Sensor Web Monitor
// Sensor: TMP36
// This program reads temperature from a TMP36 sensor
// connected to the ESP32 and hosts a web page that
// displays the temperature in real time.

// Include WiFi library for network connection
#include <WiFi.h>

// Include WebServer library to create a small web server
// running directly on the ESP32
#include <WebServer.h>

// include the time for the animations
#include <time.h>

// JSON
#include <HTTPClient.h>
// #include <ArduinoJson.h>


// ---------------- Wi-Fi credentials ----------------
// SSID and password of the wireless network.
// These must match the Wi-Fi network you want the ESP32 to connect to.
const char* ssid = "BTHub6-MC2C";
const char* password = "AVxMXvh9pRew"; // CHANGE THIS TO YOUR WIFI PASSWORD

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
const int LED_PATTERN[6][2] = {
  {0, 1},
  {1, 2},
  {2, 3},
  {3, 4},
  {4, 5},
  {5, 0}
};

// Loop counter for looping the LED patterns
unsigned long loopIteration = 0;

// keeps track of what frame of the loop patetrn should be shown
int frame = 0;

// set up the timer for the animations
time_t seconds;

// ---------------- ADC Settings ----------------

// Maximum ADC value when using 12-bit resolution
// 2^12 − 1 = 4095
#define ADC_MAX 4095.0

// Reference voltage of ESP32 analog input (approx. 3.3V)
#define VREF 3.3



// Create a web server object listening on port 80
// Port 80 is the default HTTP port used by web browsers
WebServer server(80);



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
  return tempC;
}



// ---------------------------------------------------
// Function: handleRoot()
// This function sends the main web page to the browser
// when a user visits the ESP32 IP address.
// ---------------------------------------------------
void handleRoot()
{

  // HTML webpage stored as a raw string literal
  // This page displays the temperature and updates
  // automatically every 2 seconds using JavaScript.
  String html = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
<title>ESP32 Temperature Monitor</title>

<style>

/* Page styling */

body{
font-family: Arial;
text-align:center;
background:#f4f4f4;
}

/* Card-style container */

.card{
background:white;
width:300px;
margin:auto;
padding:20px;
border-radius:10px;
box-shadow:0 0 10px rgba(0,0,0,0.2);
}

/* Temperature display style */

.temp{
font-size:40px;
color:#0077cc;
}

</style>

</head>

<body>

<h2>ESP32 Temperature Monitor</h2>

<div class="card">

Temperature

<div class="temp" id="temp">-- °C</div>

</div>


<script>

// Function to request temperature data from ESP32
function updateTemp()
{

// Fetch data from /temp endpoint
fetch("/temp")

.then(response => response.text())

.then(data => {

// Update the temperature displayed on the page
document.getElementById("temp").innerHTML = data + " °C";

});

}

// Call updateTemp every 2 seconds
setInterval(updateTemp,2000);

// Also run immediately when page loads
updateTemp();

</script>

</body>

</html>
)rawliteral";

  // Send the webpage to the browser
  server.send(200,"text/html",html);
}



// ---------------------------------------------------
// Function: handleTemp()
// Sends the current temperature value to the browser
// when the webpage requests "/temp"
// ---------------------------------------------------
void handleTemp()
{
  // Read the current temperature from the sensor
  temperatureC = readTemperature();

  // Send the temperature value as plain text
  // The "1" means show 1 decimal place
  server.send(200,"text/plain",String(temperatureC,1));
}


// handles the led patterns
void handleLEDs(int frame){
  for (int LED : ALL_LEDS){
    digitalWrite(LED, LOW);
  }
  for (int index : LED_PATTERN[frame]){
    digitalWrite(ALL_LEDS[index], HIGH);
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


  // Define web server routes

  // When browser visits "/"
  server.on("/",handleRoot);

  // When browser requests "/temp"
  server.on("/temp",handleTemp);


  // Start the web server
  server.begin();

  // Configure GPIO pin connected to the external LED
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LED5_PIN, OUTPUT);
}

void sendData() {
  HTTPClient http;
  http.begin("http://127.0.0.1/:5000/data");
  http.addHeader("Content-Type", "application/json");

  // StaticJsonDocument<200> doc;
  // doc["temperature"] = readTemperature();

  // String json;
  // serializeJson(doc, json);

  String json = ("{\"temperature\":"+String(readTemperature())+"}");

  http.POST(json);
  http.end();
}


// ---------------------------------------------------
// Function: loop()
// Runs continuously after setup()
// ---------------------------------------------------
void loop()
{
  loopIteration++;

  // Handle incoming client requests
  // (e.g., browser requests for webpage or temperature)
  server.handleClient();
  
  if (loopIteration % 500 == 0){
    int frames = sizeof(LED_PATTERN) / sizeof(LED_PATTERN[0]);

    seconds = time (NULL);

    handleLEDs(seconds % frames);
    sendData();
  }
}
