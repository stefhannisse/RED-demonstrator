#include <FastLED.h>

#define NUM_LEDS 27

CRGB leds[NUM_LEDS];

const int color = 200;
const int ledIndex = 24;
unsigned long previousMillis = 0;
int index = 0;

//IO pins
const int ledPin = 5;
const int led_status = 12;
const int led_continuous = 13;

//10 naar 13
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(led_status, INPUT_PULLUP);
  pinMode(led_continuous, INPUT_PULLUP);
  
  FastLED.addLeds<WS2812B, ledPin, RGB>(leds, NUM_LEDS);
}

void loop() { 
  if(digitalRead(led_continuous)) {
    Serial.println("Automatic mode");
    for(int i=0; i<NUM_LEDS; i++) {
      leds[i] = CHSV(color, color, color);
      leds[i+1] = CHSV(color, color, color); 
      leds[i+2] = CHSV(color, color, color); 
      FastLED.show();

      delay(500);
      leds[i] = CHSV(0, 0, 0);
      leds[i+1] = CHSV(0, 0, 0); 
      leds[i+2] = CHSV(0, 0, 0); 
      FastLED.show();
    }
  }
  if(digitalRead(led_status)) {
      //Wait untill the pin is not high anymore
      Serial.println("Pin is high");
      while(digitalRead(led_status)) {
        Serial.println("Waiting for the pin to become low again");
        return;
      }

      Serial.println("Pin turned low again");

      bool loopActive = true;
  
      while(loopActive) {
        unsigned long currentMillis = millis();
  
        if(currentMillis - previousMillis >= 500) {
          Serial.println("Writing pin");
          Serial.println(index);
          previousMillis = currentMillis;
  
          //Turn off the previous lights
          leds[index-1] = CHSV(0, 0, 0); 
          leds[index] = CHSV(0, 0, 0); 
          leds[index+1] = CHSV(0, 0, 0); 
  
          //Turn on the new lights        
          leds[index] = CHSV(color, color, color);
          leds[index+1] = CHSV(color, color, color); 
          leds[index+2] = CHSV(color, color, color); 

          FastLED.show();
  
          if(index + 2 >= NUM_LEDS) {
            leds[25] = CHSV(0, 0, 0); 
            leds[26] = CHSV(0, 0, 0); 
            leds[27] = CHSV(0, 0, 0); 
            FastLED.show();
            
            index = 0;
            loopActive = false;
          } else {
            index++;
          }
        }
        if(digitalRead(led_status)) {
          loopActive = false;
          Serial.println("Aborting loop because pin turned high again");
        }
      }
    }
}

//void checkPin() {
//  bool active = true;
//  unsigned long previousMillis = 0;
//  int index = 0;
//  
//  while(active) {
//    if(digitalRead(led_status)) {
//      //Wait untill the pin is not high anymore
//      Serial.println("Pin is high");
//      while(digitalRead(led_status)) {
//        Serial.println("Waiting for the pin to become low again");
//        return;
//      }
//
//      Serial.println("Pin turned low again");
//
//      bool loopActive = true;
//  
//      while(loopActive) {
//        unsigned long currentMillis = millis();
//  
//        if(currentMillis - previousMillis >= 500) {
//          Serial.println("Writing pin");
//          Serial.println(index);
//          previousMillis = currentMillis;
//  
//          //Turn off the previous lights
//          leds[index-1] = CHSV(0, 0, 0); 
//          leds[index] = CHSV(0, 0, 0); 
//          leds[index+1] = CHSV(0, 0, 0); 
//  
//          //Turn on the new lights        
//          leds[index] = CHSV(color, color, color); //0 - 1
//          leds[index+1] = CHSV(color, color, color); //1 - 2
//          leds[index+2] = CHSV(color, color, color); //2 - 3
//
//          FastLED.show();
//  
//          if(index + 2 >= NUM_LEDS) {
//            leds[index] = CHSV(0, 0, 0); 
//            leds[index+1] = CHSV(0, 0, 0); 
//            leds[index+2] = CHSV(0, 0, 0); 
//            FastLED.show();
//            
//            index = 0;
//            loopActive = false;
//          } else {
//            index++;
//          }
//        }
//        if(digitalRead(led_status)) {
//          loopActive = false;
//          Serial.println("Aborting loop because pin turned high again");
//        }
//      }
//    }
//  }
//}
