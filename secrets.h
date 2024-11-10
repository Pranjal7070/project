#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <DHT.h>
#include <WiFiNINA.h> // For WiFi connection

// Wi-Fi credentials
const char* ssid = "Sandeep";
const char* password = "sweethome";

// IFTTT settings
const char* iftttKey = "kqQqtzuaCdaFzU4GEZrVEP2XZ77WYZ_Xk5hlFBC4PeB";
const char* iftttEvent = "sensor_notification";

// BMP180 Sensor
Adafruit_BMP085_Unified bmp;

// DHT22 Sensor
#define DHTPIN 2  // Digital pin for DHT22
#define DHTTYPE DHT22  // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// MQ135 Sensor
int mq135Pin = A0; // Analog pin for MQ135

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address might be 0x27 or 0x3F

void setup() {
  Serial.begin(9600);

  // Initialize BMP180
  if (!bmp.begin()) {
    Serial.println("Couldn't find the BMP180 sensor");
    while (1);
  }

  // Initialize DHT22
  dht.begin();

  // Setup MQ135
  pinMode(mq135Pin, INPUT);

  // Initialize LCD
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Weather Station");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
}

void loop() {
  // Read BMP180 data
  sensors_event_t event;
  bmp.getEvent(&event);
  float pressure = event.pressure ? event.pressure : 0;
  float bmpTemperature = 0;
  bmp.getTemperature(&bmpTemperature);

  // Read DHT22 data
  float humidity = dht.readHumidity();
  float dhtTemperature = dht.readTemperature();

  // Read MQ135 data
  int mq135Value = analogRead(mq135Pin);

  // Display on Serial and LCD
  Serial.print("BMP Temp: "); Serial.println(bmpTemperature);
  Serial.print("DHT Temp: "); Serial.println(dhtTemperature);
  Serial.print("Humidity: "); Serial.println(humidity);
  Serial.print("Pressure: "); Serial.println(pressure);
  Serial.print("MQ135 Air Quality: "); Serial.println(mq135Value);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: "); lcd.print(dhtTemperature); lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: "); lcd.print(humidity); lcd.print(" %");

  delay(2000);  // Display interval

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pressure: "); lcd.print(pressure); lcd.print(" hPa");
  lcd.setCursor(0, 1);
  lcd.print("Air Quality: "); lcd.print(mq135Value);

  delay(2000);

  // Send IFTTT Notification if thresholds are met
  if (dhtTemperature > 30 || mq135Value > 600) {
    sendIFTTTNotification(dhtTemperature, humidity, mq135Value);
  }
}

// Function to send notification to IFTTT
void sendIFTTTNotification(float temperature, float humidity, int airQuality) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    const char* server = "maker.ifttt.com";
    if (client.connect(server, 80)) {
      // Create the HTTP request
      String url = "/trigger/";
      url += iftttEvent;
      url += "/with/key/";
      url += iftttKey;
      url += "?value1=" + String(temperature);
      url += "&value2=" + String(humidity);
      url += "&value3=" + String(airQuality);

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + server + "\r\n" +
                   "Connection: close\r\n\r\n");
      delay(100);  // Wait for the request to be sent
      Serial.println("IFTTT Notification Sent");
    } else {
      Serial.println("Connection to IFTTT failed");
    }
    client.stop();
  } else {
    Serial.println("WiFi not connected");
  }
}