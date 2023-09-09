//1st Step motor
#define DIR_PIN1 2
#define STEP_PIN1 4

//2nd Step motor
#define DIR_PIN2 3
#define STEP_PIN2 5

//Control motor
#define MOTOR 6
#define PWM 7

#define prox_center_switch 8//Metal detect sensor
#define prox_left_switch 10//Metal detect sensor
#define prox_right_switch 9//Metal detect sensor
#define prox_release_switch 44

String key = "-1" ;
int last_state = 3;//last state of Control motor

bool cw = LOW;//Forward Direction
bool ccw = HIGH;//Backward Direction

void motor_control(int speed,bool direction){ // motor control function
  digitalWrite(MOTOR, direction);
  analogWrite(PWM,speed);
}

void steps(int dpin, //direction pin
           int spin, //step pin
           int steps, //how many step
           int delaytime //how long for one step
           ){//step motor
  //direction
  int dir = (steps>0)?LOW:HIGH;
  digitalWrite(dpin,dir); 

  //move step motor
  for(int i = 0;i < abs(steps);i++){
    digitalWrite(spin, HIGH); 
    delayMicroseconds(delaytime); 
    digitalWrite(spin, LOW); 
    delayMicroseconds(delaytime); 
    //Serial.println(i);
  }
} 

void push(){//1.566 s
  int step = 940, 
      time = 800;
  steps(DIR_PIN1,STEP_PIN1,step,time);
  delay(250);
  steps(DIR_PIN1,STEP_PIN1,-1*(step+15),time);
}

void release(){//1.73 s
  int step = 235, 
      time = 700;
  steps(DIR_PIN1,STEP_PIN1,step,time);
  
  step = 740, time = 1000;
  steps(DIR_PIN2,STEP_PIN2,step,time);
  delay(250);
  steps(DIR_PIN2,STEP_PIN2,-1*step,time);
  if(digitalRead(prox_release_switch) == 0){
    digitalWrite(DIR_PIN2,HIGH); 
    while(digitalRead(prox_release_switch) == 0){
      digitalWrite(STEP_PIN2, HIGH); 
      delayMicroseconds(time); 
      digitalWrite(STEP_PIN2, LOW); 
      delayMicroseconds(time); 
    }
  }
  step = 250, 
  time = 700;
  steps(DIR_PIN1,STEP_PIN1,-1*step,time);

}

long microsecondsToCentimeters(long microseconds){//distance for ultrasonic sensor
  return microseconds / 29 / 2;
}

int ultra(int trig,//trig pin
          int echo//echo pin
          ){//ultrasonic sensor working
  long duration, cm;

  pinMode(trig, OUTPUT);
  
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(5);
  digitalWrite(trig, LOW);
  
  pinMode(echo, INPUT);
  
  duration = pulseIn(echo, HIGH);
  
  cm = microsecondsToCentimeters(duration);
  
  return cm;
}

int PercenT(int cm){
  if(cm <= 30){
    return 100;
  }
  else{
    return 0;
  }
  
}

void setup() {
  Serial.begin(9600);

  pinMode(DIR_PIN1, OUTPUT);
  pinMode(DIR_PIN2, OUTPUT);

  pinMode(STEP_PIN1, OUTPUT);
  pinMode(STEP_PIN2, OUTPUT);

  pinMode(MOTOR,OUTPUT);
  pinMode(PWM,OUTPUT);

  pinMode(prox_release_switch,INPUT);
  pinMode(prox_center_switch,INPUT);
  pinMode(prox_left_switch,INPUT);
  pinMode(prox_right_switch,INPUT);
 
  analogWrite(PWM,0);
}

void loop() {
  
  if (Serial.available() > 0) {
    key = Serial.readStringUntil('\n');
  
    if(key == "0"){//bin#1 -can
      motor_control(140,cw);
      while(digitalRead(prox_left_switch) == 0){delay(1);}
      motor_control(0,cw);
      push();
      last_state = 0;
    }
    
    if(key == "1"){//bin#2 -pete
      motor_control(140,cw);
      while(digitalRead(prox_left_switch) == 0){delay(1);}
      motor_control(0,cw);
      release();
      last_state = 1;
    }
    
    if(key == "2"){//bin#4 -plastic
      motor_control(150,ccw);
      while(digitalRead(prox_right_switch) == 0){delay(1);}
      motor_control(0,ccw);
      release();
      last_state = 2;
    }
    
    if(key == "3"){//bin#3 -other
      release();
      last_state = 3;
    }
    
    delay(300);
    
    if ((last_state == 0)||(last_state == 1)) {
      motor_control(50,ccw);
      while(digitalRead(prox_center_switch) == 0){delay(1);}
      motor_control(0,ccw);
    }
    else if (last_state == 2) {
      motor_control(50,cw);
      while(digitalRead(prox_center_switch) == 0){delay(1);}
      motor_control(0,cw);
    }
    last_state = 3;

    float u1 = ultra(38, 40),
        u2 = ultra(26, 28),
        u3 = ultra(30, 32),
        u4 = ultra(34, 36);

    String output = "{'can' : " + String(PercenT(u1)) + 
                    ", 'pete' : " + String(PercenT(u2)) + 
                    ", 'other' : " + String(PercenT(u3)) +
                    ", 'plastic' : " + String(PercenT(u4)) + "}";

    Serial.println(output);
  }
}
