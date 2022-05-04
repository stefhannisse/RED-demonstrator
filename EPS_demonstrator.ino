#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#define NUM_OF_LEDS 27 

Servo phi_servo;
Servo theta_servo;
Servo arch_left;
Servo arch_right;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

const int theta_angles[NUM_OF_LEDS] = {160, 155, 150, 145, 140, 135, 128, 123, 117, 112, 106, 100, 90, 83, 80, 75, 69, 64, 58, 52, 46, 41, 34, 29, 20, 20, 20};
const int phi_angles[NUM_OF_LEDS] = {90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90};

const char *performanceText[] = {"Not good", "Average", "Good", "Great"};
const char *seasons[] = {"Spring","Summer", "Fall", "Winter"};

//Calibration values
float calibration_values[NUM_OF_LEDS] = {0*NUM_OF_LEDS};
float stationary_value = 0;
float average = 0;
float maxVoltage = 0;

const int led_continuous = 10;
const int led_strip_status = 12;
const int joystick_x = A3;
const int joystick_y = A2;
const int solarPanel = A0;
const int pushButton = 13;

int previous_phi = 0;
int theta = 0;
bool lcdRefresh = false;
bool joystickFullyExtendedY = false;
int selectedSeason = 0;
int selectedAge = 0;
int joystickMenuPosition = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(led_strip_status, OUTPUT);
  pinMode(led_continuous, OUTPUT);
  pinMode(pushButton, INPUT_PULLUP);

//  digitalWrite(led_strip_status, HIGH);
  
  theta_servo.attach(3);
  phi_servo.attach(4);
  arch_left.attach(5);
  arch_right.attach(6);

  lcd.init();      
  lcd.backlight();            
}

void loop() {
  if(calibration_values[0] == 0 && stationary_value == 0) {
    Serial.println("Calibration is needed...");
    calibration();
  }
  char *someItems[]={"Automatic mode","Manual mode", "Settings", '\0'};
  int menuResult = showMenu(someItems, 3);

  Serial.println(menuResult);

  switch(menuResult) {
    case 0:
      //Automatic mode
      Serial.println("Going into automatic mode");
      automaticMode();
      break;
    case 1:
      //Manual mode
      Serial.println("Going into manual mode");
      manualMode();
      break;
    case 2:
      Serial.println("Goin ginto settings");
      handleSettings();
      break;
  }
//  manualMode();
//  automaticMode();
}

void calibration() {
  bool calibrationActive = true;

  float leftValue = 0;
  float rightValue = 0;

  arch_left.write(90);
  arch_right.write(90);

  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Calibrating...");
  
  //First get readings to the side
  theta_servo.write(180);
  phi_servo.write(0);
  delay(1000);
  leftValue = analogRead(solarPanel) * (5.0 / 1023.0);
  
  delay(500);
  //Get the value from the other side
  theta_servo.write(0);
  delay(1000);
  
  rightValue = analogRead(solarPanel) * (5.0 / 1023.0);
  stationary_value = (leftValue + rightValue) / 2;
  delay(50);

  //Turn on the led strip
  digitalWrite(led_strip_status, HIGH);
  delay(50);
  digitalWrite(led_strip_status, LOW);
  
  for(int i=0; i<NUM_OF_LEDS; i++) {
    theta_servo.write(theta_angles[i]);
    phi_servo.write(phi_angles[i]);

    const float voltage = analogRead(solarPanel) * (5.0 / 1023.0);
    
    Serial.print(i);
    Serial.print(" ");
    Serial.println(voltage);
    calibration_values[i] = voltage;

    if(maxVoltage < voltage) {
      maxVoltage = voltage;
    }

    if(i <= 24) {
      average = average + voltage;
    }
    delay(500);
  }

  average = average / 25.0;

  Serial.print("Average: ");
  Serial.println(average);

  if(maxVoltage < rightValue || maxVoltage < leftValue) {
    lcd.setCursor(0,1);
    lcd.print("Warning:");
    lcd.setCursor(0,2);
    lcd.print("Ambient light is");
    lcd.setCursor(0,3);
    lcd.print("to bright");
    delay(5000);
  }

  Serial.println("Calibration completed.");
  Serial.print("Left voltage: ");
  Serial.println(leftValue);
  Serial.print("Right voltage: ");
  Serial.println(rightValue);
  Serial.print("Average value: ");
  Serial.println(average);
  Serial.print("Max value: ");
  Serial.println(maxVoltage);
  Serial.print("Interval start: ");
  Serial.println(stationary_value);

  selectedSeason = 1;
  seasonControl(1);

  phi_servo.write(90);
  theta_servo.write(90);
}

void handleSettings() {
  clearDisplay();
  char *mainMenu[]={"Change season","Maintence mode", "Age group", "Recalibrate", '\0'};
  int menuResult = showMenu(mainMenu, 4);

  if(menuResult == 0) {
    Serial.println("Choose season");
    char *seasonMenu[]={"Spring","Summer", "Fall", "Winter", '\0'};
    menuResult = showMenu(seasonMenu, 4);
    Serial.print("Selected season");
    Serial.println(menuResult);
    selectedSeason = menuResult;
    seasonControl(menuResult);
    clearDisplay();
    lcd.setCursor(0,0);
    lcd.print("Season selected ");
    lcd.setCursor(0,1);
    lcd.print("                ");
  } else if(menuResult == 1) {
    Serial.println("Maintenance mode");
    maintenanceMode();
  } else if(menuResult == 2) {
    Serial.println("Select age");
    char *ageMenu[]={"8-14","14-17", '\0'};
    menuResult = showMenu(ageMenu, 2);
    Serial.print("Age selected");
    Serial.println(menuResult);
    selectedAge = menuResult;
    clearDisplay();
    lcd.setCursor(0,0);
    lcd.print("Age selected    ");
    lcd.setCursor(0,1);
    lcd.print("                ");
  } else if(menuResult == 3) {
    Serial.println("Calibrating");
    calibration();
  }
  delay(1000);
}

void clearDisplay() {
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,2);
  lcd.print("                ");
  lcd.setCursor(0,3);
  lcd.print("                ");
}

void maintenanceMode() {
  clearDisplay();
  phi_servo.detach();
  theta_servo.detach();
  arch_left.detach();
  arch_right.detach();
  lcd.setCursor(0, 0);
  lcd.print("Maintenance mode");
  lcd.setCursor(0, 1);
  lcd.print("Servos are detached");
  lcd.setCursor(0, 2);
  lcd.print("Press button");
  lcd.setCursor(0, 3);
  lcd.print("to continue...");

  while(digitalRead(pushButton) == HIGH) {
    
  }
  lcd.setCursor(0, 0);
  lcd.print("Restart necessary");
  lcd.setCursor(0, 1);
  lcd.print("To power the servos");
  delay(5000);
}

void manualMode() {
  digitalWrite(led_continuous, HIGH);
  unsigned long previousMillis = 0;
  unsigned long previousMillisJoystick = 0;
  double minVoltage = analogRead(solarPanel) * (5.0 / 1023.0);
  double maxVoltage = analogRead(solarPanel) * (5.0 / 1023.0);
  int xPosition = 0;
  int yPosition = 0;

  clearDisplay();
  lcd.setCursor(0, 0);
  lcd.print("Manual mode");

  lcd.setCursor(0, 2);
  lcd.print("Voltage:");
  lcd.setCursor(14, 2);
  lcd.print("V");
  lcd.setCursor(0, 3);
  lcd.print("Min:");
  lcd.setCursor(8, 3);
  lcd.print("V");
  lcd.setCursor(10, 3);
  lcd.print("Max:");
  lcd.setCursor(18, 3);
  lcd.print("V");
  lcd.setCursor(0, 1);
  lcd.print("Efficiency:");
  
  while(digitalRead(pushButton) == HIGH) {
    int total_values_x = 0;           // Variable that holds the total joystick X value, to substract by 3 later on.
    int total_values_y = 0;           // Variable that holds the total joystick Y value, to substract by 3 later on.

    if(millis() - previousMillisJoystick > 75) {
      previousMillisJoystick = millis();
      xPosition = analogRead(joystick_x);
      yPosition = analogRead(joystick_y);
//    int xPosition = analogRead(joystick_x);
//    int yPosition = analogRead(joystick_y);
  
    /*
      The next part checks if the joystick is moved enough for actions to happen. When the joystick is in rest the last position of the rebotic arm is kept.
      If the joystick has been moved enough, the angle is calculated for the bottom servo.
      The servo on top should also move to the correct position, this does not work yet as it should.
    */
  
      //Check if the joystick is moved substantially.
      if(xPosition > 700 || yPosition > 700 || xPosition < 324 || yPosition < 324) {
        //If that is the case, calculate the angle with the two joystick values.
        int Phi = calculatePhi(xPosition, yPosition);
        //Write the angle to the servo.
        phi_servo.write(Phi);
    
        if(abs(Phi - previous_phi < 10)) {
          if(xPosition < 630) {
            decrementTheta();
          } else {
            incrementTheta();
          }
        } else {
          previous_phi = Phi;
        }
    
        theta_servo.write(theta);
      }
    }

    //Check what voltage is achieved and calculate the efficiency
    float generatedVoltage = analogRead(solarPanel) * (5.0 / 1023.0);

    if(generatedVoltage > maxVoltage) {
      maxVoltage = generatedVoltage;
    } else if(generatedVoltage < minVoltage) {
      minVoltage = generatedVoltage;
    }
    
    generatedVoltage = generatedVoltage - stationary_value;

    float range = maxVoltage - stationary_value;

    int efficiency = (generatedVoltage / range) * 100;

    if(efficiency < 0) {
      efficiency = 0;
    } else if(efficiency > 100) {
      efficiency = 100;
    }

    if(millis() - previousMillis > 500) {
      previousMillis = millis();

      //Show the voltage readings
//      lcd.setCursor(0, 2);
//      lcd.print("Voltage:");
      lcd.setCursor(9, 2);
      lcd.print(analogRead(solarPanel) * (5.0 / 1023.0), 3);
//      lcd.setCursor(14, 2);
//      lcd.print("V");
      
//      lcd.setCursor(0, 3);
//      lcd.print("Min:");
      lcd.setCursor(4, 3);
      lcd.print(minVoltage);
//      lcd.setCursor(8, 3);
//      lcd.print("V");
  
//      lcd.setCursor(10, 3);
//      lcd.print("Max:");
      lcd.setCursor(14, 3);
      lcd.print(maxVoltage);
//      lcd.setCursor(18, 3);
//      lcd.print("V");
      
      if(selectedAge == 1) {
        //Tell the efficiency in words
        Serial.print("Efficiency: ");
        Serial.println(efficiency);

        lcd.setCursor(14, 1);
        lcd.print("   ");
//        lcd.setCursor(0, 1);
//        lcd.print("Efficiency:");
        lcd.setCursor(14, 1);
        lcd.print(efficiency);
      } else {
//        lcd.setCursor(0, 1);
//        lcd.print("Efficiency:");
        lcd.setCursor(12, 1);
        lcd.print("           ");

        if(efficiency >= 0 && efficiency < 25) {
          lcd.setCursor(12, 1);
          lcd.print(performanceText[0]);
        } else if(efficiency >= 25 && efficiency < 50) {
          lcd.setCursor(12, 1);
          lcd.print(performanceText[1]);
        } else if(efficiency >= 50 && efficiency < 75) {
          lcd.setCursor(12, 1);
          lcd.print(performanceText[2]);
        } else if(efficiency >= 75 && efficiency <= 100) {
          lcd.setCursor(12, 1);
          lcd.print(performanceText[3]);
        }
      }
    }
  }
  Serial.println("Leaving manual mode");
  theta_servo.write(90);
  phi_servo.write(90);
  digitalWrite(led_continuous, LOW);
  delay(500);
}

void incrementTheta() {
  if(theta < 160) {
    theta = theta + 10;
  } else {
    theta = 160;
  }
}

void decrementTheta() {
  if(theta > 20) {
    theta = theta - 10;
  } else {
    theta = 20;
  }
}

int calculatePhi(int x, int y) {
  x = x - 636;
  y = y - 628;
  
  const float DEG2RAD = PI / 180.0f;
  const float RAD2DEG = 180.0f / PI;

  float phi = atan2(x, y) * RAD2DEG;

  if(phi < 0) {
    phi += 180;
  }

  Serial.print("Phi: ");
  Serial.println(phi);
  return abs(180 - phi);
}

void automaticMode() {
  unsigned long previousMillis = 0;
  bool active = true;
  int index = 0;
  float maxVoltage = 0.0;
  float minVoltage = 0.0;

  clearDisplay();
  lcd.setCursor(0, 0);
  lcd.print("Automatic mode");

  lcd.setCursor(0, 3);
  lcd.print(seasons[selectedSeason]);

  while(active) {
    unsigned long currentMillis = millis();
    const float voltage = analogRead(solarPanel) * (5.0 / 1023.0);
    
    if(index == 0) {
      digitalWrite(led_strip_status, HIGH);
      delay(50);
      digitalWrite(led_strip_status, LOW);
      maxVoltage = voltage;
      minVoltage = voltage;
    }
    
    if (currentMillis - previousMillis >= 500) {
      previousMillis = currentMillis;

      if(voltage > maxVoltage) {
        maxVoltage = voltage;
      } else if(voltage < minVoltage) {
        minVoltage = voltage;
      }

      Serial.print("Voltage: ");
      Serial.println(voltage);
      Serial.println(index);

      lcd.setCursor(0, 1);
      lcd.print("Voltage:");
      lcd.setCursor(9, 1);
      lcd.print(voltage, 3);
      lcd.setCursor(14, 1);
      lcd.print("V");
      
      lcd.setCursor(0, 2);
      lcd.print("Min:");
      lcd.setCursor(4, 2);
      lcd.print(minVoltage);
      lcd.setCursor(8, 2);
      lcd.print("V");

      lcd.setCursor(10, 2);
      lcd.print("Max:");
      lcd.setCursor(14, 2);
      lcd.print(maxVoltage);
      lcd.setCursor(18, 2);
      lcd.print("V");

      phi_servo.write(phi_angles[index]);
      theta_servo.write(theta_angles[index]);

      if(index + 1 >= NUM_OF_LEDS) {
        index = 0;
      } else {
        index++;
      }
    }

    if(digitalRead(pushButton) == LOW) {
      active = false;
      Serial.println("Leaving automatic mode");
      phi_servo.write(90);
      theta_servo.write(90);
      delay(500);
    }
  }
}

void seasonControl(int season) {
  switch(season) {
    case 0:
      arch_left.write(110);
      arch_right.write(70);
      break;
    case 1:
      arch_left.write(100);
      arch_right.write(80);
      break;
    case 2:
      arch_left.write(80);
      arch_right.write(100);
      break;
    case 3:
      arch_left.write(70);
      arch_right.write(110);
      break;
  }
}

int showMenu(char *items[], int length) {
  bool menuActive = true;
  bool joystickMaxPosition = false;
  int menuIndex = 0;
  int selectionIndex = 0;

  lcd.clear();

  while(menuActive) {
    //Loop through the main menu
    if(lcdRefresh) {
      lcd.clear();
      lcdRefresh = false;
    }

    if(joystickMaxPosition) {
      for(int i=0; i<4;i++) {
        lcd.setCursor(0,i);
        lcd.print("  ");
      }
    }

    lcd.setCursor(0, calculateRest(selectionIndex, 4));
    lcd.print("*");

    for(int i=0; i<length; i++) {
      lcd.setCursor(2,i);
      if(selectionIndex < 4) {
        lcd.print(items[i]);
      }
//      if(selectionIndex % 2 == 0) {
//        lcd.print(*(items+i+selectionIndex));
//      } else {
//        lcd.print(*(items+i+(selectionIndex-1)));
//      }
    }

    //Check joystick values
    int verticalPositionJoystick = analogRead(joystick_y);
    if(verticalPositionJoystick < 400 && joystickMaxPosition == false) {
      Serial.println("Moved uo");
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("                ");
      joystickMaxPosition = true;
      Serial.print("Nieuwe index: ");
      Serial.println(checkDecrement(selectionIndex, 0, length));
      selectionIndex = checkDecrement(selectionIndex, 0, length);
    } else if(verticalPositionJoystick > 800 && joystickMaxPosition == false) {
      Serial.println("Moved down");
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("                ");
      joystickMaxPosition = true;
      Serial.print("Nieuwe index: ");
      Serial.println(checkIncrement(selectionIndex, length));
      selectionIndex = checkIncrement(selectionIndex, length);
    } else if(verticalPositionJoystick <= 800 && verticalPositionJoystick >= 400) {
      joystickMaxPosition = false;
    }

    if(digitalRead(pushButton) == LOW) {
      delay(800);
      menuActive = false;
      menuIndex = selectionIndex;
    }
  }
  return menuIndex;
}

int calculateRest(int total, int part) {
  return total - (int(total / part) * part);
}

int checkDecrement(int value, int min, int max) {
  if(value - 1 < 0) {
    return max - 1;
  } else {
    return value - 1;
  }
}

int checkIncrement(int value, int max) {
  if(value + 1 >= max) {
    return 0;
  } else {
    return value + 1;
  }
}
