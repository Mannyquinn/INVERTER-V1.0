void softStart()
{
  for(int dutyCycleMultiplier = 0; dutyCycleMultiplier < multiplierSoftStartValue; dutyCycleMultiplier++)
  {
    multiplier = dutyCycleMultiplier;
    delay(1);
  }
  multiplier = 500;
}
