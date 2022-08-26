/***************************************************************************
  This is a library for the AMG88xx GridEYE 8x8 IR camera

  This sketch tries to read the pixels from the sensor

  Designed specifically to work with the Adafruit AMG88 breakout
  ----> http://www.adafruit.com/products/3538

  These sensors use I2C to communicate. The device's I2C address is 0x69

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Dean Miller for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <IRremote.hpp>
#include "PinDefinitionsAndMore.h"

#define CDT 60

#define SADDRESS 0xBBAA
#define SREPEATS 0
const uint8_t COMMANDS_ARR[] = {
  0x34,
  /* fan spin command goes here*/
};

#define BODY_TEMP_MODIFIER  3.0f

Adafruit_AMG88xx amg;

float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
int timer = 1;
bool ir_on = false;

void setup()
{
  bool status;

  Serial.begin(9600);
  Serial.println(F("Celsius - A home-based smart thermal imaging system for temperature regulation"));

  IrSender.begin();
  pinMode(APPLICATION_PIN, INPUT_PULLUP);

  status = amg.begin();
  if(!status) {
      Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
      while(1);
  }

  Serial.println("Power On");
  Serial.println();

  delay(100);
}


void loop()
{
  float rtem;
  float btem;
  float max;

  Serial.println();

  amg.readPixels(pixels);

  rtem = max = pixels[0];
  for(int i = 1; i < AMG88xx_PIXEL_ARRAY_SIZE; i++){
    rtem += pixels[i];
    if(pixels[i] > max){
      max = pixels[i];
    }
  }
  rtem /= 64;
  btem = rtem + BODY_TEMP_MODIFIER;
  Serial.print("Average temperature: ");
  Serial.println(rtem);
  Serial.print("Highest Temperature: ");
  Serial.println(max);

  if((!ir_on && (max > btem && btem > 34 || max > 38)) || (ir_on && timer > CDT && max < 36 )) {
    /* commands have type uint8_t, so sizeof(COMMANDS_ARR) is number of commands */
    for(int i = 0; i < sizeof(COMMANDS_ARR); i++) {
      Serial.println();
      Serial.print(F("Send now: address=0x"));
      Serial.print(SADDRESS, HEX);
      Serial.print(F(" command=0x"));
      Serial.print(COMMANDS_ARR[i], HEX);
      Serial.print(F(" repeats="));
      Serial.print(SREPEATS);
      Serial.println();

      Serial.println(F("Send NEC with 16 bit address"));
      Serial.flush();
      delay(1000);

      IrSender.sendNEC(SADDRESS, COMMANDS_ARR[i], SREPEATS);
      Serial.println(" < Command Sent > ");
    }
    ir_on = !ir_on;
    timer = 1;
  } else {
    if(ir_on) {
      timer += 1;
    }
    Serial.println(" < No Action > ");
  }

  Serial.print("timer count: ");
  Serial.println(timer);
  Serial.print("IR status: ");
  Serial.println(ir_on);

  delay(1000);
}
