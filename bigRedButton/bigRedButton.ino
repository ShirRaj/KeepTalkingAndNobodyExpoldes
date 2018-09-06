//DON'T TOUCH UNTIL EDIT FROM HERE COMMENT
int cur_state;
int prev_state;

#define STATE_BOOT 0
#define STATE_INIT 1
#define STATE_READY 2
#define STATE_START 3
#define STATE_BOOM 4
#define STATE_SOLVED 5

#define MSG_BOOT "BOOT"
#define MSG_READY "READY"
#define MSG_UNREADY "UNREADY"
#define MSG_ACK "ACK"
#define MSG_TRIGGER "TRIGGER"
#define MSG_PENALTY "PENALTY"
#define MSG_SOLVED "SOLVED"
#define MSG_PING "PONG"

#define LED_PIN 4
#define BUTTON_PIN 6

#define INSIDE_DATA_DELIMITER ":"
#define DATA_MSG "ALL_SOLVED"
#define DATA_DELIMITER " "

String MODULE_ID =  "bigRedButton";

String raw_msg;
String msg;
String data;

int cur_time = 0;

int all_solved = 0;
int press_started = -1;
boolean pressed = false;
boolean press_change = false;

void setup()
{
  prev_state = -1;
  cur_state = STATE_BOOT;
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  pinMode(LED_PIN,OUTPUT);

  Serial.begin(9600);
}

void loop()
{
  int enter_state = 0;
  if (cur_state != prev_state) {
    enter_state = 1;
    prev_state=cur_state;
  }
  raw_msg = "";
  msg = "";
  data = "";
    // if we get a valid byte, read analog ins:
  if (Serial.available() > 0) {
      raw_msg = Serial.readString();
      int index_of_space = raw_msg.indexOf(" ");
      int str_length = raw_msg.length();
      
      if (index_of_space > 0) {
        msg = raw_msg.substring(0,index_of_space);
        data =raw_msg.substring(index_of_space+1,str_length-1);
      }
      else 
        msg = raw_msg.substring(0,str_length-1);
      
      if(msg=="TIME") {
       cur_time = data.toInt();
      }
      else if(msg=="DATA"){
        process_data(data);
      }
      else if(msg=="PING"){
        send_msg(MSG_PING,"");
      }
  }
  
  switch(cur_state) {
    
    case STATE_BOOT:
      if(enter_state)
        boot_on_enter();
      break;

    case STATE_INIT:
      if(enter_state)
        init_on_enter();
      if(check_if_ready()) {
        cur_state=STATE_READY;
      }
      break;

    case STATE_READY:
      if(enter_state)
        ready_on_enter();
        
      if(!check_if_ready()) {
        cur_state=STATE_INIT;
        send_msg(MSG_UNREADY,"");
      }
      
      if(msg=="START"){
        cur_state = STATE_START;
        send_msg(MSG_ACK,"");
      }
      break;

    case STATE_START:
      if(enter_state)
        start_on_enter();

      if(msg=="BOOM"){
        cur_state = STATE_BOOM;
        break;
      }
      
      process_logic();
      
      if(check_trigger()){
        send_msg(MSG_TRIGGER,"");
      }

      if(check_penalty()) {
        send_msg(MSG_PENALTY,"");
      }

      if(check_solved()) { 
        cur_state = STATE_SOLVED;
      }
      break;

    case STATE_BOOM:
      if(enter_state)
        boom_on_enter();
      break;

    case STATE_SOLVED:
      if(enter_state)
        solved_on_enter();
      break;
      
    default:
      break;
  }


}

//EDIT FROM HERE

void process_logic() {
  //process logic in START state

  if(digitalRead(BUTTON_PIN) == LOW) {
    delay(100);
    if(press_started<0 && pressed==false){
      press_started = cur_time;
      pressed = true;
    }
    digitalWrite(LED_PIN,LOW);
  }
  else {
    if(pressed)
      press_change = true;
    else
      press_change = false;
    pressed = false;
    if(cur_time%2 == 0){
      digitalWrite(LED_PIN,HIGH);
    }
    else {
      digitalWrite(LED_PIN,LOW);
    }  
  }
}

boolean check_trigger() {
  //check if user triggered bomb
  if (all_solved == 0 && press_started >= 0 )
  {
    press_started = -1;
    //allow button press to equalize
    return true;
    
  }
  else if (all_solved == 1 &&  press_started-cur_time < 3 && press_change == true ) {
    press_started = -1;
    return true;
  }
  return false;
}

boolean check_penalty() {
  //chek if user needs a penalty
  return false;
}

boolean check_solved() {
  //check if user solved module

  if(all_solved == 1 && press_started-cur_time >= 3 && press_started >= 0) {
    press_started = -1;
    return true;
  }
  return false;
}

void send_msg(String msg, String data) {
  if(data!=NULL){
    Serial.println(msg+" "+data);
  }
  else{
    Serial.println(msg);
  }
}

void process_data(String data) {
  //Process DATA from rpi
  int index_of_delim = data.indexOf(INSIDE_DATA_DELIMITER);
  int str_length = data.length();
  String in_data;
  String msg;
  
  if (index_of_delim > 0) {
    msg = data.substring(0,index_of_delim);
    in_data =data.substring(index_of_delim+1,str_length);
  }
  else {
    Serial.println("ERROR: no delimiter found");
  }

  if(msg==DATA_MSG) {
    all_solved = in_data.toInt();
  }
  else {
    Serial.println("ERROR: wrong data message");
  }
}

boolean check_if_ready() {
  //check if module is ready for start
  return true;
}

//FSM functions

void boot_on_enter() {
  send_msg(MSG_BOOT,MODULE_ID);
  cur_state = STATE_INIT;
}

void init_on_enter() {
}

void ready_on_enter () {
  send_msg(MSG_READY,"");
}

void start_on_enter() {
}

void boom_on_enter() {
}

void solved_on_enter () {
  send_msg(MSG_SOLVED,"");
}
