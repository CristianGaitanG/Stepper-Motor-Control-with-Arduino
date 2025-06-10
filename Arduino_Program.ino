#include <stone.h>
#include <stone_config.h>
#include <avr/sleep.h>


extern recive_group STONER;
extern unsigned char STONE_RX_BUF[RX_LEN];
extern unsigned char receive_over_flage;

/* Physical Inputs */
int wakePin = 2;
int pulse = 9;
int dir = 8;
//int inPosition = 10;
bool inPosition = false;

/* Global Variables */ 
String my_name;
int my_status;
int code_machine = 0;
//Stepper Calculation
double distCal = 1.0; //in m
int numSteps = 0; //Calculate number steps for motor
int pulseWidth = 500; //duration of the pulse - 40
int milSteps = 500; //Frequency of pulse - 10
double pi = 3.14159;
double rad = 0.5; //Radious of wheel
int stepsByRev = 800; //Steps by revolution initially was 1600
unsigned long startTime;
unsigned long currentTime;
unsigned long elapsedTime;
unsigned long deltaTime = 30000;
int totalSteps = 0;
//Auto mode
int countDist = 0;
String distText[5];
String desPos;
double distNum;
int autoSteps;

char charBuf[50];
//Manual mode
double manualInc = 0.10; //Manual increment of 10cm each press
int manualSteps = 0;

void setup(){
  /*Defining Inputs*/
  pinMode(wakePin, INPUT);
  //pinMode(inPosition,INPUT);
  
  /*Defining Outputs*/
  pinMode(pulse, OUTPUT);
  pinMode(dir,OUTPUT);
  
  /*Initializing IN/OUTs*/
  digitalWrite (wakePin, HIGH);
  digitalWrite(dir,LOW);

  /*Serial Communication Init*/
  Serial.begin(115200);
  Serial.println("INITIALIZING AUTO POSITION LENGTH STOP");
  Serial.println("TMR 06.....");
}

uint8_t x = 0;
uint8_t clears;


/* FUNCTIONS */
//---------RESET
void(* resetFunc) (void) = 0;

void set_sys(char* m_cmd);
void back_home(void);
void set_text(char* _type, char* _name, char* _text);

/*Sleep*/
void sleepNow(){
    set_brightness("0");
    delay(1000);
}
/*Wake Up*/
void wakeUpNow(){
  delay(500);
  back_home();
  set_brightness("100");
  my_name = "End";
}

int stepsCalculator(double dist){
  double radian = dist/rad;
  double revol = radian/(2*pi*rad);
  int steps = stepsByRev * revol;
  return steps;
}

bool stepMotor_on(int steps){
  int n=0;
  while(n<=steps){
    digitalWrite(pulse,HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(pulse,LOW);
    delayMicroseconds(milSteps);
    Serial.println(n);
    n=n+1;
  }
  inPosition = true;
  return inPosition;
}

void start_cal(){
  Serial.println("calibration process, might take some minutes");
  //Execute commands for Stepper motor
  open_win("dialog3");
  int numSteps = stepsCalculator(distCal);
  Serial.println(numSteps);
  delay(2000);
  startTime = millis();
  while(code_machine == 2){
    Serial.println("Motor running");
    stepMotor_on(numSteps);
    totalSteps=totalSteps + numSteps;
    currentTime = millis();
    elapsedTime = currentTime - startTime;
    if(inPosition==true and elapsedTime <= deltaTime){
      close_win("dialog3");
      delay(500);
      open_win("dialog1");
      code_machine = 0;
      my_name="End";
    }
    else if(elapsedTime > deltaTime){
      close_win("dialog3");
      delay(500);
      open_win("dialog2");
      delay(500);
      my_name="End";
      code_machine = 0;
    }
  }
}

/*          */

void loop() {

  serial_receive();
  if (receive_over_flage == 1){
      my_name = (char*)STONER.widget; 
      my_status = STONER.value;
      Serial.print("name:");
      for (int wds=0;wds<STONER.len-1;wds++)
        Serial.write(my_name[wds]);
      Serial.print("\n");
      receive_over_flage = 0;
  }
  
  /*RESTART SYSTEM*/
  if (my_name == "conreset"){
    Serial.println("System restarting....");
    delay(500);
    set_sys("sys_reboot");
    delay(500);
    my_name = "Init Screen";
    delay(1000);
    resetFunc();
  }

  /*SLEEP MODE SYSTEM*/
  if (my_name == "exit"){
    Serial.println("System in Sleep mode");
    delay(1000);
    sleepNow();
    if (digitalRead(wakePin)==LOW){
      wakeUpNow();
    } 
  }
  
  if (my_name == "setup"){
    Serial.println("Set_Up");
  }

  if (my_name == "calibrate"){
    Serial.println("Calibration");
    code_machine = 1;
  }
  if (my_name == "startcal"){
    Serial.println("Start Calibration....Sending Gauge Stop to Home");
    code_machine = 2;
    start_cal();
  }

  if (my_name == "moveL"){
    digitalWrite(dir,HIGH);
    manualSteps = stepsCalculator(manualInc);
    Serial.println(manualSteps);
    
    totalSteps = totalSteps - manualSteps;
    if (totalSteps < 0){
      totalSteps = 0;
    }
    stepMotor_on(manualSteps);
    Serial.println(totalSteps);
    my_name="End";
  }

  if (my_name == "moveR"){
    //MANUAL MODE
    digitalWrite(dir,LOW);
    manualSteps = stepsCalculator(manualInc);
    Serial.println(manualSteps);
    totalSteps = totalSteps + manualSteps;
    if (totalSteps > 150000){  //ToDo: Check Max Length of Gauge
      totalSteps = 150000;
    }
    //stepMotor_on(accumulateSteps);
    stepMotor_on(manualSteps);
    Serial.println(totalSteps);
    my_name="End";
  }

  if(my_name == "btn1"){
    distText[countDist] = "1";
    countDist = countDist +1;
    my_name="End";
    delay(800);
  }
  if(my_name == "btn2"){
    distText[countDist] = "2";
    countDist = countDist+1;
    my_name="End";
    delay(800);
  }
  if(my_name == "btn3"){
    distText[countDist] = "3";
    countDist = countDist+1;
    my_name="End";
    delay(800);
  }
  if(my_name == "btn4"){
    distText[countDist] = "4";
    countDist = countDist+1;
    delay(800);
    my_name="End";
  }
  if(my_name == "btn5"){
    distText[countDist] = "5";
    countDist = countDist+1;
    delay(800);
    my_name="End";
  }
  if(my_name == "btn6"){
    distText[countDist] = "6";
    countDist = countDist+1;
    delay(800);
    my_name="End";
  }
  if(my_name == "btn7"){
    distText[countDist] = "7";
    countDist = countDist+1;
    delay(800);
    my_name="End";
  }
  if(my_name == "btn8"){
    distText[countDist] = "8";
    countDist = countDist+1;
    delay(800);
  }
  if(my_name == "btn9"){
    distText[countDist] = "9";
    countDist = countDist+1;
    delay(800);
    my_name="End";
  }
  if(my_name == "btn0"){
    distText[countDist] = "0";
    countDist = countDist+1;
    delay(800);
    my_name="End";
  }
  if(my_name == "btndot"){
    distText[countDist] = ".";
    countDist = countDist+1;
    delay(800);
    my_name="End";
  }

  if(my_name =="button8"){
    desPos = distText[0]+distText[1]+distText[2]+distText[3];
    Serial.println(desPos); 
    delay(500);
    countDist = 0;
    distNum = desPos.toDouble();
    Serial.print("Numeric Value: ");
    Serial.println(distNum);
    autoSteps = stepsCalculator(distNum);
    Serial.println(autoSteps);
    autoSteps = totalSteps - autoSteps;
    Serial.print("Auto move: ");
    Serial.println(autoSteps);
    
    if (autoSteps > 0){
     digitalWrite(dir,HIGH);
      stepMotor_on(abs(autoSteps)); 
      my_name="End";
      autoSteps = abs(autoSteps);
      totalSteps = totalSteps-autoSteps;
    }
    if (autoSteps < 0){
      digitalWrite(dir,LOW);
      stepMotor_on(abs(autoSteps));
      my_name="End";
      autoSteps = abs(autoSteps);
      totalSteps = totalSteps + autoSteps;
    }
    Serial.print("Auto move: ");
    Serial.println(totalSteps);
  }  
}
