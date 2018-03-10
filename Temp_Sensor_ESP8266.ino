#include "ESP8266WiFi.h"
#include <Wire.h>
#include <Adafruit_MCP9808.h>
#include "Settings.h"

// Wifi Client
WiFiClient client;

// Create MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

// Wifi settings
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// ThingSpeak settings
const int channelID = TS_CHANNEL_ID;
String writeAPIKey = TS_WRITE_KEY;
const char* server = "api.thingspeak.com";

// Stop for 20 seconds at the end of each loop
// Free Thingsspeak accounts have a 15 second rate limit
const int postingInterval = 20 * 1000;

void setup() {
  // Start Serial
  Serial.begin(115200);

  // Set the pins used for I2C on your ESP8266 for the temp sensor
  Wire.pins(D4, D5); //D4=SDA, D5=SCL on ESP8266

  // Start the sensor
  if (!tempsensor.begin()) {
    Serial.println("Couldn't find MCP9808!");
    while (1);
  }

  // Start Wifi and wait until we're connected
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {

  // Wake up MSP9808 - power consumption ~200 mikro Ampere
  tempsensor.wake();
  float tempValue = tempsensor.readTempC();
  tempsensor.shutdown();

  // Measure Signal Strength (RSSI) of Wi-Fi connection
  long rssi = WiFi.RSSI();

  Serial.print("Deg. C: ");
  Serial.print(tempValue);
  Serial.print("\tRSSI: ");
  Serial.println(rssi);
  
  if (client.connect(server, 80)) {
    reportToThingspeak(tempValue, rssi);
  }
  
  client.stop();
  delay(postingInterval);
}

void reportToThingspeak(float tmp, long rssi) {
    String body = "field1=";
    body += String(rssi);
    body += "&field2=";
    body += String(tmp, 2); // Two decimal places

    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: ESP8266 (nothans)/1.0");
    client.println("Connection: close");
    client.println("X-THINGSPEAKAPIKEY: " + writeAPIKey);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(body.length()));
    client.println("");
    client.print(body);
}

