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

#define DATA_DELIMITER " "

String MODULE_ID =  "3_Blinking_Buttons";

String raw_msg;
String msg;
String data;

int cur_time = 0;


void setup()
{
  prev_state = -1;
  cur_state = STATE_BOOT;
  Serial.begin(9600);

   pinMode(2, OUTPUT);
   pinMode(3, OUTPUT);
   pinMode(4, OUTPUT);
   pinMode(5, OUTPUT);
   pinMode(6, OUTPUT);
   pinMode(7, OUTPUT);
   pinMode(8, OUTPUT);
   pinMode(9, OUTPUT);
   pinMode(10, OUTPUT);
   pinMode(11, OUTPUT);
  
   pinMode(A1, INPUT_PULLUP);
   pinMode(A2, INPUT_PULLUP);
   pinMode(A3, INPUT_PULLUP);


   randomSeed(analogRead(0));
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
      send_msg(MSG_BOOT,MODULE_ID);
      if(msg=="INIT"){
        cur_state = STATE_INIT;
      }
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

int buttons_solved = 0;
int current_7seg = 8;
int current_buttons = 0;
long long current_time_checkpoint_buttons = 0;
long long current_time_checkpoint_7seg = 0;
const int BUTTON_1_PIN = A2;
const int BUTTON_2_PIN = A3;
const int BUTTON_3_PIN = A4;
bool flags[3] = {0,0,0};

/*
int num_array[15][7] = {  { 1,1,1,1,1,1,0 },    // 0
                          { 0,1,1,0,0,0,0 },    // 1
                          { 1,1,0,1,1,0,1 },    // 2
                          { 1,1,1,1,0,0,1 },    // 3
                          { 0,1,1,0,0,1,1 },    // 4
                          { 1,0,1,1,0,1,1 },    // 5
                          { 1,0,1,1,1,1,1 },    // 6
                          { 1,1,1,0,0,0,0 },    // 7
                          { 1,1,1,1,1,1,1 },    // 8
                          { 1,1,1,0,0,1,1 },   // 9
                          { 1,1,1,0,1,1,1 },   // 10 (A)
                          { 1,1,1,1,1,1,1 },   // 11 (B)
                          { 1,0,0,1,1,1,0 },   // 12 (C)
                          { 1,0,0,1,1,1,1 },  // 13 (E)
                          { 0,0,0,0,0,0,0 }}; // 14 None
*/
int num_array[8][7] = {  
                          { 0,1,1,0,0,0,0 },    // 0 (1)
                          { 1,1,0,1,1,0,1 },    // 1 (2)
                          { 1,1,1,1,0,0,1 },    // 2 (3)
                          { 1,0,1,1,1,1,1 },    // 3 (6)
                          { 1,1,1,0,1,1,1 },   // 4 (A)
                          { 1,0,0,1,1,1,0 },   // 5 (C)
                          { 1,0,0,1,1,1,1 },  // 6 (E)
                          { 0,0,0,0,0,0,0 }}; // 7 None

bool buttons_check_table[7][4][3] = {
                          // buttons_check_table[7seg][time_phase][button_pos]
                          {{1,0,0}, {0,1,0}, {1,1,1}, {1,0,0}},
                          {{1,0,1}, {0,0,0}, {0,0,0}, {0,0,1}},
                          {{1,1,0}, {0,0,1}, {0,1,1}, {0,1,0}},
                          {{0,1,0}, {1,1,0}, {1,0,0}, {0,0,1}},
                          {{0,0,0}, {1,0,1}, {0,0,1}, {0,0,0}},
                          {{0,1,1}, {1,1,1}, {0,1,0}, {1,0,1}},
                          {{1,1,1}, {1,0,0}, {1,1,0}, {0,1,1}}
};
  



                          

void num_write(int number) 
{
  int pin= 2;
  for (int j=0; j < 7; j++) {
   digitalWrite(pin, num_array[number][j]);
   pin++;
  }
}


int cur_time_to_time_phase(int time_left) {
  if(time_left > 240) {
    return 0;  
  } else if(time_left <= 240 && time_left > 120) {
    return 1;  
  } else if(time_left <= 120 && time_left>60) {
    return 2;  
  } else {
    return 3;
  }
}

void process_logic() {
  //process logic in START state
  if(millis() > current_time_checkpoint_7seg + 15000) {
    change_7seg_state();
    current_time_checkpoint_7seg = millis();
  }
  if(millis() > current_time_checkpoint_buttons + 3000) {
    change_buttons_state();
    current_time_checkpoint_buttons = millis();
  }


}

boolean check_trigger() {
  //check if user triggered bomb
  int buttons[3] = { digitalRead(BUTTON_1_PIN),  digitalRead(BUTTON_2_PIN),  digitalRead(BUTTON_3_PIN)};
  int i = buttons_solved;
  if(!buttons[i] && !flags[i]) {
    flags[i]=true;
    Serial.println(i);
    int button_should_be = buttons_check_table[current_7seg][cur_time_to_time_phase(cur_time)][i];
    if(!(button_should_be ^ bitRead(current_buttons, i))) {
      // solve button and move next
      buttons_solved++;
      Serial.println("Solved!");
      return false;
    }else{
      // pushed current button in order but shouldn't
      return true;
    }
  } else if (buttons[i]) {
    flags[i] = false;
  }

  // check for trigger
  
  for(int j=i+1;j<3;j++) {
    // check for buttons pushed not in order
    if(!buttons[j]  && !flags[j]){
      Serial.println("boom");
      flags[j]=true;
      return true;
    } else if (buttons[j]) {
       flags[j]=false;
    }
  }
  return false;
}

boolean check_penalty() {
  //chek if user needs a penalty
  return false;
}

boolean check_solved() {
  //check if user solved module
  return buttons_solved >= 3;
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

}

void init_on_enter() {
  Serial.println("ENTER INIT");
  buttons_solved = 0;
}

void ready_on_enter () {
  send_msg(MSG_READY,"");
  Serial.println("ENTER READY");
  num_write(7);
  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
  digitalWrite(11, HIGH);
}

void start_on_enter() {
  Serial.println("ENTER START");
  num_write(7);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  start_up_random();
  change_buttons_state();
  change_7seg_state();
  current_time_checkpoint_buttons = millis();
  current_time_checkpoint_7seg = millis();
}

void start_up_random(){
 // funny stuff
   bool flag = true;
  for(int i=0;i<10;i++){
    if(flag){
      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
      digitalWrite(11, LOW);
      flag=false;
    }else{
      digitalWrite(9, HIGH);
      digitalWrite(10, HIGH);
      digitalWrite(11, HIGH);
      flag=true;
    }
     num_write(random(0, 8));
    delay(50);
  }
}

void change_buttons_state() {
  current_buttons = random(1,8);
  for (byte i=0; i<3; i++) {
    byte state = bitRead(current_buttons, i);
    digitalWrite(9+i, state);
  }

  if(buttons_solved >= 1 ) {
    digitalWrite(9, HIGH);
  }
  if(buttons_solved >= 2 ) {
    digitalWrite(10, HIGH);
  }
}

void change_7seg_state(){
  current_7seg = random(0,7);
  num_write(current_7seg);
  Serial.print("current_7seg:");
  Serial.println(current_7seg);
}

void boom_on_enter() {
  Serial.println("ENTER BOOM");
}

void solved_on_enter () {
  send_msg(MSG_SOLVED,"");
  Serial.println("ENTER SOLVED");
}
