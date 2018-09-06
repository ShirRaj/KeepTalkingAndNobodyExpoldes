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
#define MSG_PING "PING"

#define EMPTY 0
#define YELLOW 1
#define GREEN 2
#define WHITE 3
#define BLACK 4
#define RED 5

#define NUM_SWITCHES 5

#define DATA_DELIMITER " "

#define BASE_PLUS_PIN 10
#define BASE_MINUS_PIN 0
#define BASE_SCREW_PIN 5

String MODULE_ID =  "Test_Module";

String raw_msg;
String msg;
String data;

int cur_time = 0;

int color_plus [NUM_SWITCHES];
int color_minus [NUM_SWITCHES];


void setup()
{
  prev_state = -1;
  cur_state = STATE_BOOT;
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
      
      Serial.println("MSG: "+msg);
      Serial.println("DATA: "+data);
      
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
}

boolean check_trigger() {
  //check if user triggered bomb
  return false;
}

boolean check_penalty() {
  //chek if user needs a penalty
  return false;
}

boolean check_solved() {
  //check if user solved module
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
  Serial.println(data);
}

boolean check_if_ready() {
  //check if module is ready for start
  return true;
}

//FSM functions

void boot_on_enter() {
  Serial.println("ENTER BOOT");
  send_msg(MSG_BOOT,MODULE_ID);
  cur_state = STATE_INIT;
}

void init_on_enter() {
}

void ready_on_enter () {
  send_msg(MSG_READY,"");
}

void start_on_enter() {
  Serial.println("ENTER START");
}

void boom_on_enter() {
  Serial.println("ENTER BOOM");
}

void solved_on_enter () {
  send_msg(MSG_SOLVED,"");
  Serial.println("ENTER SOLVED");
}
