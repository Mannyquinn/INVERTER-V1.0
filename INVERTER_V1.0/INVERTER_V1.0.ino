/*This program is written with the intent of being used for the inverter, in no way do i assume it perfect by any margin.
 * There is room for optimization, feel free to make copies and edit.
 * use at your own risk haha :)
 * 
 * written by Eltolad Elisha
 */

#define SPWM_OUTPUT_PIN_1 PB1 
#define SPWM_OUTPUT_PIN_2 PB2
#define OUTPUT_VOLTAGE_FB_PIN A0                                                                //input voltage feedback pin
#define INPUT_VOLTAGE_FB_PIN  A1                                                                //onput voltage feedback pin
#define SATURATION_MAX 490                                                                      //constants used to avoid integration windup
#define SATURATION_MIN 10
#define SHORT_CIRCUIT_PIN PB4                                                                   //this pin is an input and is trigered by the hardware short circuit / overload circuit
#define SHUT_DOWN_PIN PB5                                                                       //When the microsontroller sees a fault, it will disable the hardware by pulling this pin
#define MULTIPLIER_MAX_VALUE 500                                                                //must be less than 500
#define VOLTAGE_REFERENCE 5.0                                                                   //the arduino reference voltage for the ADC
                                               

/* output parameter settings */
#define SET_OUTPUT_VOLTAGE  325                                                                  //325V peak not RMS since the voltage feedback is a divider and peak detection circuit [325 = 230*sqrt(2)]
#define INPUT_RESISTOR_DIVIDER 6                                                                 //the resistor divider used in the DC input, i used a 10k and 2k giving a divsion on 6
#define OUTPUT_RESISTOR_DIVIDER 200                                                              //resistor divider at the AC output, i used a 200k, 1k and 0.1uF cap giving a division of approximately 200 at 50Hz
#define OVERLOAD_VOLTAGE 288                                                                     //if the microcontroller senses the voltage to be 288v peak (200v RMS) at the maximum duty cycle, there is overload


/* PID parameters */
#define K_proportional 4
#define K_integral 1
#define K_derivative 0
#define deltaTime 0.04                                                                           //250hz sample time

float voltageError = 0;
const float Kp = K_proportional;
const float Ki = K_integral * deltaTime;
const float Kd = K_derivative / deltaTime;


/*other variables*/
volatile int PIDControllerOutput;
volatile int multiplier = 0;
volatile uint8_t sineArrayIndex = 0;
volatile int sineHalf = 1;
const float voltageScalingFactor =  float(VOLTAGE_REFERENCE / 1023.0);


 /* function declarations*/
void softStart(),
     setupTimers(),
     startupCheck(),
     PID(float&),
     systemErrorCheck(),
     readSystemParameters();


/*system parameters*/
float inputVoltage,
      inputVoltageLowValue = 12.0,              
      inputVoltageHighValue = 14.0,
      outputVoltage = 0;

        
/*system check errors*/                   //these can be used for troubleshooting
bool startupError = false,                //can be caused by undevoltage or overvoltage
     runningError = false,                //can be caused by undervoltage, overvoltage, shortcircuit or overloading
     overload = false,
     shortCircuit = false,
     highInputVoltage = false,
     lowInputVoltage = false,
     overHeating = false;


void setup()
{ 
  DDRB |= 1 << SPWM_OUTPUT_PIN_1;                                                        //Set pin 9 and 10 as output, OCR1A and OCR1B
  DDRB |= 1 << SPWM_OUTPUT_PIN_2;
  
  DDRB &= ~(1  << SHORT_CIRCUIT_PIN);                                                    //set as input, will be toggled by hardware
  PORTB |= 1 << SHORT_CIRCUIT_PIN;                                                       //set as PULLUP
  
  DDRB |= 1 << SHUT_DOWN_PIN;                                                            //this pin shuts down the IR2104 and SG3525, basically the whole system
  PORTB &= ~(1 << SHUT_DOWN_PIN);
  
  startupCheck();                                                                        //checks that the input voltage is right
  setupTimers();
  softStart();                                                                           //slow start and checks for shortcircuit
  
}

void loop() 
{
  readSystemParameters();
  systemErrorCheck();
  /*(SET_OUTPUT_VOLTAGE - outputVoltage) is the error, but since the output voltage can be set to different value, we can map the error between 0 and  its maximum value....this will help remove the need to retune the PID constants */
  voltageError = map((SET_OUTPUT_VOLTAGE - outputVoltage), 0, SET_OUTPUT_VOLTAGE, 0, 100);                                       
  PID(voltageError);  
}



