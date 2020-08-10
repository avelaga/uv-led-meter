#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <FastLED.h>

#define NUM_LEDS 10
#define DATA_PIN D4

const char* ssid = "";
const char* password = "";

const char* serverName = "http://api.openuv.io/api/v1/uv?lat=-33.12&lng=-96.69&dt=2018-01-24T10:50:52.283Z";
const char* accessToken = "";

CRGB leds[NUM_LEDS];
unsigned long lastTime = 0;
unsigned long timerDelay = 1800000; // every 30 mins
String sensorReadings;
int uv;

void setup() {
  LEDS.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  LEDS.setBrightness(255);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  updateUV();
}

// fill # of leds corresponding to UV index and increment hue for color scale
void updateLeds() {
  int hue = 0;
  for (int x = 0; x < uv; x++) {
    leds[x] = CHSV(hue, 255, 255);
    hue += 25;
  }
}

void updateUV() {
  if (WiFi.status() == WL_CONNECTED) {
    sensorReadings = httpGETRequest(serverName);
    Serial.println(sensorReadings);
    JSONVar myObject = JSON.parse(sensorReadings);

    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }

    JSONVar result = myObject["result"];
    uv = result["uv"];
    Serial.println(uv);
    updateLeds();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
  lastTime = millis();

}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    updateUV();
  }
}

String httpGETRequest(const char* serverName) {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("x-access-token", accessToken);
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return payload;
}
