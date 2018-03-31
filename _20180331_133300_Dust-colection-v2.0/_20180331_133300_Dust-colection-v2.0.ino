/*
   Title      : automatic dust collection
   Author     : Johannes Röring (joro28062002)
   Date       : 31.03.2018
   Id         : 20180331_133300_Dust-colection-v2.0
   Version    : 0011
   Arduino build : 1.8.2

   DISCLAIMER :
   The author is in no way responsible for any problems or
   damage caused by using this code. Use at your own risk.

   LICENSE    :
   This code is distributed under the GNU Public License
   which can be found at http://www.gnu.org/licenses/gpl.txt

   NOTE       :
   this sketch is only designed to sense current on AC circuits
   all time related values are given in milliseconds (1000ms equal 1 Second)
   if you are receiving any errors while compiling or running the code
   feel free to write me an E-Mail at joro28062002@gmail.com (please title mails with programming so that i'll find it)
   if you need any help understanding what this program does (because i used some selfmade functions just because i'm a little lazy)
   or even want a custom version of this also feel free to contact me

   SOURCES    :
    the whole project is also avaivable at github in case you mess anything up
    Arduino Software
    ACS712 Library https://github.com/muratdemirtas/ACS712-arduino-1
*/
////////////////////////////////////////////////////////////////////
//                       USER EDITABLE PART                       //
////////////////////////////////////////////////////////////////////

//general definitions are done here change them to your needs
#include "ACS712.h"                                //includes the needed library to run the sensor (link to the library can be found at the top of the sketch)
//#define sensorType ACS712_05B                      //uncomment the type of ACS712 you have and comment the other 2
//#define sensorType ACS712_20A                      //uncomment the type of ACS712 you have and comment the other 2
#define sensorType ACS712_30A                      //uncomment the type of ACS712 you have and comment the other 2
#define DEBUG true                                 //enables/disables the debugging output printing all internal values that can be useful to find bugs or wiring mistakes like a permanently grounded switch
#define baud 9600                                  //defines the speed of communication used for the debugging interface (only needed if debug=true)
#define treshold 0.4                               //defines the minimum current for the arduino to toggle the relay when running automatic mode
#define frequency 50                               //sets the frequency of the power grid in your region
#define relayTriggerLevel HIGH                     //determines the behaviour of the relay when it gets triggered (if your relay activates when recieving a high signal set the flag to HIGH if not set it to LOW)
//#define turnOnTimeout 1000                         //uncomment this if you want the relay to activate delayed to the machine (this only affects the automatic mode)
//#define turnOffTimeout 5000                        //uncomment this if you want the relay to deactivate delayed to the machine in order to let dust exit the piping system to prevent blocking of the pipes(this only affects the automatic mode)

//all external devices are adressed to their connented pins in this section
#define indicatorLedPin LED_BUILTIN                //defines on which digital pin a visual indicator is attached, but because most arduino boards feature a onboard led (most times on pin 13) the variable LED_BUILTIN can be used to adress this led
#define relayPin 2                                 //defines to which digital pin the relay is attached
#define autoModePin 3                              //defines at which digital pin the switch (connecting the pin to GND) that is needed to activate the auto mode is attatched
#define manModePin 4                               //defines at which digital pin the switch (connecting the pin to GND) that is needed to activate the manual mode is attatched
#define sensorPin A0                               //defines at which ANALOG input pin the ACS712 sensor is attached

////////////////////////////////////////////////////////////////////
//DONT CHANGE ANYTHING FROM HERE UNLES YOU KNOW WHAT YOU ARE DOING//
////////////////////////////////////////////////////////////////////
#define Serial if(DEBUG)Serial                    //this piece allows writing serial commands without caring about the state of the debug flag each time you use Serial.___(found at https://forum.arduino.cc/index.php?topic=155268.0)
ACS712 sensor(sensorType, sensorPin);             //the acs712 library gets initialized here and also declares that any corresponding function starts with sensor.___

//global variables are declared here
bool state = 0;                                   //this variable is used to store the curent state of the relais (useful for debugging)
bool lastState = 0;                               //this variable is used to determine if a turnOnTimeout or TurnOffTimeOut has to be executed relating to the fact that it has been previously executed or not

void setup () {
  Serial.begin(baud);
//input/output declarations (all inputs have an active pullup resistor so you need to switch them to gnd)
  pinMode(indicatorLedPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(autoModePin, INPUT_PULLUP);
  pinMode(manModePin, INPUT_PULLUP);
  digitalWrite(indicatorLedPin, LOW);
//pause for 1 second and start calibrating the sensor (indicated by the onboard led), then wait another second and enter the looping program flow
  delay(1000);
  digitalWrite(indicatorLedPin, HIGH);
  float zeroPoint = sensor.calibrate();
  Serial.print("Zero point for sensor is ");
  Serial.print(zeroPoint);
  delay(1000);
  digitalWrite(indicatorLedPin, LOW);
}
//this function is only used to short digitalRead(__) to a quick DRead(__) and to invert the values coming in so that when a switch is closed the value is 1 and not 0
bool DRead(int pin) {
  bool val = digitalRead(pin);
  return !val;
}
//now follows the debugging function used to output several internal values of the arduino
void printString(float current, bool autoMode, bool manMode, bool state) {
  Serial.print(current);
  Serial.print(",");
  Serial.print(manMode);
  Serial.print(",");
  Serial.print(autoMode);
  Serial.print(",");
  Serial.println(state);
}
void set(bool state) {
  int val = state;
  digitalWrite(indicatorLedPin, state);
  if (val == 1) val = relayTriggerLevel;
  else val = !relayTriggerLevel;
//now follows the control structure for turnOn-/Off-Timeout
  if (state == lastState);
  else if (state > lastState) {
    #ifdef turnOnTimeout
    delay(turnOnTimeout);
    #endif
  }
  else if (state < lastState) {
    #ifdef turnOffTimeout
    delay(turnOffTimeout);
    #endif
  }
//and now the value is written to the relay
  digitalWrite(relayPin, val);
  lastState = state;
}
void loop () {
  float current = sensor.getCurrentAC(frequency); //converts the sensor input to a value given in amps and stores this value temporarily for use in the void loop() part (after execution of void loop the variable needs to be declared again)
  bool autoMode = DRead(autoModePin);             //stores the state of the switch for this mode
  bool manMode = DRead(manModePin);               //stores the state of the switch for this mode
  printString(current, autoMode, manMode, state); //gives debug informations via serial
//the following structure checks if manual or automatic mode has to be activated and runs the corresponding functions
  if (manMode) {
    state = 1;
  }
  else if (autoMode) {
    if (current >= treshold) state = 1;
    else state = 0;
  }
  else {
    state = 0;
  }
  set(state);                                     //this passes the relay state to the function which writes the value to the relay in relation to the behavior of the relay
}
