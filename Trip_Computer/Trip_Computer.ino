#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(9, 6, 5, 4, 3, 8);
const int nextButton = 7;
int clicks;

int buttonState;          
int lastButtonState = LOW;

unsigned long lastDebounceTime = 0; 
unsigned long debounceDelay = 50;

int avgSpeed, instFuel, avgFuel, distRemain, speed;

int reverse, lastReverseState = 0;

void setup() {

  pinMode(nextButton, INPUT);
  pinMode(A0, OUTPUT);
  digitalWrite(A0,HIGH);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  clicks = 3;

  //Serial.begin(9600); // For debug use
  //Serial.println("CAN Read - Testing receival of CAN Bus message");  
  //delay(1000);
  
  Canbus.init(CANSPEED_125);  //Initialise MCP2515 CAN controller at the specified speed
    //Serial.println("CAN Init ok");
//  else
    //Serial.println("Can't init CAN");

  lcd.setCursor(0,0);
  lcd.print("    Welcome");
  delay(300);
  for (int positionCounter = 0; positionCounter < 23; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayRight();
    // wait a bit:
    delay(150);
  }

  lcd.clear();
}

void loop() {
  
  int reading = digitalRead(nextButton);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState){
      buttonState = reading;

       if (buttonState == HIGH) {
       clicks = clicks + 1;
       lcd.clear();

       if (clicks == 5) {
          clicks = 0;
        }
      }
    }
  }

    tCAN message;
  if (mcp2515_check_message()) {
    
    if (mcp2515_get_message(&message)) {
     
      if(message.id == 0x400) {
        avgSpeed = (unsigned char)message.data[1];
        instFuel = (unsigned char)message.data[3] / 10;
        avgFuel = (unsigned char)message.data[5] / 10;
        distRemain = (unsigned char)message.data[6] * 256 + (unsigned char)message.data[7];
      }
      
      if (message.id == 0x433) {
        //if 4 == 2 or 3
        
        if (message.data[3] == 0x02 or message.data[3] == 0x03 or message.data[3] == 0x06 or message.data[3] == 0x0A or message.data[3] == 0x0B or message.data[3] == 0x0E or message.data[3] == 0x0F) {
          reverse = 1;  
        } else {
          reverse = 0;
        }
      }
      
      if (message.id == 0x201 ) {
        speed = (unsigned char)message.data[4] * 256 + (unsigned char)message.data[5];
        speed = speed / 100;
      }
    }
  }

  lcd.setCursor(0,0);
  if (clicks == 0) {
    lcd.print("Average Speed:");
    lcd.setCursor(0, 1);
    lcd.print(avgSpeed);
    lcd.print(" Km/h");
  } else if (clicks == 1) {
    lcd.print("Inst. Fuel Econ:");
    lcd.setCursor(0, 1);
    if (instFuel == 25) {
      lcd.print("--");
    } else {
      lcd.print(instFuel);
    }
    lcd.print(" L/100Km");
  } else if (clicks == 2) {
    lcd.print("Avg. Fuel Econ:");
    lcd.setCursor(0, 1);
    lcd.print(avgFuel);
    lcd.print(" L/100Km");
  } else if (clicks == 3) {
    lcd.print("Dist. Remaining:");
    lcd.setCursor(0, 1);
    lcd.print(distRemain);
    lcd.print(" Km ");
  } else {
    lcd.print("Speed:");
    lcd.setCursor(0,1);
    if (speed == -327) {
      lcd.print("0");
    } else {
      lcd.print(speed);
    }
    lcd.print(" Km/h   ");
  }
  
  if (reverse != lastReverseState) {
    if (reverse == 1) {
      digitalWrite(A0, LOW);
    } else {
      digitalWrite(A0, HIGH);
    }
  }
  
  lcd.setCursor(15,1);
  lcd.print(reverse);
  
  lastReverseState = reverse;
  lastButtonState = reading;  
}
