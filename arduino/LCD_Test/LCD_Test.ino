/*
 * LCD Screen Test Code
 * Tests basic LCD functionality including text display, cursor positioning,
 * and screen updates
 * 
 * Common LCD types supported:
 * - 16x2 LCD (I2C or parallel)
 * - 20x4 LCD (I2C or parallel)
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD Configuration
// Adjust the address (0x27 or 0x3F) and size (16x2 or 20x4) based on your LCD
#define LCD_ADDR 0x27    // Common I2C addresses: 0x27 or 0x3F
#define LCD_COLS 16      // Number of columns
#define LCD_ROWS 2       // Number of rows

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

void setup() {
  Serial.begin(9600);
  Serial.println("LCD Test Starting...");
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Test 1: Basic text display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD Test");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  
  Serial.println("LCD initialized successfully!");
}

void loop() {
  // Test 2: Scrolling text
  testScrollingText();
  delay(1000);
  
  // Test 3: Cursor positioning
  testCursorPositions();
  delay(1000);
  
  // Test 4: Character display
  testCharacters();
  delay(1000);
  
  // Test 5: Counter display
  testCounter();
  delay(1000);
  
  // Test 6: Backlight control
  testBacklight();
  delay(1000);
}

void testScrollingText() {
  Serial.println("Test: Scrolling Text");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scrolling Test");
  
  String text = "Hello World! This is a scrolling message. ";
  for (int i = 0; i < text.length(); i++) {
    lcd.setCursor(0, 1);
    lcd.print(text.substring(i, i + LCD_COLS));
    delay(300);
  }
}

void testCursorPositions() {
  Serial.println("Test: Cursor Positions");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cursor Test");
  
  // Display markers at corners and center
  lcd.setCursor(0, 1);
  lcd.print("*");
  delay(500);
  
  lcd.setCursor(LCD_COLS - 1, 1);
  lcd.print("*");
  delay(500);
  
  lcd.setCursor(LCD_COLS / 2, 1);
  lcd.print("*");
  delay(1000);
}

void testCharacters() {
  Serial.println("Test: Characters");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ASCII Test");
  
  // Display some ASCII characters
  lcd.setCursor(0, 1);
  for (int i = 0; i < LCD_COLS && i < 16; i++) {
    lcd.print((char)(65 + i)); // A-P
    delay(100);
  }
  delay(1000);
}

void testCounter() {
  Serial.println("Test: Counter");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Counter Test");
  
  for (int i = 0; i <= 10; i++) {
    lcd.setCursor(0, 1);
    lcd.print("Count: ");
    lcd.print(i);
    lcd.print("  "); // Clear extra digits
    delay(500);
  }
}

void testBacklight() {
  Serial.println("Test: Backlight");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Backlight Test");
  
  for (int i = 0; i < 3; i++) {
    lcd.setCursor(0, 1);
    lcd.print("OFF");
    lcd.noBacklight();
    delay(500);
    
    lcd.backlight();
    lcd.setCursor(0, 1);
    lcd.print("ON ");
    delay(500);
  }
}
