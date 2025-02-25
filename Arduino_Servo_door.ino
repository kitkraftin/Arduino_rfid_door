#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// OLED Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RFID Settings
#define RST_PIN 9
#define SS_PIN 10
MFRC522 rfid(SS_PIN, RST_PIN);

// Servo Settings
Servo myServo;
int servoPin = 3;
int lockedPos = 0;
int unlockedPos = 100;

// Authorized RFID UID
byte authorizedUID[4] = { 0xE0, 0x6D, 0xAE, 0x10 }; // Change this to your card's UID

void setup() {
  Serial.begin(9600);

  // OLED Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found"));
    while (true);
  }

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  // Servo
  myServo.attach(servoPin);
  myServo.write(lockedPos);

  // Initial Display
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("LOCKED");
  display.setCursor(0,20);
  display.print("SCAN CARD");
  display.display();
}

void loop() {
  // Look for new RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Display Scanning Message
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("SCANNING...");
  display.display();
  delay(500);

  // Check UID
  if (checkUID()) {
    // Correct UID - Unlock Door
    myServo.write(unlockedPos);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("UNLOCKED");
    display.display();
    delay(5000); // Keep the door open for 5 seconds

    // Lock Door Again
    myServo.write(lockedPos);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("LOCKED");
    display.setCursor(0,20);
    display.print("SCAN CARD");
    display.display();
  } 
  else {
    // Incorrect UID
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("DENIED");
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setCursor(0,0);
    display.print("LOCKED");
    display.setCursor(0,20);
    display.print("SCAN CARD");
    display.display();
  }
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// Function to Check UID
bool checkUID() {
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != authorizedUID[i]) {
      return false;
    }
  }
  return true;
}
