#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

const byte rxPin = 10;
const byte txPin = 11;

SoftwareSerial HC12(rxPin, txPin); // recieve at pin 10, outputs at pin 11

LiquidCrystal_I2C lcd(0x27, 20, 4);

String currentMessage = ""; // the message that will actually be sent when user clicks on send
String visibleMessage = ""; // the message that the user sees in the lcd

// setting up keypad
const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

const byte rowPins[ROWS] = {9, 8, 7, 6}; 
const byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// letter layout thing
const char* layout[] = {" 0", ".,1", "abc2", "def3", "ghi4", "jkl5", "mno6", "pqrs7", "tuv8", "wxyz9"};

void setup() {
  Wire.begin();
  HC12.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.print("Walkie Talkie Ready...");

  delay(500);
  lcd.clear();
}

int lastKey = -1;
byte sameKeyTaps = 0;

void loop() {
  if (HC12.available()) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(HC12.readString());
  }

  char key = customKeypad.getKey();
  // *= send
  // # = backspace
  // c = convert to letter
  if (key >= '0' && key <= '9') {
    int intKey = key - '0';
    if (intKey == lastKey || lastKey == -1){ //same key or first key you pressed
      sameKeyTaps++;
      visibleMessage += key;
    }
    else { // not same key, switched keys
      sameKeyTaps = 1;
      visibleMessage = (currentMessage + key); // reset all the visible numbers
    }
    lastKey = intKey;
    refreshLCD();
  }
  // if not in the 0-9 keys
  else {
    switch (key){
      case 'A': 
        break;
      case 'B': // backspace for the number key taps
        if (visibleMessage.length() > 0){
          visibleMessage.remove(visibleMessage.length() - 1);
        }
        refreshLCD();
        break;
      case 'C': // convert currentNumbers to char
        int index = (sameKeyTaps - 1) % strlen(layout[lastKey]); // so if you press it 5 times, it goes back to the first char
        currentMessage += layout[lastKey][index];
        visibleMessage = currentMessage;
        refreshLCD();
        
        // resetting the variables
        sameKeyTaps = 0; 
        lastKey = -1;
        break;
      case 'D':
        break;
      case '*': //send
        HC12.print(currentMessage);
        currentMessage = "";
        visibleMessage = "";
        lcd.clear();
        lcd.print("Message sent!");
        break;
      case '#': // backspace
        if (currentMessage.length() > 0){
          currentMessage.remove(currentMessage.length() - 1);
        }
        visibleMessage = currentMessage;
        refreshLCD();
        break;
    }
  }

}

void refreshLCD(){
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print(visibleMessage);
}