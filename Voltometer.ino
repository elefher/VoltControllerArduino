
float lowVoltThreshold = 0;
float highVoltThreshold = 0;
float tolerance = 0;
float upAlarmVolts = 0;
float lowAlarmVolts = 0;
float voltage = 0;

int lowVoltLed = 2;
int highVoltLed = 4;
int highRelay = 4;
int lowRelay = 2;
int alarmLed = 7;

// the setup routine runs once when you press reset:
void setup() {
  lowVoltThreshold = 2.9;  
  highVoltThreshold = 4.5;
  tolerance = 0.3;

  highAlarmVolts = highVoltThreshold + tolerance;
  lowAlarmVolts = lowVoltThreshold - tolerance;

  pinMode(lowVoltLed, OUTPUT);
  pinMode(highVoltLed, OUTPUT);
  pinMode(highRelay, OUTPUT);
  pinMode(lowRelay, OUTPUT);
  pinMode(alarmLed, OUTPUT);
  
  // initialize serial communication at 9600 bits per seconds  
  Serial.begin(9600);
}

void loop() {
  if(lowVoltThreshold == 0 || highVoltThreshold == 0 || lowVoltThreshold >= highVoltThreshold){
    return;
  }
  
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltage = sensorValue * (5.0 / 1023.0);  

  if(voltage <= lowVoltThreshold && voltage > lowAlarmVolts){
    setProcess("DOWN");
  }

  if(voltage >= highVoltThreshold && voltage < highAlarmVolts){
    setProcess("UP");
  }

  if(voltage > highAlarmVolts || voltage < lowAlarmVolts){
    alarmProcess();
  }
}

void alarmProcess(){
  setLed(alarmLed, HIGH);
  setLed(lowVoltLed, HIGH);
  setRelay(lowRelay, HIGH);
  setLed(highVoltThreshold, LOW);
  setRelay(lowRelay, LOW);
}

void setProcess(String process){
  if(process.equals("DOWN")){
    setRelay(highRelay, LOW);
    setRelay(lowRelay, HIGH);
    setLed(lowVoltLed, HIGH);
    setLed(highVoltThreshold, LOW);
    setLed(alarmLed, LOW);
  }else if(process.equals("UP")){
    setRelay(lowRelay, LOW);
    setRelay(highRelay, HIGH);
    setLed(highVoltThreshold, HIGH);
    setLed(lowVoltLed, LOW);
    setLed(alarmLed, LOW);
  }
}

void setRelay(int relayId, int statusRelay){
  digitalWrite(relayId, statusRelay);
}

void setLed(int ledId, int statusLed){
  digitalWrite(ledId, statusLed);
}

