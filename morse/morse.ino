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

//Define the LED Pin
#define PIN_OUT        6
#define PIN_RED        2
#define PIN_YELLOW        3
#define PIN_GREEN        4
#define PIN_BLUE        5
//Define unit length in ms
#define UNIT_LENGTH    250

String MODULE_ID =  "Morse";

String raw_msg;
String msg;
String data;

int cur_time = 0;


void setup()
{
  prev_state = -1;
  cur_state = STATE_BOOT;
  pinMode( PIN_OUT, OUTPUT );
  digitalWrite( PIN_OUT, LOW );

  randomSeed(analogRead(0));

   pinMode(PIN_RED, INPUT_PULLUP);
   pinMode(PIN_YELLOW, INPUT_PULLUP);
   pinMode(PIN_GREEN, INPUT_PULLUP);
   pinMode(PIN_BLUE, INPUT_PULLUP);

  
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
//Build a struct with the morse code mapping
static const struct {const char letter, *code;} MorseMap[] =
{
  { 'A', ".-" },
  { 'B', "-..." },
  { 'C', "-.-." },
  { 'D', "-.." },
  { 'E', "." },
  { 'F', "..-." },
  { 'G', "--." },
  { 'H', "...." },
  { 'I', ".." },
  { 'J', ".---" },
  { 'K', ".-.-" },
  { 'L', ".-.." },
  { 'M', "--" },
  { 'N', "-." },
  { 'O', "---" },
  { 'P', ".--." },
  { 'Q', "--.-" },
  { 'R', ".-." },
  { 'S', "..." },
  { 'T', "-" },
  { 'U', "..-" },
  { 'V', "...-" },
  { 'W', ".--" },
  { 'X', "-..-" },
  { 'Y', "-.--" },
  { 'Z', "--.." },
  { ' ', "     " }, //Gap between word, seven units 
    
  { '1', ".----" },
  { '2', "..---" },
  { '3', "...--" },
  { '4', "....-" },
  { '5', "....." },
  { '6', "-...." },
  { '7', "--..." },
  { '8', "---.." },
  { '9', "----." },
  { '0', "-----" },
};

static const struct {const char letter; bool state[4];} buttons_map[] = {
  {'A', {false, false, true, true}},
  {'B', {false, true, false, true}},
  {'C', {true, false, false, false}},
  {'D', {false, false, true, false}},
  {'E', {true, false, true, false}},
  {'F', {false, true, true, false}},
  {'G', {false, true, true, false}},
  {'H', {false, true, false, true}},
  {'I', {true, false, true, false}},
  {'J', {false, false, true, false}},
  {'K', {false, true, false, true}},
  {'L', {true, true, false, false}},
  {'M', {false, false, false, true}},
  {'N', {true, false, false, true}},
  {'O', {false, false, true, true}},
  {'P', {true, true, false, false}},
  {'Q', {false, true, true, false}},
  {'R', {true, false, false, true}},
  {'S', {true, false, true, false}},
  {'T', {true, false, false, true}},
  {'U', {false, true, false, false}},
  {'V', {true, false, false, false}},
  {'W', {false, false, true, true}},
  {'X', {true, true, false, false}},
  {'Y', {true, true, false, false}},
  {'Z', {false, false, true, true}},
  {'0', {true, false, false, true}},
  {'1', {false, true, true, false}},
  {'2', {false, false, false, true}},
  {'3', {true, false, false, false}},
  {'4', {false, true, false, false}},
  {'5', {false, true, false, true}},
  {'6', {false, false, true, false}},
  {'7', {false, true, false, false}},
  {'8', {true, false, true, false}},
  {'9', {false, false, false, true}}
};

char* str_index = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

long long current_time_checkpoint = 0;

String encode(const char *string)
{
  size_t i, j;
  String morseWord = "";
  
  for( i = 0; string[i]; ++i )
  {
    for( j = 0; j < sizeof MorseMap / sizeof *MorseMap; ++j )
    {
      if( toupper(string[i]) == MorseMap[j].letter )
      {
        morseWord += MorseMap[j].code;
        break;
      }
    }
    morseWord += " "; //Add tailing space to seperate the chars
  }

  return morseWord;  
}

String decode(String morse)
{
  String msg = "";
  
  int lastPos = 0;
  int pos = morse.indexOf(' ');
  while( lastPos <= morse.lastIndexOf(' ') )
  {    
    for( int i = 0; i < sizeof MorseMap / sizeof *MorseMap; ++i )
    {
      if( morse.substring(lastPos, pos) == MorseMap[i].code )
      {
        msg += MorseMap[i].letter;
      }
    }

    lastPos = pos+1;
    pos = morse.indexOf(' ', lastPos);
    
    // Handle white-spaces between words (7 spaces)
    while( morse[lastPos] == ' ' && morse[pos+1] == ' ' )
    {
      pos ++;
    }
  }

  return msg;
}

int i=0;
String morseWord;
char char_choosen_buf[3] = "\0\0"; /* gives {\0, \0, \0} */

int buttons[4] = {0,0,0,0};
bool buttons_answer[4] = {0,0,0,0};
bool triggered_flag = false;

void update_buttons_state(){
  buttons[0] = digitalRead(PIN_RED);
  buttons[1] = digitalRead(PIN_YELLOW);
  buttons[2] = digitalRead(PIN_GREEN);
  buttons[3] = digitalRead(PIN_BLUE);
}

void process_logic() {
  //process logic in START state

  update_buttons_state();
 
    if(triggered_flag && buttons[0] && buttons[1] && buttons[2] and buttons[3]) {
      triggered_flag = false;
    }
  if(i>=morseWord.length()) {
      i=0;
  }
  switch( morseWord[i] )
  {
    case '.': //dit
      if(millis() > current_time_checkpoint + UNIT_LENGTH && millis() <= current_time_checkpoint + 2*UNIT_LENGTH) {
        digitalWrite( PIN_OUT, HIGH );
      } else if(millis() > current_time_checkpoint + 2*UNIT_LENGTH){
        digitalWrite( PIN_OUT, LOW );
        current_time_checkpoint = millis();
        i++;
      }
      break;

    case '-': //dah
      if(millis() > current_time_checkpoint + UNIT_LENGTH && millis() <= current_time_checkpoint + 4*UNIT_LENGTH) {
        digitalWrite( PIN_OUT, HIGH );
      } else if(millis() > current_time_checkpoint + 4*UNIT_LENGTH){
        digitalWrite( PIN_OUT, LOW );
        current_time_checkpoint = millis();  
        i++;
      }  
      break;

    case ' ': //gap
    if(millis() < current_time_checkpoint + UNIT_LENGTH){
        digitalWrite( PIN_OUT, LOW );
      }else {
        current_time_checkpoint = millis();    
        i++;
      }
    
  }
}

boolean check_trigger() {
  //check if user triggered bomb
  if(triggered_flag) {
    //already sent trigger event
    return false;
  }
  bool state = (!buttons_answer[0] && !buttons[0]) ||
    (!buttons_answer[1] && !buttons[1]) ||
    (!buttons_answer[2] && !buttons[2]) ||
    (!buttons_answer[3] && !buttons[3]);
  if(state) {
    triggered_flag=true;
  }
  return state;
}

boolean check_penalty() {
  //chek if user needs a penalty
  return false;
}

boolean check_solved() {
  //check if user solved module
  return (
    !(buttons_answer[0] ^ !buttons[0]) &&
    !(buttons_answer[1] ^ !buttons[1]) &&
    !(buttons_answer[2] ^ !buttons[2]) &&
    !(buttons_answer[3] ^ !buttons[3])
  );
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
}

void ready_on_enter () {
  send_msg(MSG_READY,"");
  Serial.println("ENTER READY");
}

void start_on_enter() {
  Serial.println("ENTER START");
  int temp = random(0,  sizeof MorseMap / sizeof *MorseMap);
  char choosenLetter = MorseMap[temp].letter;

  char_choosen_buf[0] = choosenLetter;
  char_choosen_buf[1] =' ';

  morseWord = encode(char_choosen_buf);
  current_time_checkpoint = millis();
  
  
  char *e;
  int index;
  e = strchr(str_index, choosenLetter);
  index = (int)(e - str_index);

  for(int i=0;i<4;i++){
    buttons_answer[i] = buttons_map[index].state[i];
  }
}

void boom_on_enter() {
  Serial.println("ENTER BOOM");
}

void solved_on_enter () {
  send_msg(MSG_SOLVED,"");
  Serial.println("ENTER SOLVED");
}
