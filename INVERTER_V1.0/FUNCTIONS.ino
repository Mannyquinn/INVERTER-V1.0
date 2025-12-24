/*startup checks, only checks if the input rails are in the right range*/
void startupCheck()                                                                                                            
{
  inputVoltage = (float)analogRead(INPUT_VOLTAGE_FB_PIN) * INPUT_RESISTOR_DIVIDER * voltageScalingFactor;
  
 /*make sure that if the voltage is not in the right range, we can't start the pwm or anything, shutdown everything*/
  if(inputVoltage <= inputVoltageLowValue || inputVoltage >= inputVoltageHighValue)                                                                                  
  {
    /* diagnostic variable*/
    startupError = true;
    
    if(inputVoltage >= inputVoltageHighValue){
      highInputVoltage = true;
      lowInputVoltage = false;}
      
    else if(inputVoltage <= inputVoltageLowValue){
      lowInputVoltage = true; 
      highInputVoltage = false;}                                                                                                  
  }
  
  systemErrorCheck();
}

void softStart()                                                                                                             
{
  /*soft start is used when the system first begin, 1. to avoid inrush current, 2. to check for overloading or shorts at startup*/
  
  /*if the duty cycle is high and the output voltage isn't up to half the voltage it should be, there is an error*/
  
  while(outputVoltage < SET_OUTPUT_VOLTAGE)
  {
    /*directly increase the multiplier slowly to perform a softstart*/
    /*this while loop only runs as long as the output voltage hasn't reached the set value*/ 

    /*if the multiplier is not maximum and the voltage hasn't reached the set voltage*/
    if(multiplier < MULTIPLIER_MAX_VALUE)                
      {
        multiplier++;  
        delay(2);
      }
    /*if the multiplier has started increasing but the voltage hasn't picked up, there might  be a short */
      else if(multiplier > 250 && outputVoltage < 0.1*SET_OUTPUT_VOLTAGE)
      {
        startupError = true;
        shortCircuit = true;
        break;
      }

    /*if the multiplier is maximum and the output voltage hasn't reached the set voltage, most likely overloading or output short */
    else
     {
       multiplier = 0;
       startupError = true;
       overload = true;
       break;
     }

    /*read the output voltage after a change in multiplier*/ 
     outputVoltage = (float)analogRead(OUTPUT_VOLTAGE_FB_PIN) * OUTPUT_RESISTOR_DIVIDER * (float)voltageScalingFactor;
  }

  /*check if there were any errors in the startup*/
  systemErrorCheck();
  
}

/* this system check if there is any issue while running, it will setup the flags which will be picked up by the system error check function*/
void readSystemParameters()
{
  /*read the input voltages*/
  inputVoltage = (float)analogRead(INPUT_VOLTAGE_FB_PIN) * INPUT_RESISTOR_DIVIDER * (float)voltageScalingFactor;

  /*read the output voltages*/
  outputVoltage = (float)analogRead(OUTPUT_VOLTAGE_FB_PIN) * OUTPUT_RESISTOR_DIVIDER * (float)voltageScalingFactor;

  /*check for issues with the input voltages and set flags*/
      if(inputVoltage >= inputVoltageHighValue)
      {
        runningError = true;
        highInputVoltage = true;
        lowInputVoltage = false;
      }
      else if(inputVoltage <= inputVoltageLowValue)
      {
        runningError = true;
        lowInputVoltage = true; 
        highInputVoltage = false;
      }
      else
      {
        lowInputVoltage = false;
        highInputVoltage = false;
      }
      if(multiplier >= MULTIPLIER_MAX_VALUE)
      {
        if(outputVoltage <= OVERLOAD_VOLTAGE)
        runningError = true;
        overload = true;
      }

  /*check for issues with shorted outputs and set the flags*/    
      if(digitalRead(SHORT_CIRCUIT_PIN) == LOW)
      {
        runningError = true;
        shortCircuit = true;
      }
      else
      {
        shortCircuit = false;
      }

      if(shortCircuit == false && highInputVoltage == false && lowInputVoltage == false)
      {
        runningError = false;
      }
}

void systemErrorCheck()
{
  /*check if there is any system error*/
  if(startupError == true || runningError == true)
  {
    shutDown();
  }
}



void shutDown()
{
  /* 
   *  disable spwm
      turn off mosfet driver and sg3525
      shut down everything.
      you must restart system to begin normal operation
   *  
   */

  DDRB |= 1 << SHUT_DOWN_PIN;                               //shuts down external hardware
  SREG &= ~(1 << 7);                                        //disable interrupts
  TCCR1A = 0;                                               //disconnect the SPWM pins
  TCCR1B = 0;
  PORTB &= ~(1 << SPWM_OUTPUT_PIN_1);                       //pull both driver input low, incase the hardware shutdown fails 
  PORTB &= ~(1 << SPWM_OUTPUT_PIN_2);
}

void PID(int& error)
{
    static int previousError = 0;
    static int controller = 0;
    static float integral = 0;
    static float P_controller = 0;
    static float I_controller = 0;
    static float D_controller = 0;
    
   /*prevent windup, don't integrate again if controller is almost saturated*/
    if(controller >= SATURATION_MIN && controller <= SATURATION_MAX)                                                            
      integral += error; 

   /*Ki = actual Ki * deltaTime and Kd = actual Kd / deltaTime*/
    P_controller += error * Kp;                               //velocity based proportional controller
    P_controller = constrain(P_controller, 0, 500);        //p_controller must not be allowed to go above 500
    I_controller = Ki * integral;                             //integral
    D_controller = Kd*(error - previousError);                //derivative
    controller = constrain(P_controller + I_controller + D_controller , 0, 500);        

  /* write the controller output to the SPWM*/
  /* you may wonder why i am writing the controller output to the PIDControllerOutput variable instead of the multiplier directly
   *  it is because writing directly to the multiplier variable directly will update it immediately which may cause jitters in the sine wave
   *  the PIDControllerOutput variable updates the SPWM every zero crossing point
   *  you may also wonder why i compute the PID into the controller variable instead of the PIDControllerOutput variable directly
   *  it is to avoid race conditions
   */
    PIDControllerOutput = controller;                                     
    previousError = error;
                                                         
}

