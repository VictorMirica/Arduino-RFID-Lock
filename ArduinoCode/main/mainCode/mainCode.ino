#include <Keypad.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// RFID variables
MFRC522 mfrc522(10, 9); // SDA, SCK

// Keypad variables
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {A0, A1, A2, A3};  //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 3, 2, 1}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LCD variables
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 columns and 2 rows

// Servo variables
Servo servo;
const int unlockPosition = 90;
const int lockPosition = 0;  

// Password variables
const String password = "1234";
String enteredPassword = "";

// Timing variables
const unsigned long unlockDuration = 3000; // Unlock duration in milliseconds
unsigned long unlockStartTime = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin(); // Initialize SPI for the RFID reader
  mfrc522.PCD_Init(); // Initialize MFRC522 RFID reader
  lcd.begin(16, 2, 0x27);
  // Servo setup
  // Set up Timer/Counter 0 (TC0) in Fast PWM mode
  TCCR0A = (1 << WGM00) | (1 << WGM01) | (1 << COM0A1);
  TCCR0B = (1 << CS01) | (1 << CS00); // Set prescaler to 64
  DDRD |= (1 << PD6);
  OCR0A = lockPosition; // Move servo to lock position
  // Buzzer Setup
  DDRD |= (1 << PD5);
  PORTD |= (1 << PD5);
  // lcd Setup
  lcd.backlight();
  lcd.clear();
  lcd.print("Welcome!"); // Display initial message on LCD
}

void loop() {
  // Check for RFID card
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Check if the card UID matches the desired UID
    String cardUID = getCardUID();
    cardUID.toUpperCase();
    if (cardUID == "69 F3 FC 53") {
      // RFID card with the desired UID detected
      unlockLock();
    } else {
      // Incorrect card detected
      lcd.clear();
      lcd.print("Invalid Card");
      delay(2000); // Display the message for 2 seconds
      lcd.clear();
      lcd.print("Welcome!");
    }
    mfrc522.PICC_HaltA(); // Stop reading RFID
  }

  // Check for keypad input
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      // Check entered password
      if (enteredPassword == password) {
        unlockLock();
      } else {
        lcd.clear();
        lcd.print("Wrong password");
        delay(2000); // Display the message for 2 seconds
        lcd.clear();
        lcd.print("Welcome!");
      }
      enteredPassword = "";
    } else {
      // Append key to entered password
      enteredPassword += key;
    }
  }

  // Lock the lock after the unlock duration
  if (unlockStartTime != 0 && millis() - unlockStartTime >= unlockDuration) {
    lock();
  }
}

// Function to get the card UID as a string
String getCardUID() {
    String cardUID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      cardUID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      cardUID.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    return cardUID.substring(1);
  }

void unlockLock() {
  lcd.clear();
  lcd.print("Unlocking...");
  PORTD &= ~(1 << PD5); // Buzzer
  OCR0A = unlockPosition; // Servo
  unlockStartTime = millis(); // Start unlock timer
  delay(2000); // Display the message for 2 seconds
  lcd.clear();
  lcd.print("Welcome!");
}

void lock() {
  lcd.clear();
  lcd.print("Locking...");
  PORTD |= (1 << PD5); // Buzzer
  OCR0A = lockPosition; // SErvo
  unlockStartTime = 0; // Reset unlock timer
  delay(2000); // Display the message for 2 seconds
  lcd.clear();
  lcd.print("Welcome!");
}
