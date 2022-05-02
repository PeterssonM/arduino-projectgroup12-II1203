#include <WiFi.h>
#include <ESP32Ping.h>


const char* ssid = "woohaaa";
const char* password =  "1234567890";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password); 
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  bool success = Ping.ping("www.google.com", 3);
  if(!success){
    Serial.println("Ping failed");
    return;
  }
 
  Serial.println("Ping succesful.");
}

void loop() {}
