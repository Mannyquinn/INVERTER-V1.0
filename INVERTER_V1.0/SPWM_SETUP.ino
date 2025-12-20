const float sineValues[59] =                                          //these are the values for a half cycle
 {   
    0.00, 0.05, 0.10, 0.16, 0.21, 0.26,
    0.31, 0.36, 0.41, 0.45, 0.50, 0.54,
    0.59, 0.63, 0.67, 0.71, 0.74, 0.78,
    0.81, 0.84, 0.87, 0.89, 0.91, 0.93,
    0.95, 0.97, 0.98, 0.99, 0.99, 1.00,
    1.00, 0.99, 0.99, 0.98, 0.97, 0.95, 
    0.93, 0.91, 0.89, 0.87, 0.84, 0.81, 
    0.78, 0.74, 0.71, 0.67, 0.63, 0.59,
    0.54, 0.50, 0.45, 0.41, 0.36, 0.31,
    0.26, 0.21, 0.16, 0.10, 0.05
};

void setupTimers()
{
                                                                 //setup timer1 for base PWM signal, this will give a 32khz pwm signal
  TCCR1A |= 1 << COM1A1 | 1 << COM1B1 | 1 << WGM11;                
  TCCR1A &= ~(1 << COM1A0 | 1 << COM1B0 | 1 << WGM10);  
  TCCR1B |= 1 << WGM12 | 1 << CS10;
  TCCR1B &= ~(1 << WGM13 | 1 << CS12 | 1 << CS11);
  TCNT1 = 0;
  OCR1A = 0;                                                    //This can be set between 0 and 511, but let the maximum be 500
  OCR1B = 0;                                                    //This can be set between 0 and 511, but let the maximum be 500

                                                                 
  TCCR2A &= ~(1 << COM2A1 | 1 << COM2A0  | 1 << WGM20);         //setup timer to trigger changes in duty cycle, this modulates the base PWM signal into SPWM using the sine table
  TCCR2A |= (1 << WGM21);                                       //sets the interrupt to happen almost 6000 per second, divided by the 60 steps in the array gives ~50Hz
  TCCR2B &= ~(1 << WGM22 | 1 << CS22);
  TCCR2B |= 1 << CS21 | 1 << CS20;
  TIMSK2 |= 1 << OCIE2A;
  TCNT2 = 0;
  OCR2A = 82;                                                  //timer 2 
}

ISR(TIMER2_COMPA_vect)                                          //triggers every 190us, used to modulate the base PWM into SPWM            
{
  if(sineArrayIndex < 59 && sineHalf == 1)                      //checks if we're still in the positive half cycle and if we are, step into the next value in the array
  {
      OCR1A = sineValues[sineArrayIndex] * multiplier;          //All this is done so that the positive cycle of the sine wave looks sine-wavy
      sineArrayIndex++;                                         
  }
  else if(sineArrayIndex < 59 && sineHalf == -1)                //checks if we're in the negative cycle and repeat the process
  {
      OCR1B = sineValues[sineArrayIndex]* multiplier;
      sineArrayIndex++;
  } 
  else if(sineArrayIndex == 59 || sineArrayIndex > 59)          //checks for zero crossings and changes the cycle polarity
  {
    sineHalf = -sineHalf;
    sineArrayIndex = 0;
   
    if(sineHalf == 1) OCR1B = 0;                               //makes sure the other mosfet is turned off
    else OCR1A = 0;
    multiplier = PIDControllerOutput;                          //only change the mulltiplier at zero crossings to avoid distorted waveforms
  }
}
