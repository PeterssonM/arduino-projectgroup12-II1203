/*
 * This file is not ment to be included in the final product.
 * Written 2022-04-08.
 */


#include <Arduino.h>


/*
 * This method is called as the interupt flag is set.
 * It sets pin 8 high.
 */
void lightningon()
{
  while(PIND & (0b000000100)) //Only read from pin 2 on port D.
  {
    PORTB = PORTB | (1<<0); //Set output pin 13 to high.
  }
}

/*
 * This method is run upon startup only.
 * Initializes all registers.
 */
void setup()
{
  DDRB = DDRB | (1<<5); //Set pin 13 to output.
  DDRB = DDRB | (1<<0); //Set pin 8 to output.
  PORTB = PORTB | (0<<0); //Set output pin 8 to low.

  DDRB = DDRB | (0<<2); //Set pin 10 to input although input is default.

  //Next from MP to MP: try to make this in baremetall C as well :)
  attachInterrupt(0, lightningon, RISING); //Arduino has two interupts, on pin 2 and pin 3 (see datasheet). 
}

/*
 * This method is executed infinetly. 
 * It constanly checks the status of a button, if high light up a led. 
 */
void loop()
{
  if(PINB & (0b000000100))
  {
    PORTB = PORTB | (1<<0);
  }
  else
  {
    PORTB = PORTB & (0b11111110<<0);
  }
}