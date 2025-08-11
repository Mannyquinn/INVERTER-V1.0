#define SPWM_OUTPUT_PIN_1 PB1 
#define SPWM_OUTPUT_PIN_2 PB2
#define OUTPUT_VOLTAGE_FB_PIN A0                                                                //output voltage feedback pin
#define INPUT_VOLTAGE_FB_PIN  A1
#define SATURATION_MAX 450                                                                      //used to avoid integration windup
#define SATURATION_MIN 10
#define SHORT_CIRCUIT_PIN PB0                                                                   //resistor dividers used on output
#define MULTIPLIER_SOFTSTART_VALUE 250
#define VOLTAGE_REFERENCE 5.0
                                               

/* output parameter settings */
#define SET_OUTPUT_VOLTAGE  10.0                                                                  //10V peak not RMS
#define INPUT_RESISTOR_DIVIDER 3                                                                  //the resistor divider used in the DC input
#define OUTPUT_RESISTOR_DIVIDER 2.0                                                              //resistor divider at the AC output
#define OVERLOAD_VOLTAGE 2

/* PID parameters */
float error = 0;
#define K_proportional 20
#define K_integral 100
#define K_derivative 0
#define deltaTime 0.004                                                                       //250hz

const float Kp = K_proportional;
float Ki = K_integral * deltaTime;
float Kd = K_derivative / deltaTime;



/*other variables*/
uint16_t PIDControllerOutput;
volatile int multiplier;
volatile uint8_t sineArrayIndex = 0;
volatile int sineHalf = 1;
const float voltageScalingFactor =  float(VOLTAGE_REFERENCE / 1023.0);


 /* function declarations*/
void softStart(),
     setupTimers(),
     startupCheck();
uint16_t PID(float);

/*system parameters*/
float inputVoltage,
      inputVoltageLowValue = 11.0,
      inputVoltageHighValue = 14.0,
      outputVoltage = 0;

        
/*system check errors*/
bool startupError = false,                //can be caused by undevoltage or overvoltage
     runningError = false,                //can be caused by undervoltage, overvoltage, shortcircuit or overloading
     overload = false,
     shortCircuit = false,
     highInputVoltage = false,
     lowInputVoltage = false;

void setup()
{  
  DDRB |= 1 << SPWM_OUTPUT_PIN_1;                                                        //Set pin 9 and 10 as output, OCR1A and OCR1B
  DDRB |= 1 << SPWM_OUTPUT_PIN_2;
  DDRB &= ~(1  << SHORT_CIRCUIT_PIN);                                                     //set as input, will be toggled by hardware
  //startupCheck();                                                                       //checks that the input voltage is right
  setupTimers();
  softStart();                                                                            //slow start and checks for shortcircuit
  
}

void loop() 
{
  inputVoltage = (float)analogRead(INPUT_VOLTAGE_FB_PIN) * INPUT_RESISTOR_DIVIDER * (float)voltageScalingFactor;
  outputVoltage = (float)analogRead(OUTPUT_VOLTAGE_FB_PIN) * OUTPUT_RESISTOR_DIVIDER * (float)voltageScalingFactor;
  outputVoltage += (0.4 * OUTPUT_RESISTOR_DIVIDER);                                       //because of diodes  
  error = (float)(SET_OUTPUT_VOLTAGE - outputVoltage);  
  error = map(error, 0, SET_OUTPUT_VOLTAGE,0 ,5);                                         //normalize error to 5, so the maximum error will always be 5 no matter the input voltage and the PID settings don't need to be changed for different voltages
  PIDControllerOutput = PID(error);                                               
 
  if(multiplier > 495 && outputVoltage < OVERLOAD_VOLTAGE)                                           //overload or short 
   {
      runningError = true;
      shutDown();
   }
     

}
