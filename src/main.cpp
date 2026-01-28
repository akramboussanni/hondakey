#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// For ESP8266, use:
// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void showMessage(String msg, int delayMs = 0) {
  if (USE_OLED) {
    display.ssd1306_command(SSD1306_DISPLAYON);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Wake-on-LAN Key");
    
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.println(msg);
    
    display.display();
    
    if (delayMs > 0) {
      delay(delayMs);
      display.clearDisplay();
      display.display();
      display.ssd1306_command(SSD1306_DISPLAYOFF);
    }
  } else {
    Serial.println(msg);
    if (delayMs > 0) delay(delayMs);
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(LED_PIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF); 

  Serial.println();
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); 
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  digitalWrite(LED_PIN, HIGH);
}

void sendWOLPacket() {
  if (String(TARGET_MAC).length() != 17) {
    showMessage("Bad MAC!", 2000);
    return;
  }

  byte mac[6];
  // Parse MAC address
  sscanf(TARGET_MAC, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
         &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

  // Construct Magic Packet: 6 * 0xFF followed by 16 * MAC
  byte magicPacket[102];
  for (int i = 0; i < 6; i++) magicPacket[i] = 0xFF;
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 6; j++) {
      magicPacket[6 + i * 6 + j] = mac[j];
    }
  }

  WiFiUDP udp;
  udp.beginPacket(IPAddress(255, 255, 255, 255), 9);
  udp.write(magicPacket, 102);
  udp.endPacket();
  
  showMessage("Packet Sent!", 2000);
  
  for(int i=0; i<3; i++) {
        digitalWrite(LED_PIN, LOW); delay(100);
        digitalWrite(LED_PIN, HIGH); delay(100);
  }
}

void wakeMachine() {
  showMessage("Waking...");
  
  if(WiFi.status() == WL_CONNECTED) {
    if (USE_LOCAL_WOL) {
      Serial.println("Sending Local WoL Packet...");
      sendWOLPacket();
    } else {
      HTTPClient http;
    
    Serial.print("Sending Wake request to: ");
    Serial.println(SERVER_URL);
    
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");
    
    String payload = "{\"machine_id\": " + String(MACHINE_ID) + ", \"password\": \"" + String(MACHINE_PASSWORD) + "\"}";
    
    Serial.println("Payload: " + payload);
    
    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.println(response);
      
      if (httpResponseCode == 200) {
        showMessage("Success!", 3000);
        for(int i=0; i<3; i++) {
            digitalWrite(LED_PIN, LOW); delay(100);
            digitalWrite(LED_PIN, HIGH); delay(100);
        }
      } else {
        showMessage("Error: " + String(httpResponseCode), 3000);
        digitalWrite(LED_PIN, LOW); delay(1000);
        digitalWrite(LED_PIN, HIGH);
      }
      
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      showMessage("Net Error", 3000);
    }
    
    http.end();
    } 
  } else {
    Serial.println("WiFi Disconnected");
    showMessage("No WiFi", 3000);
  }
}

void loop() {
  if (digitalRead(BUTTON_PIN) == HIGH) { 
    Serial.println("Button Pressed!");
    wakeMachine();
    delay(1000); 
  }
  delay(10);
}