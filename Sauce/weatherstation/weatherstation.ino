#include <SPI.h>
#include <Ethernet.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

#define DHTPIN 8     // what pin we're connected to

#define DHTTYPE DHT22   // DHT 22  (AM2302)

//RTC_Millis RTC;
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP085 bmp;

// Enter a MAC address and IP address for your controller below.
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  IPAddress ip(10, 17, 20, 15);
  IPAddress dns1(10, 17, 1, 21);
  IPAddress gateway(10, 17, 0, 254);
  IPAddress subnet(255, 255, 0, 0);
// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  Serial.begin(9600);

  //initiate the various sensors and components
  Wire.begin();
  wdt_enable(WDTO_2S);
  dht.begin();
  bmp.begin();
 
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	while (1) {}
  }

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, dns1, gateway, subnet);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float th = dht.readTemperature();
  float tp = bmp.readTemperature();
  float t = (dht.readTemperature()+bmp.readTemperature())/2;
  float p = bmp.readPressure();
  Serial.print("Pressure = ");
  Serial.print(p);
  Serial.print(" Pa");
  Serial.print(" \t");
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Humid. Temperature: ");
    Serial.print(th);
    Serial.println(" *C");
    Serial.print("Press. Temperature 2: ");
    Serial.print(tp);
    Serial.println(" *C");
    Serial.print("Av. Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
  }
    wdt_reset();
    delay(500);
 
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
        client.println();
          client.println ("{");
          client.print("\"Humid_Temperature\": ");
          client.print(th);
          client.println(",");
          client.print("\"Press_Temperature\": ");
          client.print(tp);
          client.println(",");
          client.print("\"Av_Temperature\": ");
          client.print(t);
          client.println(",");
          client.print("\"Humidity\": ");
          client.print(h);
          client.println(",");
          client.print("\"Pressure\": ");
          client.println(p);
          client.println("}");
          
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}
