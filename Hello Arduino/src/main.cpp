#include <Arduino.h>
/*
 * This method is called as the interupt flag is set.
 * It sets pin 8 high
 */

ISR(INT0_vect) {
  PORTB ^=(1 << 0); //flip ouput pin 8 with bitwise xOR
  delay(200000);                                              //!!temp!! other components does stuff
  PORTB ^=(1 << 0); //flip ouput pin 8 back
}

/*
 * This method is run upon startup only.
 * Initializes all registers.
 */
void setup()
{
  DDRB = DDRB | (1<<0); //Set pin 8 to output.
  PORTB = PORTB | (0<<0); //Set output pin 8 to low.
 
  //set INT0 to RISING edge
  EICRA |= (1 << ISC01);
  EICRA |= (1 << ISC00);
  //enables interupts for INT0
  EIMSK |= (1 << INT0);

  //enables global interupts
  sei();
}

/*
 * This method is executed infinetly.  
 */
void loop()
{

}