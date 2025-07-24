// ENGR 111 Cornerstone Project Skeleton Code
#include <LiquidCrystal_I2C.h>
#include <string.h>

/**********************************************************/
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
/**********************************************************/

// Pin assignments
const int buttonPin = 3;
const int pumpPos = 4;
const int pumpNeg = 5;
const int valveNeg = 6;
const int valvePos = 7;
const int echoPin = 12;    // echo for ultrasonic sensor
const int trigPin = 11;    // trigger for ultrasonic sensor
const int turbPin = A0;
const int simTurbPin = A3;

// LCD settings
int displaySetting = 0;
const int maxDisplays = 5;
unsigned long lastDisplaySwitch = millis();
const int displayDelay = 250;

// YOUR GLOBAL VARIABLES SHOULD BE DECLARED HERE
float turbid_value = turbPin;
int valve_status = 0;
float sim_turb = simTurbPin;
float tank_volume = 0;
float flow_rate = 0;
int i = 1;

void setup() {
  // initialize LCD & its backlight
  lcd.init();
  lcd.backlight();

  // initialize pushbutton for LCD toggle
  pinMode(buttonPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), changeDisplaySetting, FALLING);

  // initialize pins for ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // initialize pins used for flow control
  pinMode(turbPin, INPUT);
  pinMode(pumpPos, OUTPUT);
  pinMode(pumpNeg, OUTPUT);
  pinMode(valvePos, OUTPUT);
  pinMode(valveNeg, OUTPUT);
  pinMode(simTurbPin, INPUT);

  // ALWAYS keep these low!!!
  digitalWrite(pumpNeg, LOW);
  digitalWrite(valveNeg, LOW);

  // IF you want these to turn on, write them HIGH
  digitalWrite(pumpPos, LOW);
  digitalWrite(valvePos, LOW);
}

void loop() {
  // Set inital variable values

  unsigned long sTime = 0;
  unsigned long eTime = 0;
  float sVol ;
  float eVol = 0;
  turbid_value = analogRead(turbPin);
  sim_turb = analogRead(simTurbPin);
  tank_volume = findVolume();

  valve_status = digitalRead(valvePos);
  sVol = findVolume();
  float dist = checkDist();

  // Set turbidity threshold
  float turbidity_threshold = 550; // Adjust the threshold value based on your requirements

  // Check if the turbidity is above the threshold
  if (sim_turb > turbidity_threshold) {
    // Open valve and start pump
    digitalWrite(valvePos, HIGH);
    digitalWrite(pumpPos, HIGH);
  }else if (sim_turb <= turbidity_threshold) {
    digitalWrite(valvePos,LOW);
  }
  
  // Update the valve status
  valve_status = digitalRead(valvePos);
  //water too high
  if(dist < 11.5){
    //if both the pump and valve are off, start recirculation
    //find start time and vol
    if((digitalRead(pumpPos)==LOW) && (digitalRead(valvePos)==LOW) && (i == 1)){
      sVol = findVolume();
      sTime = millis();
      i = i+1;
      digitalWrite(pumpPos,HIGH);
    }else{
      digitalWrite(pumpPos,HIGH);
    }
  }

  //low water
  if((dist >= 11.5) && (i > 1)){

    digitalWrite(pumpPos,LOW);
    digitalWrite(valvePos,LOW);
    eTime = millis();
    eVol = findVolume();
   flow_rate = ((eVol-sVol)/(eTime-sTime));
  }




  displayLCD();
  delay(500);
}


//Call this function when you want to update the LCD Display
//The ONLY changes you should make to this function are the variables inside the lcd.print for each case!
void displayLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  switch(displaySetting%maxDisplays)
  {
    case 0:
    lcd.print("Water Turbidity");
    lcd.setCursor(0, 1);
    lcd.print(turbid_value);
    //The names are all correct but the "DATA 1" needs to be swaped with the analog sensor data
    break;
    case 1:
    lcd.print("Recirculation Tank Volume");
    lcd.setCursor(0, 1);
    lcd.print(tank_volume);
    break;
    case 2:
    lcd.print("Simulated Turbidity");
    lcd.setCursor(0, 1);
    lcd.print(sim_turb);
    break;
    case 3:
    lcd.print("Motorized Valve State");
    lcd.setCursor(0, 1);
    if(valve_status==0){
    lcd.print("Recirculation");
    }else if(valve_status==1){
    lcd.print("Discharge");
    }
    break;
    case 4:
    lcd.print("Discharge Flow Rate");
    lcd.setCursor(0, 1);
    if(flow_rate != 0.00){
      lcd.print(flow_rate);
    }else{
      lcd.print("Not Discharging Yet");
    }

    break;
    default:
    lcd.print("Unknown Setting!");
  }
}

void changeDisplaySetting() {
    if(lastDisplaySwitch + displayDelay < millis()) { // this limits how quickly the LCD Display can switch
    lastDisplaySwitch = millis();
    displaySetting++;
  }
}

//Call this function to get a distance measurement in cm from the Ultrasonic sensor
float checkDist()
{
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  float d = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Returns the distance in cm
  return d;
}
float findVolume(){
  float d = checkDist();
  float v = (((5.125-d/2.54)*10*6.25)+28)*16.39;
  return v;
}