/* Arduino Dart ScoreBoard

  Created by Yvan / https://Brainy-Bits.com
  This code is in the public domain...
  You can: copy it, use it, modify it, share it or just plain ignore it!
  Thx!

*/

// Include needed Libraries
#include "Wire.h" // Used to communicate with I2C devices (Pin A4-SDA, A5-SCL)
#include "LiquidCrystal_I2C.h" // I2C connected LCD display
#include "TM1637Display.h" // For 7 segment LED display
#include "Keypad.h" // For 12 buttons keypad

//Define the keypad
int firstnumber = 99; // used to tell how many numbers were entered on keypad
int secondnumber = 99;
int thirdnumber = 99;
const byte ROWS = 4; // Keypad has four rows
const byte COLS = 3; // Keypad has three columns

//Define the keys of the keypad

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {7, 12, 11, 9}; // connect to the Rows pinouts of the keypad to NANO pins
byte colPins[COLS] = {8, 6, 10}; // connect to the Columns pinouts of the keypad to NANO pins
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); // Library init

// Setup the I2C module connected to the LCD
//
//    Board        I2C / TWI pins   (SDA=Datline, SCL=Clock)
//Uno,Ethernet   A4 (SDA), A5 (SCL)
//Mega2560       20 (SDA), 21 (SCL)
//Leonardo,Micro 2 (SDA), 3 (SCL)

#define I2C_ADDR 0x27 // <<- Add your address here. 0x27 is default
#define Rs_pin 0
#define Rw_pin 1
#define En_pin 2
#define BACKLIGHT_PIN 3
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
//LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin); // Library init

//NOTE TO JOE - IS THE ERROR HERE CAUSED BY WRONG LIBRARY? I FOUND THE BELOW LINE I ADDED BUT COMMENTED OUT WORKS OK BUT NOT SURE IF IT FUNCTIONS THE SAME?
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Setup the 4 Digits 7 segment LED
#define P1CLK A0 // Player 1 led display CLK connected to NANO pin A0
#define P1DI0 A1 // Player 1 led display DI0 connected to NANO pin A1
#define P2CLK A2 // Player 2 led display CLK connected to NANO pin A2
#define P2DIO A3 // Player 2 led display DIO connected to NANO pin A3

TM1637Display P1Display(P1CLK, P1DI0); // Library init of Player 1 display
TM1637Display P2Display(P2CLK, P2DIO); // Library init of Player 2 display

// Setup the pins of the Rotary Encoder
#define RotaryCLK 2 // Rotary CLK pin connected to UNO interrupt 0 on NANO pin 2
#define RotaryDT 3 // Rotary DT pin connected to NANO pin 3
#define RotarySwitch 4 // Rotary Switch connected to NANO pin 4

// Players LED indicators
#define Player1Led 13 // 3mm red LED connected to NANO pin 13
#define Player2Led 5 // 3mm red LED connected to NANO pin 5

// Define needed Variables
volatile int GAMESTART = 1; // 1=Display start menu : 0=game started
volatile int GAMESTARTVALUE = 501; // Score start value
volatile int PLAYER1Score = 0; // Stores Player 1 current score
volatile int PLAYER2Score = 0; // Stores Player 2 current score
volatile int PLAYERTurn = 1; // Player 1 is first to start
volatile int GAMERESET = 0; // 0=no : 1=display reset menu
volatile int QUITYES = 2; // Reset Game
volatile int QUITNO = 2; // Don't reset game

// Interrupt routine on Pin 2 (interrupt zero) runs if Rotary Encoder CLK pin changes state
void rotarydetect ()  {
  delay(1); // delay for Debouncing Rotary Encoder

  if (GAMERESET == 1) { // If in Reset game menu, rotary encoder controls the Yes/No dialog
    if (digitalRead(RotaryCLK)) {
      if (digitalRead(RotaryDT)) {
        QUITNO = 1;
        QUITYES = 0;
      }
      if (!digitalRead(RotaryDT)) {
        QUITYES = 1;
        QUITNO = 0;
      }
    }
  }

  if (GAMERESET == 0) { // Not in Game reset menu
    if (digitalRead(RotaryCLK)) {
      if (digitalRead(RotaryDT)) {
        if (GAMESTART == 1) { // Starting new game then rotary encoder
          if (GAMESTARTVALUE > 101) {
            GAMESTARTVALUE = GAMESTARTVALUE - 100;
          }
        }
      }
      if (!digitalRead(RotaryDT)) {
        if (GAMESTART == 1) {
          if (GAMESTARTVALUE < 9901) {
            GAMESTARTVALUE = GAMESTARTVALUE + 100;
          }
        }
      }
    }
  }
}

void setup()
{
  pinMode(Player1Led, OUTPUT);
  pinMode(Player2Led, OUTPUT);
  pinMode(RotarySwitch, INPUT);
  digitalWrite(RotarySwitch, INPUT_PULLUP); // Rotary Switch uses internal pullup resistor
  //  lcd.begin (16, 2); // <<-- our LCD is a 16x2, change for your LCD if needed
  lcd.begin ();

  // LCD Backlight ON
  //  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  //  lcd.setBacklight(HIGH);

  // 7 Segment LED displays brightness
  P1Display.setBrightness(0x0f);
  P2Display.setBrightness(0x0f);

  // Display Starting score on both P1 and P2 7 segment Displays
  P1Display.showNumberDec(8888, 4);
  P2Display.showNumberDec(8888, 4);
 

  // Flash the Intro text on LCD at power up
  lcd.home (); // go home on LCD
  lcd.print("DART SCOREBOARD!");
  lcd.setCursor (0, 1);
  lcd.print("  Version  1.0  ");
  delay(1000);
  lcd.clear();

  // Attach interrupt to rotary encoder
  attachInterrupt (0, rotarydetect, CHANGE); // interupt 0 always connected to pin 2 on Arduino UNO
}

void loop()
{
  char keypressed = keypad.getKey(); // Put value of keypad button if pressed
  if (keypressed != NO_KEY) { // If keypad button pressed check which key it was
    switch (keypressed) {

      case '1':
        checknumber(1);
        break;

      case '2':
        checknumber(2);
        break;

      case '3':
        checknumber(3);
        break;

      case '4':
        checknumber(4);
        break;

      case '5':
        checknumber(5);
        break;

      case '6':
        checknumber(6);
        break;

      case '7':
        checknumber(7);
        break;

      case '8':
        checknumber(8);
        break;

      case '9':
        checknumber(9);
        break;

      case '0':
        checknumber(0);
        break;

      case '*':
        deletenumber();
        break;

      case '#':
        subtractnumber();
        break;
    }
  }

  if (!(digitalRead(RotarySwitch))) { // Do if Rotary Encoder switch is pressed
    delay(250); // debounce switch
    if (GAMESTART == 0) { // Game has started
      if (GAMERESET == 0) { // Reset screen not active
        GAMERESET = 1; // Display reset screen
      }
      if (GAMERESET == 1 && QUITNO == 1) { // Game is in reset screen and NO is selected
        GAMERESET = 0; // Reset GameReset to 0
        QUITNO = 2; // Reset NO
        QUITYES = 2; // Reset YES
      }
      if (GAMERESET == 1 && QUITYES == 1) { // Game is in reset screen and YES is selected
        softReset(); // Reset Game
      }
    }
    if (GAMESTART == 1) { // Game is at select score screen
      GAMESTART = 0; // Start Game
    }
  }

  if (GAMESTART == 1) { // Set various stuff at startup
    digitalWrite(Player1Led, LOW); // Set Player 1 LED to OFF
    digitalWrite(Player2Led, LOW); // Set Player 2 LED to OFF
  
    // Display on LCD starting text
    lcd.home();
    lcd.print("Starting Score >");
    lcd.setCursor (0, 1); // go to start of 2nd line
    lcd.print(GAMESTARTVALUE);
    lcd.print(" ");

    // Display Starting score on both P1 and P2 7 segment Displays
    P1Display.showNumberDec(PLAYER1Score, false, 4, 0);
    P2Display.showNumberDec(PLAYER2Score, false, 4, 0);
    PLAYER1Score = GAMESTARTVALUE;
    PLAYER2Score = GAMESTARTVALUE;

  }

  if (GAMESTART == 0) { // Stuff displayed while in Game
    if (GAMERESET == 0) { // Not in reset game mode
      if (PLAYERTurn == 1) { // Player 1 is up
        digitalWrite(Player1Led, HIGH);
        digitalWrite(Player2Led, LOW);
        lcd.home();
        lcd.print("Player 1:       ");
        lcd.setCursor (0, 1);
        lcd.print(PLAYER1Score);
        lcd.print("  MINUS ");
      } else { // Player 2 is up
        digitalWrite(Player1Led, LOW);
        digitalWrite(Player2Led, HIGH);
        lcd.home();
        lcd.print("Player 2:       ");
        lcd.setCursor (0, 1);
        lcd.print(PLAYER2Score);
        lcd.print("  MINUS ");
      }
    }
  }

  if (GAMERESET == 1) { // Stuff displaed while in Game Reset Mode
    if (QUITNO == 1)  { // Display NO on LCD
      lcd.home();
      lcd.print("   Reset Game?   ");
      lcd.setCursor(0, 1);
      lcd.print("NO!              ");
    } else {
      if (QUITYES == 1)  { // Display YES on LCD
        lcd.home();
        lcd.print("   Reset Game?   ");
        lcd.setCursor(0, 1);
        lcd.print("             YES!");
      } else {
        if (QUITYES == 2)  { // Display both NO and YES on LCD
          lcd.home();
          lcd.print("   Reset Game?   ");
          lcd.setCursor(0, 1);
          lcd.print("NO!          YES!");
        }
      }
    }
  }
}

//
// Various created functions in Main Loop
//

// Reset Arduino to Restart Game
void softReset() {
  asm volatile (" jmp 0"); // Restart code from beginning wityh assembly jump statement
}                       // *note : Does not reset external hardware connected to the Arduino.

void checknumber(int x) {
  if (firstnumber == 99) {
    firstnumber = x;
    lcd.setCursor(13, 1);
    lcd.print(x);
  } else {
    if (secondnumber == 99) {
      secondnumber = x;
      lcd.setCursor(14, 1);
      lcd.print(x);
    } else {
      thirdnumber = x;
      lcd.setCursor(15, 1);
      lcd.print(x);
    }
  }
}


void deletenumber() {
  if (thirdnumber != 99) {
    lcd.setCursor(15, 1);
    lcd.print(" ");
    thirdnumber = 99;
  } else {

    if (secondnumber != 99) {
      lcd.setCursor(14, 1);
      lcd.print(" ");
      secondnumber = 99;
    } else {

      if (firstnumber != 99) {
        lcd.setCursor(13, 1);
        lcd.print(" ");
        firstnumber = 99;
      }
    }
  }
}

void clearNumberDisplay() {
  lcd.setCursor(13, 1);
  lcd.print(" ");
  lcd.setCursor(14, 1);
  lcd.print(" ");
  lcd.setCursor(15, 1);
  lcd.print(" ");
}

void subtractnumber() {
  int keyfullnumber = 0;

  // Score is 1 digit
  if (thirdnumber == 99 && secondnumber == 99 && firstnumber != 99) {
    keyfullnumber = firstnumber;
  }
  // Score is 2 digits
  if (secondnumber != 99 && thirdnumber == 99) {
    keyfullnumber = (firstnumber * 10) + secondnumber;
  }
  // Score is 3 digits
  if (thirdnumber != 99) {
    keyfullnumber = (firstnumber * 100) + (secondnumber * 10) + thirdnumber;
  }
  
  if (PLAYERTurn == 1) {
    if (PLAYER1Score - keyfullnumber >= 0) {
      int oldscore = PLAYER1Score;
      PLAYER1Score = PLAYER1Score - keyfullnumber;
      subtractScoreAnimation(1, oldscore, PLAYER1Score);
    }
    PLAYERTurn = 2;
  } else {
    if (PLAYER2Score - keyfullnumber >= 0) {
      int oldscore = PLAYER2Score;
      PLAYER2Score = PLAYER2Score - keyfullnumber;
      subtractScoreAnimation(2, oldscore, PLAYER2Score);
    }
    PLAYERTurn = 1;
  }
  
  resetnumbers();
  clearNumberDisplay();
}

void subtractScoreAnimation(int player, int oldscore, int newscore) {
  while(oldscore > newscore) {
    oldscore--;
    if (player == 1) {
      P1Display.showNumberDec(oldscore, false, 4, 0);
    } else {
      P2Display.showNumberDec(oldscore, false, 4, 0);  
    }
  }
}

void resetnumbers() {
  firstnumber = 99;
  secondnumber = 99;
  thirdnumber = 99;
}
