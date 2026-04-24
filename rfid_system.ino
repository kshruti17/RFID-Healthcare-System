#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// --- USER SETTINGS ---
const char* ssid = "Vivo"; // Change this
const char* password = "12345678"; // Change this
const char* host = "10.149.220.236"; // Change to your Laptop's WiFi IP
// --- PINS (ESP32 Specific) ---
#define SS_PIN 5 // ESP32 GPIO 5
#define RST_PIN 4 // ESP32 GPIO 4
// --- OBJECTS ---
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Address 0x27 or 0x3F
void setup() {
Serial.begin(115200); // ESP32 uses faster serial by default
// 1. LCD SETUP
Wire.begin(); // Uses default SDA=21, SCL=22
lcd.init();
lcd.backlight();
lcd.setCursor(0,0);
lcd.print("Connecting WiFi");
// 2. WIFI SETUP
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print("
.
");
lcd.print("
.
");
}
Serial.println("\nWiFi Connected!");
Serial.print("ESP32 IP: ");
Serial.println(WiFi.localIP());
lcd.clear();
lcd.print("WiFi OK!");
lcd.setCursor(0,1);
lcd.print(WiFi.localIP());
delay(2000);
// 3. RFID SETUP
SPI.begin(); // Uses default SCK=18, MISO=19, MOSI=23
rfid.PCD_Init();
lcd.clear();
lcd.print(" HOSPITAL ");
lcd.setCursor(0,1);
lcd.print(" Scan Patient ");
Serial.println("System Ready.
");
}
void loop() {
// Check for new card
if (!rfid.PICC_IsNewCardPresent()) return;
if (!rfid.PICC_ReadCardSerial()) return;
// Read UID
String cardUID = "";
for (byte i = 0; i < rfid.uid.size; i++) {
cardUID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
cardUID += String(rfid.uid.uidByte[i], HEX);
}
cardUID.toUpperCase();
Serial.println("Scanned: " + cardUID);
lcd.clear();
lcd.print("Checking DB...
");
sendData(cardUID);
// Reset UI
delay(3000);
lcd.clear();
lcd.print(" HOSPITAL ");
lcd.setCursor(0,1);
lcd.print(" Scan Patient ");
rfid.PICC_HaltA();
rfid.PCD_StopCrypto1();
}
void sendData(String uid) {
if(WiFi.status() == WL_CONNECTED){
HTTPClient http;
// Construct URL: http://192.168.1.5/api.php?mode=update&uid=XXYY
String url = "http://" + String(host) + "/api.php?mode=update&uid=" + uid;
Serial.print("Requesting: ");
Serial.println(url);
http.begin(url);
int httpCode = http.GET();
if (httpCode > 0) {
String payload = http.getString();
Serial.println("Response: " + payload);
// PARSE RESPONSE FOR LCD
lcd.clear();
if (payload.indexOf("Patient Found:") >= 0) {
int nameStart = payload.indexOf("Patient Found:") + 14;
String name = payload.substring(nameStart);
// Remove any extra newlines
name.trim();
lcd.print("ACCESS GRANTED");
lcd.setCursor(0,1);
lcd.print(name.substring(0, 16));
} else {
lcd.print("UNKNOWN TAG");
lcd.setCursor(0,1);
lcd.print("Add to DB -->");
}
} else {
Serial.println("HTTP Error");
lcd.clear();
lcd.print("Server Error");
}
http.end();
} else {
Serial.println("WiFi Disconnected");
lcd.clear();
lcd.print("WiFi Lost");
}
}