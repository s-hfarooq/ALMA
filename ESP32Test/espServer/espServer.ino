#include <WiFi.h>
#include <analogWrite.h>

#define LED_PIN_R 32
#define LED_PIN_G 33
#define LED_PIN_B 25

const char* ssid = "ssid";
const char* password = "password";

WiFiServer server(10000);

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN_R, OUTPUT);
    pinMode(LED_PIN_G, OUTPUT);
    pinMode(LED_PIN_B, OUTPUT);

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
    
    server.begin();
}

void loop(){
 WiFiClient client = server.available();   // listen for incoming clients

  if(client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    
    while(client.connected()) {            // loop while the client's connected
      if(client.available()) {             // if there's bytes to read from the client,
        char c = toupper(client.read());             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        
        currentLine += c;

        // Check to see if the client request was "GET /H" or "GET /L":
        if(currentLine.endsWith("ON")) {
          digitalWrite(LED_PIN_R, HIGH);
          digitalWrite(LED_PIN_G, HIGH);
          digitalWrite(LED_PIN_B, HIGH);
          client.write("Turned to high");
          currentLine = "";
        } else if(currentLine.endsWith("OFF")) {
          digitalWrite(LED_PIN_R, LOW);
          digitalWrite(LED_PIN_G, LOW);
          digitalWrite(LED_PIN_B, LOW);
          client.write("Turned to low");
          currentLine = "";
        } else if(currentLine.endsWith("COL")) {
          int rCol = 0, gCol = 0, bCol = 0;
          String temp = currentLine.substring(0, currentLine.indexOf(' '));
          currentLine = currentLine.substring(currentLine.indexOf(' '));
          currentLine.trim();
          rCol = temp.toInt();

          temp = currentLine.substring(0, currentLine.indexOf(' '));
          currentLine = currentLine.substring(currentLine.indexOf(' '));
          currentLine.trim();
          gCol = temp.toInt();
          
          temp = currentLine.substring(0, currentLine.indexOf(' '));
          bCol = temp.toInt();

          rCol = max(0, rCol);
          gCol = max(0, gCol);
          bCol = max(0, bCol);

          while(rCol > 255)
            rCol %= 255;
          while(gCol > 255)
            gCol %= 255;
          while(bCol > 255)
            bCol %= 255;

          analogWrite(LED_PIN_R, rCol);
          analogWrite(LED_PIN_G, gCol);
          analogWrite(LED_PIN_B, bCol);
          client.write("Set new col");
          currentLine = "";
        }
      }
    }
    
    // Close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
