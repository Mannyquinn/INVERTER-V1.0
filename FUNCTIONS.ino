void startupCheck()                                                                                                            //only checks if the input voltages are in the right range
{
  inputVoltage = (float)analogRead(INPUT_VOLTAGE_FB_PIN) * INPUT_RESISTOR_DIVIDER * voltageScalingFactor;
  while(inputVoltage <= inputVoltageLowValue)                                                                                  //while loop ensures that this code runs till the voltage becomes higher
  {
    startupError = true;                                                                                                       //this variable can be used for troubleshooting 
    //print lowVoltage to LCD and turn off relay
  }
  while(inputVoltage >= inputVoltageHighValue)                                                                                 //while loop ensures that this code runs till the voltage becomes lower
  {
    startupError = true;
    //print overVoltage to LCD and turn off relay
  }
  startupError = false;
}


void softStart()                                                                                                               //softStart will check for shortCircuit
{
  int dutyCycleMultiplier = 0;
  while(dutyCycleMultiplier < MULTIPLIER_SOFTSTART_VALUE && outputVoltage < (0.5 * SET_OUTPUT_VOLTAGE) )
  {
    PIDControllerOutput = dutyCycleMultiplier++;                                                                            //overides the PID controller
    outputVoltage = (float)analogRead(OUTPUT_VOLTAGE_FB_PIN) * OUTPUT_RESISTOR_DIVIDER * voltageScalingFactor;
    delay(1);
  }
  delay(100);
  
  
 while(outputVoltage < (SET_OUTPUT_VOLTAGE * 0.4))                                                                    //if the output voltage is too low, most likely  over loading, while loop ensures that we don't proceed
  {
      runningError = true;
      overload = true;
      PIDControllerOutput = 0;
    //print overloading on lcd.
  }
  runningError = false;


}
void shutDown()
{
  //turn off all mosfet drivers and high voltage generator
}

uint16_t PID(float error)
{
  static int previousError = 0, controller = 0;
  static float integral = 0;
    if(controller >= SATURATION_MIN && controller <= SATURATION_MAX)                                                            //prevent windup, don't integrate again if controller is almost saturated
      integral += error;
  controller = constrain(Kp*error + Ki*integral + Kd*(error - previousError), 0, 500);                                          //Ki = actual Ki * deltaTime and Kd = actual Kd / deltaTime
  previousError = error;
  return controller;                                                        
}
