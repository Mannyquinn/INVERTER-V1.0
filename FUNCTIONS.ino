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
  for(int dutyCycleMultiplier = 0; dutyCycleMultiplier < MULTIPLIER_SOFTSTART_VALUE; dutyCycleMultiplier++)
  {
    PIDControllerOutput = dutyCycleMultiplier;                                                                                 //overides the PID controller
    delay(1);
  }
  delay(100);
  outputVoltage = (float)analogRead(OUTPUT_VOLTAGE_FB_PIN) * OUTPUT_RESISTOR_DIVIDER * voltageScalingFactor;
  
  while(outputVoltage < OUTPUT_VOLTAGE_SOFTSTART_CRITERIA)                                                                    //if the output voltage is too low, most likely  over loading
  {
    runningError = true;
      multiplier = 0;
    //print overloading on lcd.
  }
  runningError = false;


}
void shutDown()
{
  //turn off all mosfet drivers and high voltage generator
}

uint16_t PID(int error)
{
  static int previousError = 0, controller = 0;
  static double deltaTime = 0.001, integral = 0;
  static unsigned long previousTime = 0, currentTime = 0;
 
  currentTime = millis();
  deltaTime = (currentTime - previousTime) / 1000.0;                                                                          //divide by 1000 to get it into seconds
    if(deltaTime > 1)                                                                                                         //avoids integrating over long period
      deltaTime = 0.001;
    if(controller >= SATURATION_MIN && controller <= SATURATION_MAX)                                                            //prevent windup, don't integrate again if controller is almost saturated
      integral += error * deltaTime;
  controller = constrain(Kp*error + Ki*integral + Kd*(error - previousError) / deltaTime, 0, 500);
  previousTime = currentTime;   
  previousError = error;
  return controller;                                                        
}
