#define SPWM_OUTPUT_PIN_1 PB1 
#define SPWM_OUTPUT_PIN_2 PB2
#define OUTPUT_VOLTAGE_FB_PIN A0               //output voltage feedback pin
#define INPUT_VOLTAGE_FB_PIN  A1
#define SATURATION_MAX 490                                                    //used to avoid integration windup
#define SATURATION_MIN 10
#define SHORT_CIRCUIT_PIN PB0                                                        //resistor dividers used on output
#define MULTIPLIER_SOFTSTART_VALUE 500
#define VOLTAGE_REFERENCE 5.0
                                               

/* output parameter settings */
#define SET_OUTPUT_VOLTAGE 5                                                                  //5V
#define INPUT_RESISTOR_DIVIDER 3                                                              //the resistor divider used in the DC input
#define OUTPUT_RESISTOR_DIVIDER 1.0                                                           //resistor divider at the AC output
#define OUTPUT_VOLTAGE_SOFTSTART_CRITERIA 50                                                  //50V, after the soft start has ended the output voltage must be minimum this set value


/* PID parameters */
#define Kp 0
#define Ki 10
#define Kd 0


int PIDControllerOutput;
volatile int multiplier;
volatile uint8_t sineArrayIndex = 0;
volatile int sineHalf = 1;
const float voltageScalingFactor =  VOLTAGE_REFERENCE / 1023.0;

 /* function declarations*/
void softStart();
void setupTimers();
uint16_t PID(int);
void startupCheck();


/*system parameters*/
float   inputVoltage,
        inputVoltageLowValue = 11.0,
        inputVoltageHighValue = 14.0,
        outputVoltage = 0;

        
/*system check errors*/
bool startupError = false,                //can be caused by undevoltage or overvoltage
     runningError = false;                //can be caused by undervoltage, overvoltage, shortcircuit or overloading

void setup()
{  
  analogReference(DEFAULT);
  Serial.begin(9600);
  DDRB |= 1 << SPWM_OUTPUT_PIN_1;                                                        //Set pin 9 and 10 as output, OCR1A and OCR1B
  DDRB |= 1 << SPWM_OUTPUT_PIN_2;
  DDRB &= ~(1  << SHORT_CIRCUIT_PIN);                                                     //set as input, will be toggled by hardware
  //startupCheck();                                                                       //checks that the input voltage is right
  setupTimers();
  //softStart();                                                                            //slow start and checks for shortcircuit
 // pinMode(A0, INPUT);
  
}

void loop() 
{
  static float error;
  inputVoltage = (float)analogRead(INPUT_VOLTAGE_FB_PIN) * INPUT_RESISTOR_DIVIDER * (float)voltageScalingFactor;
  outputVoltage = (float)analogRead(OUTPUT_VOLTAGE_FB_PIN) * OUTPUT_RESISTOR_DIVIDER * (float)voltageScalingFactor;

  error = SET_OUTPUT_VOLTAGE - outputVoltage;
  PIDControllerOutput = PID(error);
  Serial.print(error);
  Serial.print(" ");
  Serial.println(PIDControllerOutput);




















                                                     
 
//  if(multiplier > 495 && outputVoltage < 0)                                           //overload or shor 
//   {
//      runningError = true;
//      shutDown();
//   }
     

}
