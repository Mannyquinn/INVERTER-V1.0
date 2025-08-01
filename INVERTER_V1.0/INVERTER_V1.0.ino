#define INVERTER_OUTPUT_FREQ 50
#define INVERTER_SPWM_FREQUENCY 31    //Khz
#define SPWM_OUTPUT_PIN_1 9 
#define SPWM_OUTPUT_PIN_2 10
#define VOLTAGE_FB_PIN A0


int multiplierSoftStartValue = 250;
int multiplier = 500;
volatile int sineArrayIndex = 0;
volatile int positiveHalf = 1;

void softStart();
void setupTimers();


void setup()
{  
  pinMode(SPWM_OUTPUT_PIN_1, OUTPUT);
  pinMode(SPWM_OUTPUT_PIN_2, OUTPUT);

  setupTimers();
  //softStart();
}

void loop() 
{
  


}
