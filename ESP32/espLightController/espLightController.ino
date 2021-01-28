#include <WiFi.h>
#include <analogWrite.h>
#include <esp_bt.h>
#include "driver/adc.h"

#define LED_PIN_R_1 32
#define LED_PIN_G_1 33
#define LED_PIN_B_1 25
#define LED_PIN_R_2 18
#define LED_PIN_G_2 5
#define LED_PIN_B_2 17
#define PORT 10000

const char* ssid = "ssid";
const char* password = "password";

int oR, oG, oB;

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

  oR = 0;
  oG = 0;
  oB = 0;

  Serial.println("Starting server...");
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

void fadeToNewCol(int newR, int newG, int newB, int duration, int type) {
  Serial.println("Fading...");
  int rDiff = newR - oR;
  int gDiff = newG - oG;
  int bDiff = newB - oB;
  int delayAmnt = 20;
  int steps = duration / delayAmnt;
  int rV, gV, bV;

  for(int i = 0; i < steps - 1; i++) {
    rV = oR + (rDiff * i / steps);
    gV = oG + (gDiff * i / steps);
    bV = oB + (bDiff * i / steps);

    displayCol(rV, gV, bV, type);

    delay(delayAmnt);
  }

  displayCol(newR, newG, newB, type);
  Serial.println("Fade complete");
  return;
}

void displayCol(int r, int g, int b, int type) {
  if(type == 0 || type == 1) {
    analogWrite(LED_PIN_R_1, r);
    analogWrite(LED_PIN_G_1, g);
    analogWrite(LED_PIN_B_1, b);
  }

  if(type == 0 || type == 2) {
    analogWrite(LED_PIN_R_2, r);
    analogWrite(LED_PIN_G_2, g);
    analogWrite(LED_PIN_B_2, b);
  }

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
        if(currentLine.endsWith("1COL")) {
          int rCol = 0, gCol = 0, bCol = 0;
          getValues(currentLine, &rCol, &gCol, &bCol);

          if(currentLine.indexOf("FIN") == -1)
            displayCol(rCol, gCol, bCol, 1);
          else
            fadeToNewCol(rCol, gCol, bCol, 150, 1);

          oR = rCol;
          oG = gCol;
          oB = bCol;

          client.write("Set new 1col");
          currentLine = "";
        } else if(currentLine.endsWith("2COL")) {
          int rCol = 0, gCol = 0, bCol = 0;
          getValues(currentLine, &rCol, &gCol, &bCol);

          if(currentLine.indexOf("FIN") == -1)
            displayCol(rCol, gCol, bCol, 2);
          else
            fadeToNewCol(rCol, gCol, bCol, 150, 2);

          oR = rCol;
          oG = gCol;
          oB = bCol;

          client.write("Set new 2col");
          currentLine = "";
        } else if(currentLine.endsWith("BOTH")) {
          int rCol = 0, gCol = 0, bCol = 0;
          getValues(currentLine, &rCol, &gCol, &bCol);

          if(currentLine.indexOf("FIN") == -1)
            displayCol(rCol, gCol, bCol, 0);
          else
            fadeToNewCol(rCol, gCol, bCol, 150, 0);

          oR = rCol;
          oG = gCol;
          oB = bCol;

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
