
#include <Wire.h>

#include <ctype.h>

#define ADDRESS            11

#define X_DIR_PIN          0
#define X_STEP_PIN         1
#define X_ENABLE_PIN       2

#define Y_DIR_PIN          3
#define Y_STEP_PIN         4
#define Y_ENABLE_PIN       5

#define Z_DIR_PIN          6
#define Z_STEP_PIN         7
#define Z_ENABLE_PIN       8

#define A_DIR_PIN          9
#define A_STEP_PIN         10
#define A_ENABLE_PIN       11

#define X_STEP_HIGH             PORTD |=  0b00000010;
#define X_STEP_LOW              PORTD &= ~0b00000010;

#define Y_STEP_HIGH             PORTD |=  0b00010000;
#define Y_STEP_LOW              PORTD &= ~0b00010000;

#define Z_STEP_HIGH             PORTD |=  0b10000000;
#define Z_STEP_LOW              PORTD &= ~0b10000000;

#define A_STEP_HIGH             PORTB |=  0b00000100;
#define A_STEP_LOW              PORTB &= ~0b00000100;

#define TIMER1_INTERRUPTS_ON    TIMSK1 |=  (1 << OCIE1A);
#define TIMER1_INTERRUPTS_OFF   TIMSK1 &= ~(1 << OCIE1A);

// ^muutettava

//Kokokierros 5760
#define   WHOLE_LAP             5760

#define   defaultStepInterval   900

/*
volatile int clocktime = -1;
volatile int clocktime_previous = -1;
*/

// Järjestys: vasen min, vasen h, oikea min, oikea h; järj: 0,1,2,...8,9, keskustaan osoitus




int newPositions[4] = {0,0,0,0};


volatile bool startMoveFlag;
volatile bool syncOn;
volatile bool calculateMaxMoveTime;
volatile float maxMovementTime;
volatile bool prepMoveFlag;
volatile bool adjSpeedScalesFlag;

String requestedDataLabel;

struct stepperInfo {
  // externally defined parameters
  float acceleration;
  void (*dirFunc)(int);
  void (*stepFunc)();
  int latestStepInterval;

  // derived parameters
  unsigned int c0;                // step interval for first step, determines acceleration
  long stepPosition;              // current position of stepper (total of all movements taken so far)

  // per movement variables (only changed once per movement)
  volatile int dir;                        // current direction of movement, used to keep track of position
  volatile unsigned int totalSteps;        // number of steps requested for current movement
  volatile bool movementDone = false;      // true if the current movement has been completed (used by main program to wait for completion)
  volatile unsigned int rampUpStepCount;   // number of steps taken to reach either max speed, or half-way to the goal (will be zero until this number is known)
  volatile unsigned long estStepsToSpeed;  // estimated steps required to reach max speed
  volatile unsigned long estTimeForMove;   // estimated time (interrupt ticks) required to complete movement
  volatile unsigned long rampUpStepTime;
  volatile float speedScale;               // used to slow down this motor to make coordinated movement with other motors
  volatile int extraLaps;
  int minStepInterval;

  // per iteration variables (potentially changed every interrupt)
  volatile unsigned int n;                 // index in acceleration curve, used to calculate next interval
  volatile float d;                        // current interval length
  volatile unsigned long di;               // above variable truncated
  volatile unsigned int stepCount;         // number of steps completed in current movement
};




// step/dir funktiot:

void xStep() {
  X_STEP_HIGH
  X_STEP_LOW
}
void xDir(int dir) {
  digitalWrite(X_DIR_PIN, dir);
}

void yStep() {
  Y_STEP_HIGH
  Y_STEP_LOW
}
void yDir(int dir) {
  digitalWrite(Y_DIR_PIN, dir);
}

void zStep() {
  Z_STEP_HIGH
  Z_STEP_LOW
}
void zDir(int dir) {
  digitalWrite(Z_DIR_PIN, dir);
}

void aStep() {
  A_STEP_HIGH
  A_STEP_LOW
}
void aDir(int dir) {
  digitalWrite(A_DIR_PIN, dir);
}


void resetStepperInfo( stepperInfo& si ) {
  si.n = 0;
  si.d = 0;
  si.di = 0;
  si.stepCount = 0;
  si.rampUpStepCount = 0;
  si.rampUpStepTime = 0;
  si.totalSteps = 0;
  si.stepPosition = 0;
  si.movementDone = false;
}


#define NUM_STEPPERS 4

volatile stepperInfo steppers[NUM_STEPPERS];




void setup() {
  Wire.begin(ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  //Serial.begin(9600);
  //Serial.println("toimii!");

  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);
  pinMode(A_ENABLE_PIN, OUTPUT);

  digitalWrite(X_ENABLE_PIN, HIGH);
  digitalWrite(Y_ENABLE_PIN, HIGH);
  digitalWrite(Z_ENABLE_PIN, HIGH);
  digitalWrite(A_ENABLE_PIN, HIGH);

  delay(50);

  pinMode(X_STEP_PIN,   OUTPUT);
  pinMode(X_DIR_PIN,    OUTPUT);

  pinMode(Y_STEP_PIN,   OUTPUT);
  pinMode(Y_DIR_PIN,    OUTPUT);

  pinMode(Z_STEP_PIN,   OUTPUT);
  pinMode(Z_DIR_PIN,    OUTPUT);

  pinMode(A_STEP_PIN,   OUTPUT);
  pinMode(A_DIR_PIN,    OUTPUT);

  delay(50);

  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(Y_ENABLE_PIN, LOW);
  digitalWrite(Z_ENABLE_PIN, LOW);
  digitalWrite(A_ENABLE_PIN, LOW);

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 1000;                                     // compare value
  TCCR1B |= (1 << WGM12);                           // CTC mode
  TCCR1B |= ((1 << CS11) | (1 << CS10));            // 64 prescaler
  interrupts();

  startMoveFlag = false;
  syncOn = false;
  calculateMaxMoveTime = false;
  requestedDataLabel = "m_t";                       // m_t -> move_time
  maxMovementTime = 0;
  prepMoveFlag = false;
  adjSpeedScalesFlag = false;

  steppers[0].dirFunc = xDir;
  steppers[0].stepFunc = xStep;
  steppers[0].acceleration = 100000;
  steppers[0].stepPosition = 5760/2;
  steppers[0].extraLaps = 0;
  steppers[0].minStepInterval = defaultStepInterval;
  steppers[0].latestStepInterval = defaultStepInterval;

  steppers[1].dirFunc = yDir;
  steppers[1].stepFunc = yStep;
  steppers[1].acceleration = 100000;
  steppers[1].stepPosition = 5760/2;
  steppers[1].extraLaps = 0;
  steppers[1].minStepInterval = defaultStepInterval;
  steppers[1].latestStepInterval = defaultStepInterval;

  steppers[2].dirFunc = zDir;
  steppers[2].stepFunc = zStep;
  steppers[2].acceleration = 100000;
  steppers[2].stepPosition = 5760/2;
  steppers[2].extraLaps = 0;
  steppers[2].minStepInterval = defaultStepInterval;
  steppers[2].latestStepInterval = defaultStepInterval;

  steppers[3].dirFunc = aDir;
  steppers[3].stepFunc = aStep;
  steppers[3].acceleration = 100000;
  steppers[3].stepPosition = 5760/2;
  steppers[3].extraLaps = 0;
  steppers[3].minStepInterval = defaultStepInterval;
  steppers[3].latestStepInterval = defaultStepInterval;
}



void resetStepper(volatile stepperInfo& si) {
  si.c0 = si.acceleration;
  si.d = si.c0;
  si.di = si.d;
  si.stepCount = 0;
  si.n = 0;
  si.rampUpStepCount = 0;
  si.movementDone = false;
  si.speedScale = 1;

  si.minStepInterval = si.latestStepInterval;
  float a = si.minStepInterval / (float)si.c0;
  a *= 0.676;

  float m = ((a*a - 1) / (-2 * a));
  float n = m * m;

  si.estStepsToSpeed = n;
}

volatile byte remainingSteppersFlag = 0;



float getDurationOfAcceleration(volatile stepperInfo& s, unsigned int numSteps) {
  float d = s.c0;
  float totalDuration = 0;
  for (unsigned int n = 1; n < numSteps; n++) {
    d = d - (2 * d) / (4 * n + 1);
    totalDuration += d;
  }
  return totalDuration;
}


void prepareMovement(int whichMotor, long steps) {
  volatile stepperInfo& si = steppers[whichMotor];
  si.dirFunc( steps < 0 ? HIGH : LOW );
  si.dir = steps > 0 ? 1 : -1;
  si.totalSteps = abs(steps);
  resetStepper(si);

  remainingSteppersFlag |= (1 << whichMotor);

  unsigned long stepsAbs = abs(steps);

  if ( (2 * si.estStepsToSpeed) < stepsAbs ) {
    // there will be a period of time at full speed
    unsigned long stepsAtFullSpeed = stepsAbs - 2 * si.estStepsToSpeed;
    float accelDecelTime = getDurationOfAcceleration(si, si.estStepsToSpeed);
    si.estTimeForMove = 2 * accelDecelTime + stepsAtFullSpeed * si.minStepInterval;
  }
  else {
    // will not reach full speed before needing to slow down again
    float accelDecelTime = getDurationOfAcceleration( si, stepsAbs / 2 );
    si.estTimeForMove = 2 * accelDecelTime;
  }
}


volatile byte nextStepperFlag = 0;

void setNextInterruptInterval() {

  bool movementComplete = true;

  unsigned long mind = 999999;
  for (int i = 0; i < NUM_STEPPERS; i++) {
    if ( ((1 << i) & remainingSteppersFlag) && steppers[i].di < mind ) {
      mind = steppers[i].di;
    }
  }

  nextStepperFlag = 0;
  for (int i = 0; i < NUM_STEPPERS; i++) {
    if ( ! steppers[i].movementDone )
      movementComplete = false;
    if ( ((1 << i) & remainingSteppersFlag) && steppers[i].di == mind )
      nextStepperFlag |= (1 << i);
  }

  if ( remainingSteppersFlag == 0 ) {
    TIMER1_INTERRUPTS_OFF
    OCR1A = 65500;
  }

  OCR1A = mind;
}


// Interrupt method to run steppers
ISR(TIMER1_COMPA_vect)
{
  unsigned int tmpCtr = OCR1A;

  OCR1A = 65500;

  for (int i = 0; i < NUM_STEPPERS; i++) {

    if ( ! ((1 << i) & remainingSteppersFlag) )
      continue;

    if ( ! (nextStepperFlag & (1 << i)) ) {
      steppers[i].di -= tmpCtr;
      continue;
    }

    volatile stepperInfo& s = steppers[i];

    if ( s.stepCount < s.totalSteps ) {
      s.stepFunc();
      s.stepCount++;
      s.stepPosition += s.dir;
      if ( s.stepCount >= s.totalSteps ) {
        s.movementDone = true;
        remainingSteppersFlag &= ~(1 << i);
      }
    }

    if ( s.rampUpStepCount == 0 ) {
      s.n++;
      s.d = s.d - (2 * s.d) / (4 * s.n + 1);
      if ( s.d <= s.minStepInterval ) {
        s.d = s.minStepInterval;
        s.rampUpStepCount = s.stepCount;
      }
      if ( s.stepCount >= s.totalSteps / 2 ) {
        s.rampUpStepCount = s.stepCount;
      }
      s.rampUpStepTime += s.d;
    }
    else if ( s.stepCount >= s.totalSteps - s.rampUpStepCount ) {
      s.d = (s.d * (4 * s.n + 1)) / (4 * s.n + 1 - 2);
      s.n--;
    }

    s.di = s.d * s.speedScale; // integer
  }

  setNextInterruptInterval();

  TCNT1  = 0;
}


// Calculate the max time for own steppers to complete the movement.
float calcMaxMoveTime() {
  float maxTime = 0;

  for (int i = 0; i < NUM_STEPPERS; i++) {
    if ( ! ((1 << i) & remainingSteppersFlag) )
      continue;
    if ( steppers[i].estTimeForMove > maxTime )
      maxTime = steppers[i].estTimeForMove;
  }

  return maxTime;
}


// Adjust speed scale to match the longest time any stepper needs to complete movement.
void adjustSpeedScales() {
  float maxTime = 0;

  for (int i = 0; i < NUM_STEPPERS; i++) {
    if ( ! ((1 << i) & remainingSteppersFlag) )
      continue;
    if ( steppers[i].estTimeForMove > maxTime )
      maxTime = steppers[i].estTimeForMove;
  }

  if (maxTime != 0 && maxMovementTime > maxTime) {
    maxTime = maxMovementTime;
  }

  if ( maxTime != 0 ) {
    for (int i = 0; i < NUM_STEPPERS; i++) {
      if ( ! ( (1 << i) & remainingSteppersFlag) )
        continue;
      steppers[i].speedScale = maxTime / steppers[i].estTimeForMove;
    }
  }
}


void runAndWait() {
  setNextInterruptInterval();
  TIMER1_INTERRUPTS_ON
  while ( remainingSteppersFlag );
  remainingSteppersFlag = 0;
  nextStepperFlag = 0;
}


// Steps to reach target position
int getsteps(int whichMotor, int target)
{
    volatile stepperInfo& si = steppers[whichMotor];
    int steps = target - si.stepPosition;
    int extraRounds = si.extraLaps;

    if (steps > 0 and steps <= WHOLE_LAP/2)
    {
        steps = steps;
    }
    else if (steps < 0 and steps >= -WHOLE_LAP/2)
    {
        steps = steps;
    }

    else if (steps > 0 and steps > WHOLE_LAP/2)
    {
        steps = -(WHOLE_LAP-steps);
    }
    else
    {
        steps = WHOLE_LAP+steps;
    }

    // Extra laps:
    if(steps < 0 && extraRounds > 0) {
      steps -= extraRounds * WHOLE_LAP;
    }
    else if(steps > 0 && extraRounds > 0) {
      steps += extraRounds * WHOLE_LAP;
    }

    return steps;
}


// Fix stepper positions to be non-negative & between 0 and WHOLE_LAP
void fixPositions() {

    for (int i = 0; i < 4; i++)
    {
      while (true)
      {
        volatile stepperInfo& si = steppers[i];

        if (si.stepPosition >= WHOLE_LAP)
        {
            si.stepPosition -= WHOLE_LAP;
        }
        if (si.stepPosition < 0)
        {
            si.stepPosition += WHOLE_LAP;
        }

        if (si.stepPosition >= 0 && si.stepPosition < WHOLE_LAP)
        {
          break;
        }
      }
    }
}


void receiveEvent(int howMany) {
  
  String data = "";
  while( Wire.available()){
    data += (char)Wire.read();
  }

  // New target positions
  if (isDigit(data[0])) {
    int clockNumber = ((String)data[0]).toInt();
    int clockPos = data.substring(2).toInt();

    newPositions[clockNumber] = clockPos;
  }

  // Sync next movement
  else if(data.indexOf("sync") > -1) {
    adjSpeedScalesFlag = true;
  }

  // Prepare movement
  else if (data.indexOf("prepare") > -1) {
    prepMoveFlag = true;
  }

  // Start movement
  else if (data.indexOf("move") > -1) {
    startMoveFlag = true;
  }

  // Data request type change & start prep:
  else if (data.indexOf("req") > -1) {
    requestedDataLabel = data.substring(4);
  }

  // Set movement time
  else if (data[0] == 't') {
    float time = data.substring(2).toFloat();
    maxMovementTime = time;
  }

  // Set acceleration parameters
  else if (data[0] == 'a') {
    if(isDigit(data[1])){
      int stNum = String(data[1]).toInt();
      int newAcc = data.substring(3).toInt();
      steppers[stNum].acceleration = newAcc;
    }
    else{
      int newAcc = data.substring(2).toInt();
      steppers[0].acceleration = newAcc;
      steppers[1].acceleration = newAcc;
      steppers[2].acceleration = newAcc;
      steppers[3].acceleration = newAcc;
    }
  }

  // Extra loops
  else if (data[0] == 'l') {
    if(isDigit(data[1])){
      int stNum = String(data[1]).toInt();
      int laps = data.substring(3).toInt();
      steppers[stNum].extraLaps = laps;
    }
    else{
      int laps = data.substring(2).toInt();
      steppers[0].extraLaps = laps;
      steppers[1].extraLaps = laps;
      steppers[2].extraLaps = laps;
      steppers[3].extraLaps = laps;
    }
  }


  // Set min step interval parameters
  else if (data[0] == 's') {
    if (isDigit(data[1])){
      int stNum = String(data[1]).toInt();
      int newStepInterval = data.substring(3).toInt();
      steppers[stNum].latestStepInterval = newStepInterval;
    }
    else {
      int newStepInterval = data.substring(2).toInt();
      steppers[0].latestStepInterval = newStepInterval;
      steppers[1].latestStepInterval = newStepInterval;
      steppers[2].latestStepInterval = newStepInterval;
      steppers[3].latestStepInterval = newStepInterval;
    }
  }
}


void requestEvent() {
  // Data requested is the previously calculated (max) move_time
  if (requestedDataLabel == "m_t") {
    Wire.write(String(maxMovementTime).c_str());
  }
}


void loop() {
    // Move preparation requested:
    if (prepMoveFlag) {
      for (int i = 0; i < NUM_STEPPERS; i++)
      {
        if (steppers[i].stepPosition != newPositions[i])
        {
          prepareMovement(i, getsteps(i, newPositions[i]));
        }
      }
      maxMovementTime = calcMaxMoveTime();
      prepMoveFlag = false;
    }

    /* Adjust speed scales requested. Done before movement 
     when clock hands are moved in sync. */
    if (adjSpeedScalesFlag) {
      adjustSpeedScales();
      adjSpeedScalesFlag = false;
    }

    // Movement start requested:
    if (startMoveFlag)
    {
      runAndWait();
      fixPositions();
      maxMovementTime = 0;
      startMoveFlag = false;
    }
}
