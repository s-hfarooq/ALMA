#include <WiFi.h>
#include <analogWrite.h>

#include <esp_bt.h>
#include "driver/adc.h"

#define LED_PIN_R_1 32
#define LED_PIN_G_1 33
#define LED_PIN_B_1 25
#define LED_PIN_R_2 5
#define LED_PIN_G_2 17
#define LED_PIN_B_2 18
#define PORT 10000

const char* ssid = "ssid";
const char* password = "password";

WiFiServer server(PORT);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN_R_1, OUTPUT);
  pinMode(LED_PIN_G_1, OUTPUT);
  pinMode(LED_PIN_B_1, OUTPUT);
  pinMode(LED_PIN_R_2, OUTPUT);
  pinMode(LED_PIN_G_2, OUTPUT);
  pinMode(LED_PIN_B_2, OUTPUT);

  delay(10);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    WiFi.begin(ssid, password);
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Turning off things...");
  btStop();
  esp_bt_controller_disable();
  adc_power_off();

  server.begin();
}

// Get RGB values from input string
void getValues(String currentLine, int * rCol, int * gCol, int * bCol) {
  // Parse string for int values desired
  String temp = currentLine.substring(0, currentLine.indexOf(' '));
  currentLine = currentLine.substring(currentLine.indexOf(' '));
  currentLine.trim();
  *rCol = temp.toInt();

  temp = currentLine.substring(0, currentLine.indexOf(' '));
  currentLine = currentLine.substring(currentLine.indexOf(' '));
  currentLine.trim();
  *gCol = temp.toInt();

  temp = currentLine.substring(0, currentLine.indexOf(' '));
  *bCol = temp.toInt();

  // Ensure values are at minimum 0
  *rCol = max(0, *rCol);
  *gCol = max(0, *gCol);
  *bCol = max(0, *bCol);

  // Ensure values are not above 255
  while(*rCol > 255)
    *rCol %= 255;
  while(*gCol > 255)
    *gCol %= 255;
  while(*bCol > 255)
    *bCol %= 255;

  return;
}

void loop() {
  // Listen for clients
  WiFiClient client = server.available();

  // Run if a client has connected
  if(client) {
    Serial.println("New Client.");
    String currentLine = "";

    // Loop while client is connected
    while(client.connected()) {
      // Read bytes from client
      if(client.available()) {
        char c = toupper(client.read());
        Serial.write(c);

        currentLine += c;

        // Check if request is a valid option
        if(currentLine.endsWith("ON")) {
          digitalWrite(LED_PIN_R_1, HIGH);
          digitalWrite(LED_PIN_G_1, HIGH);
          digitalWrite(LED_PIN_B_1, HIGH);
          digitalWrite(LED_PIN_R_2, HIGH);
          digitalWrite(LED_PIN_G_2, HIGH);
          digitalWrite(LED_PIN_B_2, HIGH);
          client.write("Turned to high");
          currentLine = "";
        } else if(currentLine.endsWith("OFF")) {
          digitalWrite(LED_PIN_R_1, LOW);
          digitalWrite(LED_PIN_G_1, LOW);
          digitalWrite(LED_PIN_B_1, LOW);
          digitalWrite(LED_PIN_R_2, LOW);
          digitalWrite(LED_PIN_G_2, LOW);
          digitalWrite(LED_PIN_B_2, LOW);
          client.write("Turned to low");
          currentLine = "";
        } else if(currentLine.endsWith("2COL")) {
          int rCol = 0, gCol = 0, bCol = 0;
          getValues(currentLine, &rCol, &gCol, &bCol);

          analogWrite(LED_PIN_R_2, rCol);
          analogWrite(LED_PIN_G_2, gCol);
          analogWrite(LED_PIN_B_2, bCol);
          client.write("Set new 2col");
          currentLine = "";
        } else if(currentLine.endsWith("1COL")) {
          int rCol = 0, gCol = 0, bCol = 0;
          getValues(currentLine, &rCol, &gCol, &bCol);

          analogWrite(LED_PIN_R_1, rCol);
          analogWrite(LED_PIN_G_1, gCol);
          analogWrite(LED_PIN_B_1, bCol);
          client.write("Set new 1col");
          currentLine = "";
        } else if(currentLine.endsWith("BOTH")) {
          int rCol = 0, gCol = 0, bCol = 0;
          getValues(currentLine, &rCol, &gCol, &bCol);

          analogWrite(LED_PIN_R_1, rCol);
          analogWrite(LED_PIN_G_1, gCol);
          analogWrite(LED_PIN_B_1, bCol);
          analogWrite(LED_PIN_R_2, rCol);
          analogWrite(LED_PIN_G_2, gCol);
          analogWrite(LED_PIN_B_2, bCol);
          client.write("Set both col");
          currentLine = "";
        }
      }
    }

    // Close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
