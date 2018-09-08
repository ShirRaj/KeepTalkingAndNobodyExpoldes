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

#define BASE_PLUS_PIN 15
#define BASE_MINUS_PIN 14
#define BASE_SCREW_PIN A0

#define SEL_BASE_PIN 6
#define READ_PIN 2

#define STATE_TNT_THREE_SAME 0
#define STATE_TNT_THREE_OR_FOUR_TWO_WHITE 1
#define STATE_TNT_THREE_OR_FOUR_TWO_YELLOW 2
#define STATE_TNT_FIVE_THREE_SAME 3
#define STATE_TNT_FIVE_TWO_RED 4
#define STATE_TNT_FIVE_TWO_GREEN 5
#define STATE_DYNAMITE_THREE_SAME 6
#define STATE_DYNAMITE_THREE_OR_FOUR_TWO_WHITE 7
#define STATE_DYNAMITE_THREE_OR_FOUR_TWO_YELLOW 8
#define STATE_DYNAMITE_FIVE_THREE_SAME 9
#define STATE_DYNAMITE_FIVE_TWO_RED 10
#define STATE_DYNAMITE_FIVE_TWO_GREEN 11
#define INSIDE_DATA_DELIMITER ":"
#define COLOR_ROW_DATA "ROW_DATA"
#define STATE_DATA "SETUP_STATE"


#define MULTIPLEXER_BITS 4

String MODULE_ID =  "wires";

String raw_msg;
String msg;
String data;

int cur_time = 0;

int color_row [NUM_SWITCHES];
int state;

int cur_trigger = 0;
int cur_penalty = 0;
int correct_plus_switches [] = { -1, -1, -1, -1, -1};
int correct_minus_switches [] = { -1, -1, -1, -1, -1};
int correct_wires []  = { -1, -1, -1, -1, -1};

int right_wires;
int wrong_wires;
int right_switches;
int wrong_switches;



void setup()
{
  prev_state = -1;
  cur_state = STATE_BOOT;
  for (int i = 0; i < NUM_SWITCHES; i ++) {
    pinMode(BASE_SCREW_PIN + i, INPUT_PULLUP);
  }

  for (int i = 0; i < MULTIPLEXER_BITS; i++) {
    pinMode(SEL_BASE_PIN - i, OUTPUT);
  }

  pinMode(READ_PIN, INPUT);

  Serial.begin(9600);
}

void loop()
{
  int enter_state = 0;
  if (cur_state != prev_state) {
    enter_state = 1;
    prev_state = cur_state;
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
      msg = raw_msg.substring(0, index_of_space);
      data = raw_msg.substring(index_of_space + 1, str_length - 1);
    }
    else
      msg = raw_msg.substring(0, str_length - 1);


    if (msg == "TIME") {
      cur_time = data.toInt();
    }
    else if (msg == "DATA") {
      process_data(data);
    }
    else if (msg == "PING") {
      send_msg(MSG_PING, "");
    }
  }

  switch (cur_state) {

    case STATE_BOOT:
      if(enter_state)
        boot_on_enter();
      send_msg(MSG_BOOT,MODULE_ID);
      if(msg=="INIT"){
        cur_state = STATE_INIT;
      }
      break;

    case STATE_INIT:
      if (enter_state)
        init_on_enter();
      if (check_if_ready()) {
        cur_state = STATE_READY;
      }
      break;

    case STATE_READY:
      if (enter_state)
        ready_on_enter();

      if (!check_if_ready()) {
        cur_state = STATE_INIT;
        send_msg(MSG_UNREADY, "");
      }

      if (msg == "START") {
        cur_state = STATE_START;
        send_msg(MSG_ACK, "");
      }
      break;

    case STATE_START:
      if (enter_state)
        start_on_enter();

      if (msg == "BOOM") {
        cur_state = STATE_BOOM;
        break;
      }

      process_logic();

      if (check_trigger()) {
        send_msg(MSG_TRIGGER, "");
      }

      if (check_penalty()) {
        send_msg(MSG_PENALTY, "");
      }

      if (check_solved()) {
        cur_state = STATE_SOLVED;
      }
      break;

    case STATE_BOOM:
      if (enter_state)
        boom_on_enter();
      break;

    case STATE_SOLVED:
      if (enter_state)
        solved_on_enter();
      break;

    default:
      break;
  }


}

//EDIT FROM HERE

void process_logic() {
  right_wires = 1;
  wrong_wires = 0;
  right_switches = 1;
  wrong_switches = 0;
  int wire_states [NUM_SWITCHES];
  for (int i = 0; i < NUM_SWITCHES; i++) {
    wire_states[i] = (digitalRead(BASE_SCREW_PIN + i) == LOW);
  }

  for (int i = 0; i < NUM_SWITCHES; i++) {
    if (correct_wires[i] >= 0 ) {

      if (wire_states[correct_wires[i]] == 1) {
        Serial.println("WIRE " + String(correct_wires[i]) + " IS STILL CONNECTED");
        right_wires = 0;
      }
      wire_states[correct_wires[i]] = 1;
    }
  }
  for (int i = 0; i < NUM_SWITCHES; i++) {
    if (color_row[i] != EMPTY && wire_states[i] == 0) {
      Serial.println("WIRE " + String(i) + " IS WRONGLY DISCONNECTED");
      wrong_wires = 1;
    }
  }
  int plus_switch_state [NUM_SWITCHES];
  int minus_switch_state [NUM_SWITCHES];
  for (int i = 0; i < NUM_SWITCHES; i++) {
    plus_switch_state[i] = (read_pin(BASE_PLUS_PIN - 2 * i) == LOW);
    minus_switch_state[i] = (read_pin(BASE_MINUS_PIN - 2 * i) == LOW);
    //    Serial.println("PIN " + String(i) + " PLUS " + String(plus_switch_state[i]) + " MINUS " + String(minus_switch_state[i]));
  }

  for (int i = 0; i < NUM_SWITCHES; i++) {
    if (correct_plus_switches[i] >= 0) {
      if (plus_switch_state[correct_plus_switches[i]] == 1) {
        right_switches = 0;
        Serial.println("PLUS SWITCH " + String(correct_plus_switches[i]) + " IS NOT SWITCHED");
      }
      plus_switch_state[correct_plus_switches[i]] = 1;
    }
    if (correct_minus_switches[i] >= 0) {
      if (minus_switch_state[correct_minus_switches[i]] == 1) {
        Serial.println("MINUS SWITCH " + String(correct_minus_switches[i]) + " IS NOT SWITCHED");
        right_switches = 0;
      }
      minus_switch_state[correct_minus_switches[i]] = 1;
    }
  }

  for (int i = 0; i < NUM_SWITCHES; i++) {
    if (plus_switch_state[i] == 0) {
      Serial.println("PLUS SWITCH " + String(i) + " IS WRONGLY SWITCHED");
      wrong_switches = 1;
    }
    if (minus_switch_state[i] == 0) {
      Serial.println("MINUS SWITCH " + String(i) + " IS WRONGLY SWITCHED");
      wrong_switches = 1;
    }
  }

}

boolean check_trigger() {
  //check if user triggered bomb
  if ((wrong_switches || wrong_wires)) {
    if (cur_trigger == 0) {
      cur_trigger = 1;
      return true;
    }
  }
  else
    cur_trigger = 0;
  return false;
}

boolean check_penalty() {
  //chek if user needs a penalty
  
  if (right_wires == 1 && right_switches == 0) {
    if(cur_penalty == 0) {
      cur_penalty = 1;
      return true;
    }
  }
  else
    cur_penalty = 0;
  return false;
}

boolean check_solved() {
  Serial.println("right wires " + String(right_wires) + " right switches " + String(right_switches));
  if (right_wires == 1 && right_switches == 1) {
    return true;
  }
  return false;
}

void send_msg(String msg, String data) {
  if (data != NULL) {
    Serial.println(msg + " " + data);
  }
  else {
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
    msg = data.substring(0, index_of_delim);
    in_data = data.substring(index_of_delim + 1, str_length);
  }
  else {
    Serial.println("ERROR: no delimiter found");
    return;
  }

  if (msg == COLOR_ROW_DATA) {
    for (int i = 0; i < NUM_SWITCHES; i ++) {
      switch (in_data.charAt(2 * i)) {
        case 'Y':
          color_row[i] = YELLOW;
          break;
        case 'W':
          color_row[i] = WHITE;
          break;
        case 'G':
          color_row[i] = GREEN;
          break;
        case 'R':
          color_row[i] = RED;
          break;
        case 'B':
          color_row[i] = BLACK;
          break;
        case 'E':
          Serial.println("Getting them empty at " + String(i));
          color_row[i] = EMPTY;
          break;
      }
    }
  }
  else if (msg == STATE_DATA) {
    Serial.println("Feeding STATE DATA");
    state = in_data.toInt();
    int left;
    int right;
    int middle;
    int counter;
    int red_counter;
    int green_counter;

    switch (state) {
      case STATE_TNT_THREE_SAME:
        left = NUM_SWITCHES + 1;
        right = -1;
        middle = -1;
        for (int i = 0; i < NUM_SWITCHES; i++) {
          if (color_row[i] != EMPTY) {
            if (left > i)
              left = i;
            else if (right < 0)
              right = i;
            else {
              middle = right;
              right = i;
            }
          }
        }
        correct_plus_switches[0] = left;
        correct_minus_switches[0] = right;
        correct_wires[0] = middle;
        break;
      case STATE_TNT_THREE_OR_FOUR_TWO_WHITE:
        left = NUM_SWITCHES + 1;
        right = -1;
        middle = -1;
        for (int i = 0; i < NUM_SWITCHES; i++) {
          if (color_row[i] == WHITE) {
            if (left > i)
              left = i;
            else if (right < i)
              right = i;
          }
        }
        correct_plus_switches[0] = right;
        correct_minus_switches[0] = left;
        counter = 0;
        for (int i = 0; i < NUM_SWITCHES; i++) {
          if (color_row[i] != EMPTY) {
            correct_wires[counter] = i;
            counter++;
          }
        }
        break;

      case STATE_TNT_THREE_OR_FOUR_TWO_YELLOW:
        left = NUM_SWITCHES + 1;
        right = -1;
        middle = -1;
        for (int i = 0; i < NUM_SWITCHES; i++) {
          if (color_row[i] == YELLOW) {
            if (left > i)
              left = i;
            else if (right < i)
              right = i;
          }
          if (color_row[i] != EMPTY && left < NUM_SWITCHES && right < 0) {
            middle = i;
          }
        }
        correct_plus_switches[0] = right;
        correct_plus_switches[1] = left;
        correct_minus_switches[0] = middle;

        correct_wires[0] = right;
        correct_wires[1] = left;
        break;

      case STATE_TNT_FIVE_THREE_SAME:
        correct_plus_switches [0] = 4;
        correct_minus_switches[0] = 0;
        for (int i = 0; i < NUM_SWITCHES; i++) {
          correct_wires[i] = i;
        }
        break;

      case STATE_TNT_FIVE_TWO_RED:
        red_counter = 0;
        counter = 0;
        for (int i = 0; i < NUM_SWITCHES; i ++) {
          if (color_row[i] == RED) {
            correct_plus_switches[red_counter] = i;
            red_counter = red_counter + 1;
          }
          else {
            correct_wires[counter] = i;
            counter = counter + 1;
          }
        }
        correct_minus_switches[0] = 4;
        break;

      case STATE_TNT_FIVE_TWO_GREEN:
        green_counter = 0;
        counter = 0;
        for (int i = 0; i < NUM_SWITCHES; i ++) {

          if (color_row[i] == GREEN) {
            correct_plus_switches[green_counter] = i;
            green_counter = green_counter + 1;
          }
          correct_wires[counter] = i;
          counter = counter + 1;
        }
        correct_minus_switches[0] = 0;
        break;




      case STATE_DYNAMITE_THREE_SAME:
        left = NUM_SWITCHES + 1;
        right = -1;
        middle = -1;
        for (int i = 0; i < NUM_SWITCHES; i++) {
          if (color_row[i] != EMPTY) {
            if (left > i)
              left = i;
            else if (right < 0)
              right = i;
            else {
              middle = right;
              right = i;
            }
          }
        }
        correct_plus_switches[0] = right;
        correct_minus_switches[0] = left;
        correct_wires[0] = middle;
        break;
      case STATE_DYNAMITE_THREE_OR_FOUR_TWO_WHITE:
        left = NUM_SWITCHES + 1;
        right = -1;
        middle = -1;
        for (int i = 0; i < NUM_SWITCHES; i++) {
          if (color_row[i] == WHITE) {
            if (left > i)
              left = i;
            else if (right < i)
              right = i;
          }
        }
        correct_plus_switches[0] = left;
        correct_minus_switches[0] = right;
        counter = 0;
        for (int i = 0; i < NUM_SWITCHES; i++) {
          if (color_row[i] != EMPTY) {
            correct_wires[counter] = i;
            counter++;
          }
        }
        break;

      case STATE_DYNAMITE_THREE_OR_FOUR_TWO_YELLOW:
        left = NUM_SWITCHES + 1;
        right = -1;
        middle = -1;
        for (int i = 0; i < NUM_SWITCHES; i++) {
          if (color_row[i] == YELLOW) {
            if (left > i)
              left = i;
            else if (right < i)
              right = i;
          }
          if (color_row[i] != EMPTY && left < NUM_SWITCHES && right < 0) {
            middle = i;
          }
        }
        correct_minus_switches[0] = right;
        correct_minus_switches[1] = left;
        correct_plus_switches[0] = middle;

        correct_wires[0] = right;
        correct_wires[1] = left;
        break;

      case STATE_DYNAMITE_FIVE_THREE_SAME:
        correct_minus_switches[0] = 4;
        correct_plus_switches[0] = 0;
        for (int i = 0; i < NUM_SWITCHES; i++) {
          correct_wires[i] = i;
        }
        break;

      case STATE_DYNAMITE_FIVE_TWO_RED:
        red_counter = 0;
        counter = 0;
        for (int i = 0; i < NUM_SWITCHES; i ++) {
          if (color_row[i] == RED) {
            correct_plus_switches[red_counter] = i;
            red_counter = red_counter + 1;
          }
          else {
            correct_wires[counter] = i;
            counter = counter + 1;
          }
        }
        correct_minus_switches[0] = 4;
        break;

      case STATE_DYNAMITE_FIVE_TWO_GREEN:
        green_counter = 0;
        counter = 0;
        for (int i = 0; i < NUM_SWITCHES; i ++) {

          if (color_row[i] == GREEN) {
            correct_minus_switches[green_counter] = i;
            red_counter = green_counter + 1;
          }
          correct_wires[counter] = i;
          counter = counter + 1;
        }
        correct_minus_switches[0] = 0;
        break;
    }

    for (int i = 0; i < NUM_SWITCHES; i++) {
      if (correct_wires[i] >= 0) {
        //color_row[correct_wires[i]] = EMPTY;
      }
    }
  }
  else {
    Serial.println("ERROR: wrong data message");
  }
}

boolean check_if_ready() {
  if (correct_plus_switches[0] == -1 && correct_minus_switches[0] == -1 && correct_wires[0] == -1) {
    Serial.println("No data fed");
    return false;
  }

  for (int i = 0; i < NUM_SWITCHES; i++) {
    if (color_row[i] != EMPTY) {
      if (digitalRead(BASE_SCREW_PIN + i) == HIGH) {
        Serial.println("SCREW " + String(i) + " not ready SHOULD BE LOW");
        return false;
      }
    }
    else {
      if (digitalRead(BASE_SCREW_PIN + i) == LOW) {
        Serial.println("SCREW " + String(i) + " not ready SHOULD BE HIGH");
        return false;
      }
    }
  }
  for (int i = 0; i < NUM_SWITCHES; i++) {
    if (read_pin(BASE_MINUS_PIN - 2 * i) == HIGH || read_pin(BASE_PLUS_PIN - 2 * i) == HIGH) {
      Serial.println("PLUS or MINUS SWITCH " + String(i) + " not ready");
      return false;
    }
  }

  return true;
}

int read_pin(int pin) {
  //LSB to MSB
  for (int i = 0; i < MULTIPLEXER_BITS; i++) {
    digitalWrite(SEL_BASE_PIN - i, pin % 2);
    pin = pin / 2;
  }
  return digitalRead(READ_PIN);
}

//FSM functions

void boot_on_enter() {

}

void init_on_enter() {
}

void ready_on_enter () {
  send_msg(MSG_READY, "");
}

void start_on_enter() {
}

void boom_on_enter() {
}

void solved_on_enter () {
  send_msg(MSG_SOLVED, "");
}
