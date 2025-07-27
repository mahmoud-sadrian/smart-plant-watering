#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// #*#*#*#*#*# Important Caution #*#*#*#*#*#
// Dont touch relay module specially it's esp cause short connection.

// ****Relay power supply Wiring:****
// null (N) → COM terminal of Relay module (White wire)
// NC terminal → connected to a pole of electric fuset (Blue wire)


// ****Caution:****
// Because of electrical induction by L (faz) wire 
// we attempt to plug the N wire of 220V AC to COM terminal of relay


// ****Relay module Activation:****
// because my relay is low active these instructions means:
// digitalWrite(pin, HIGH); == OFF
// digitalWrite(pin, LOW); == ON

// ****Power supply switching module wiring:****
// This module convert 220v AC to 5v DC.
// It has I/O. The pins(Terminals) that are near to a Green capacitor is Input which are connect to 220v AC
// And the pins that are near to stand electrolitic capacitor is Output which are provide 5v DC 
// cosider that 5v Outputs terminals have a schematic sign - and + which are surrounded by a circle which means they are 5v Outputs.

// ****Relay module power supply:****
// When we take the relay module in hand so that the terminals are placed in front:
// Right socket of two terminal's sockets is vcc and the Left one is GND

// ****Relay pinout wiring:****
// When we take the relay module in hand so that the terminals are placed in front:
// Right most socket of three terminal's sockets is NO 
// middle one is COM
// Left most one is NC
// vcc → 3.3v
// GND → GND
// IN → GPIO (that I declare below as relayPin)

const unsigned int relayPin = 0U;

const char* ssid = "****";
const char* password = "****";

WiFiServer server(80);

unsigned int dayOfWeek, hour, minute, second;

/*If the relay turns on this variable will be true for make 60 second delay 
because if relay be turn on in the early seconds and then be on for 30 seconds
after this delay loop will be repeated and minute is same as the time that relay 
being on and in this time relay will be on again. This var make 60 seconds delay 
to solve this problem. ↓*/
bool intervalDelay = false;

bool userRequestedOff = false; // Flag to track user request to turn relay off
bool relayState = HIGH; // Initial state is OFF

const char* htmlContent = R"(
<!DOCTYPE HTML>
<html>
<head><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Pasio Watering</title>
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
.button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;
text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
</style></head>
<body>
<h1 align=center>Pasio Watering</h1>
<h2 align=center>Status: %s</h2>
<br><br>
<a href="/RELAY=ON"><button class="button">TURN ON</button></a><br><br>
<a href="/RELAY=OFF"><button class="button">TURN OFF</button></a><br>
</body></html>
)";

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  RelayOff();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000U);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");

  server.begin();
  Serial.println("Server started");
  Serial.println("Use this URL to connect:");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
} // end of setup

void loop() { //*********************************************************************************
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client");

    String request = client.readStringUntil('\r');
    Serial.println(request);

    if (request.indexOf("/RELAY=ON") != -1) {
      RelayOn();
      userRequestedOff = false; // Reset user request flag
    } else if (request.indexOf("/RELAY=OFF") != -1) {
      RelayOff();
      userRequestedOff = true; // Set user request flag
    }

    client.flush();

    String statusMessage = (relayState == LOW) ? "ON" : "OFF";
    String response = String(htmlContent);
    response.replace("%s", statusMessage);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println(response);

    client.stop();
    Serial.println("Client disconnected");
  }

  GetTime();

  // Check if the user requested to turn the relay off
  if (userRequestedOff) {
    RelayOff();
  }
  
  if ( (dayOfWeek == 2U || dayOfWeek == 6U) && hour == 7U && minute == 0U ) {
    RelayOn();
    delay(17000U); // delay for 17 seconds
    intervalDelay = true;
  }
  
  if(intervalDelay){
    RelayOff();
    delay(62000U); //because the delay time is so short, in the next cycle time is hit again
    intervalDelay = false;
  }
} // end of loop *********************************************************************************


/*This function curl a time API and fetch day of week, hour, minute 
and second and then asign them to it's each global variables*/
void GetTime() {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, "http://worldtimeapi.org/api/timezone/Asia/Tehran");

  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    dayOfWeek = doc["day_of_week"]; //Extract day of weel from JSON

    //Fetch (Extract) Time(hour, minute, second) from JSON and search time elements in it's string position
    const char* datetime = doc["datetime"];
    hour = atoi(datetime + 11); //convert hour string to int and store it in "hour" var
    minute = atoi(datetime + 14); //convert minute string to int and store it in "minute" var
    second = atoi(datetime + 17); //convert second string to int and store it in "second" var


    //print hour, minute, second in Serial Monitor (HH:MM:SS)
    Serial.print("Day of the week: ");
    Serial.println(dayOfWeek);
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(second);
  } else {
    Serial.println("Error on HTTP request");
  }

  http.end();
} // end of GetTime

void RelayOn() {
  digitalWrite(relayPin, LOW);
  relayState = LOW;
}

void RelayOff() {
  digitalWrite(relayPin, HIGH);
  relayState = HIGH;
}
