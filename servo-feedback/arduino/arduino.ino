// Code based on example by Bill Earl (https://learn.adafruit.com/analog-feedback-servos/servos-as-input-devices)
// Serial I/O based on example http://forum.arduino.cc/index.php?topic=396450

// Test commands to send in serial monitor:
//  Report position: <servo,4,0>
//  Move to position 10: <servo,2,10>

#include <Servo.h> 

// Serial communication
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];
char messageFromPC[numChars] = {0};
int integerFromPC = 0;
float floatFromPC = 0.0;
boolean newData = false;

// Control and feedback pins
int servoPin = 9;
int feedbackPin = A0;

// Calibration values
int minDegrees;
int maxDegrees;
int minFeedback;
int maxFeedback;
int tolerance = 2; // max feedback measurement error

Servo servo;  

enum {
  MsgAcknowledge, // 0
  MsgError,       // 1
  MsgMove,        // 2
  MsgMoveResult,  // 3
  MsgPosition,    // 4
  MsgTwist1,       // 5
  MsgTwist2,       // 6
  MsgTwist3       // 7
};


void setup()  {
 Serial.begin(115200);
  
  // Set up servo
  servo.attach(servoPin);   
  calibrate(servo, feedbackPin, 20, 160);  // calibrate for the 20-160 degree range
  servo.detach();   

  report(MsgAcknowledge, "Ready");
} 

void loop() {
  // Process serial communucation
  recvWithStartEndMarkers();

  // If we received a command, process it
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    parseData();
    switch (integerFromPC) {
      case MsgPosition:
        // Report servo position
        report(MsgPosition, getPos(feedbackPin));
        break;
      case MsgMove:
        // Move to location
        moveServo(servo, feedbackPin, floatFromPC);
        break;
       case MsgTwist1:
        twist1();
        break;
       case MsgTwist2:
        twist2();
        break;
       case MsgTwist3:
        twist3();
        break;
      }
      // Debug: print parsed command to serial
      // showParsedData();
      newData = false;
    }
}
// Min funktion

void twist1(){
  moveServo(servo, feedbackPin, 1);
  moveServo(servo, feedbackPin, 180);
  moveServo(servo, feedbackPin, 1);
  moveServo(servo, feedbackPin, 180);
  moveServo(servo, feedbackPin, 1);
  moveServo(servo, feedbackPin, 180);

}

void twist2(){
  Serial.println("move to 1");
  moveServo(servo, feedbackPin, 1);
  delay (300);
  Serial.println("move to 30");
  moveServo(servo, feedbackPin, 30);
  delay (300);
  Serial.println("move to 60");
  moveServo(servo, feedbackPin, 60);
  delay (300);
  Serial.println("move to 90");
  moveServo(servo, feedbackPin, 90);
  delay (300);
  Serial.println("move to 120");
  moveServo(servo, feedbackPin, 120);
  delay (300);
  Serial.println("move to 150");
  moveServo(servo, feedbackPin, 150);
  delay (300);
  Serial.println("move to 180");
  moveServo(servo, feedbackPin, 180);
}
void twist3(){
  servo.attach(servoPin);
  
  Serial.println("move to 1");
  servo.write(1);
  delay (2000);

  Serial.println("move to 30");
  servo.write(30);
  delay (500);
  
  Serial.println("move to 60");
  servo.write(60);
  delay (500);
  
  Serial.println("move to 90");
  servo.write(90);
  delay (500);
  
  Serial.println("move to 120");
  servo.write(120);
  delay (500);
  
  Serial.println("move to 150");
  servo.write(150);
  delay (500);
  
  Serial.println("move to 180");
  servo.write(180);
  delay(500);

  servo.detach();
}



// Calibrate servo, and establish relation between physical movement and
// sensor values
void calibrate(Servo s, int analogPin, int minPos, int maxPos) {
  // Move to the minimum position and record the feedback value
  s.write(minPos);
  minDegrees = minPos;
  delay(1000); // make sure it has time to get there and settle
  minFeedback = analogRead(analogPin);
  
  // Move to the maximum position and record the feedback value
  s.write(maxPos);
  maxDegrees = maxPos;
  delay(1000); // make sure it has time to get there and settle
  maxFeedback = analogRead(analogPin);
}


// Moves the servo a position
void moveServo(Servo s, int analogPin, int pos) {
  int servoAnalogValue = 0;
  
  // Start the move...
  s.attach(servoPin); 
  s.write(pos);
  
  // Calculate the target feedback value for the final position
  int target = map(pos, minDegrees, maxDegrees, minFeedback, maxFeedback); 
  
  // Wait until it reaches the target
  do {
    servoAnalogValue = analogRead(analogPin);
  } while(abs(servoAnalogValue - target) > tolerance);
  
  //while(abs(analogRead(analogPin) - target) > tolerance) {
  //} // wait...

  // Send final - and actual - position
  report(MsgMoveResult, getPos(analogPin));

  // Detach to allow manual manipulation of servo
  s.detach();
}

// Returns position of servo
int getPos(int analogPin) {
  return map(analogRead(analogPin), minFeedback, maxFeedback, minDegrees, maxDegrees);
}


// ---- Serial communication
void report(int code, const char *message) {
  Serial.print("<servo,");
  Serial.write(code);
  Serial.write(",");
  Serial.write(message);
  Serial.write("\r\n");
  Serial.flush();
}

void report(int code, int message) {
  Serial.print("<servo,");
  Serial.print(code);
  Serial.print(",");
  Serial.print(message);
  Serial.print(">\r\n");
  Serial.flush();
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void parseData() {      // split the data into its parts
    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(tempChars,",");      // get the first part - the string
    strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC
 
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    integerFromPC = atoi(strtokIndx);     // convert this part to an integer

    strtokIndx = strtok(NULL, ",");
    floatFromPC = atof(strtokIndx);     // convert this part to a float
}

void showParsedData() {
    Serial.print("Message ");
    Serial.println(messageFromPC);
    Serial.print("Integer ");
    Serial.println(integerFromPC);
    Serial.print("Float ");
    Serial.println(floatFromPC);
}

