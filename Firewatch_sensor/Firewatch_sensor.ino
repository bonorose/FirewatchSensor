/*********
  Hardware Source
  Ver. 0.2a
  
  The following code is used for operating the sensor hardware.

  Team Firewatch, 2018-2019
*********/

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme; // I2C

#define Gas_AOUT  A0

// Network Credentials
const char* ssid = "*******";
const char* password = "*******";

float h, t, p, pin, dp, ppt;
char temperatureFString[6];
char dpString[6];
char humidityString[6];
char pressureString[7];
char pressureInchString[6];

// Web Server on port 80
WiFiServer server(80);

//BOOT
void setup() {
  
  // Serial Comm
  Serial.begin(115200);
  delay(10);
  Wire.begin(D3, D4);
  Wire.setClock(100000);
  
  //WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(2000);

  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
  Serial.println(F("Sensor Operational"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void getWeather() {

  h = bme.readHumidity();
  t = bme.readTemperature();
  dp = t - 0.36 * (100.0 - h);

  p = bme.readPressure() / 100.0F;
  pin = 0.02953 * p;
  dtostrf(t, 5, 1, temperatureFString);
  dtostrf(h, 5, 1, humidityString);
  dtostrf(p, 6, 1, pressureString);
  dtostrf(pin, 5, 2, pressureInchString);
  dtostrf(dp, 5, 1, dpString);
  delay(100);

}

void getGas()
{
  ppt = analogRead(Gas_AOUT);
}


//Main Loop
void loop() {
  // Listenning for new clients
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '\n' && blank_line) {
          getWeather();
          getGas();
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/json");
          client.println("Connection: close");
          client.println();

          client.println("{");
          client.println("\"temperature\":");
          client.printf("\"%s\",", temperatureFString);
          client.println("\"humidity\":");
          client.printf("\"%s\",", humidityString);
          client.println("\"dewPoint\":");
          client.printf("\"%s\",", dpString);
          client.println("\"pressure\":");
          client.printf("\"%s\",", pressureString);
          client.println("\"gasPpt\":");
          client.printf("\"%f\",", ppt);
          client.println("\"hpa\":");
          client.printf("\"%s\"", pressureInchString);
          client.println("}");
          break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
    }
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}
