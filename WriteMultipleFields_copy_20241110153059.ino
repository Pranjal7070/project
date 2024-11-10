#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <DHT.h>
#include <WiFiNINA.h>  // For Wi-Fi on Arduino Nano 33 IoT

// Wi-Fi credentials
const char* ssid = "Sandeep";  
const char* password = "sweethome";  

// IFTTT settings
const char* iftttKey = "kqQqtzuaCdaFzU4GEZrVEP2XZ77WYZ_Xk5hlFBC4PeB";  
const char* iftttEvent = "sensor_notification";  // Event name used to trigger IFTTT

// BMP180 Sensor initialization
Adafruit_BMP085_Unified bmp;

// DHT22 Sensor initialization
#define DHTPIN 2  // Define pin for DHT22 sensor (DHT data pin connected to digital pin 2)
#define DHTTYPE DHT22  // Define the sensor type (DHT22)
DHT dht(DHTPIN, DHTTYPE);  // Initialize the DHT sensor

// MQ135 Sensor pin
int mq135Pin = A0;  // Define analog pin for MQ135 (Air quality sensor)


// Wi-Fi connection setup function
void connectToWiFi() {
  WiFi.begin(ssid, password);  // Attempt to connect to Wi-Fi with SSID and password
  while (WiFi.status() != WL_CONNECTED) {  // Keep checking the connection status
    delay(1000);  // Wait for 1 second before retrying
    Serial.println("Connecting to WiFi...");  // Print the status to Serial Monitor
  }
  Serial.println("Connected to WiFi!");  // Once connected, print this message
}

void setup() {
  Serial.begin(9600);  // Start serial communication at 9600 baud rate
  connectToWiFi();  // Call the Wi-Fi connection function

  // Initialize BMP180 sensor
  if (!bmp.begin()) {  // Check if the sensor was successfully initialized
    Serial.println("Couldn't find BMP180 sensor");  // Print an error if the sensor is not found
    while (1);  // Halt the program
  }
  
  dht.begin();  // Initialize DHT22 sensor
  pinMode(mq135Pin, INPUT);  // Set MQ135 sensor pin as input
}

void loop() {
  // Read BMP180 sensor data (Pressure and Temperature)
  sensors_event_t event;
  bmp.getEvent(&event);  // Get pressure data
  float pressure = event.pressure ? event.pressure : 0;  // If pressure is available, use it, otherwise set to 0
  float bmpTemperature;  // Declare a variable for BMP180 temperature
  bmp.getTemperature(&bmpTemperature);  // Get temperature from BMP180 sensor

  // Read DHT22 sensor data (Humidity and Temperature)
  float humidity = dht.readHumidity();  // Read humidity from DHT22
  float dhtTemperature = dht.readTemperature();  // Read temperature from DHT22

  // Read MQ135 sensor data (Air Quality - Analog)
  int mq135Value = analogRead(mq135Pin);  // Read analog value from MQ135

  // Print sensor data to Serial Monitor
  Serial.print("Pressure: "); Serial.println(pressure);  // Print pressure
  Serial.print("BMP Temp: "); Serial.println(bmpTemperature);  // Print BMP180 temperature
  Serial.print("DHT Temp: "); Serial.println(dhtTemperature);  // Print DHT22 temperature
  Serial.print("Humidity: "); Serial.println(humidity);  // Print humidity
  Serial.print("Air Quality (MQ135): "); Serial.println(mq135Value);  // Print air quality from MQ135

  // Check if temperature or air quality exceeds the set thresholds
  if (dhtTemperature > 25 || mq135Value > 300) {  // If temperature is above 25°C or air quality value exceeds 300
    sendIFTTTNotification(dhtTemperature, humidity, mq135Value);  // Trigger IFTTT notification
  }

  delay(2000);  // Wait for 2 seconds before the next loop iteration
}

// Function to send notification to IFTTT
void sendIFTTTNotification(float temperature, float humidity, int airQuality) {
  if (WiFi.status() == WL_CONNECTED) {  // Check if Wi-Fi is connected
    WiFiClient client;  // Create a Wi-Fi client
    const char* server = "maker.ifttt.com";  // Define the server URL (IFTTT Webhooks server)
    
    // Attempt to connect to the IFTTT Webhooks server
    if (client.connect(server, 80)) {
      // Construct the URL for the Webhooks request
      String url = "/trigger/";
      url += iftttEvent;  // Add the event name to the URL
      url += "/with/key/";
      url += iftttKey;  // Add the IFTTT key to the URL
      url += "?value1=" + String(temperature);  // Append the temperature value
      url += "&value2=" + String(humidity);  // Append the humidity value
      url += "&value3=" + String(airQuality);  // Append the air quality value

      // Send HTTP GET request to IFTTT Webhooks
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + server + "\r\n" +  // Set the host header
                   "Connection: close\r\n\r\n");  // Close the connection after sending request

      delay(100);  // Wait briefly to allow for the request to be sent
      Serial.println("IFTTT Notification Sent");  // Print success message to Serial Monitor
    } else {
      // If the connection to IFTTT fails, print an error message
      Serial.println("Connection to IFTTT failed");
    }
    client.stop();  // Stop the Wi-Fi client
  } else {
    // If Wi-Fi is not connected, print this message
    Serial.println("WiFi not connected");
  }
}
