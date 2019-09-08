#include "HID-Project.h"
#include <Wire.h>

#define PSOC_I2C_SLAVE_ADDRESS 0x08
#define ARDUINO_I2C_SLAVE_ADDRESS 0x07
#define BUFFER_SIZE 5

#define CIRCLE 3
#define CROSS 2
#define SQUARE 1
#define TRIANGLE 4

#define LL 19
#define LR 20
#define RL 21
#define RR 22

#define LU 15
#define LD 26

#define L1 5
#define R1 6

#define L2 7
#define R2 8

#define L3 11
#define R3 12

#define SHARE 9
#define OPTION 10

#define TOUCH 14
#define PS 13

#define CIRCLE_PIN 4
#define CROSS_PIN 5
#define SQUARE_PIN 6
#define TRIANGLE_PIN 7
#define START_PIN 15

#define SERIAL_DEBUG_PIN 10

#define BUTTON_NUM 5

#define MIN16 -32768
#define MAX16 32767
#define MIN8 -128
#define MAX8 127

const unsigned char axis_serial_table[8] = {
  LU, LD, L2, R2, LL, LR, RL, RR
};

const unsigned char button_direct_table[8] = {
  TRIANGLE, SQUARE, CROSS, CIRCLE, OPTION, 0, 0, 0
};

const unsigned char button_direct_logic = 0b00000000;

const int button_direct_pin_table[BUTTON_NUM] = {
  TRIANGLE_PIN, SQUARE_PIN, CROSS_PIN, CIRCLE_PIN, START_PIN
};

unsigned char button_data_byte = 0;

int data_bytes_count = 0;
//int data_bytes_count2 = 0;
int flagL = 0;
int flagR = 0;
unsigned char serial_data_byte[BUFFER_SIZE] = {};

unsigned char readDirectlyConnectedButtons(int *pin_table, unsigned char pin_logic);
void addHIDaxisReportFromTable(unsigned char serial_data_byte, unsigned char *button_table, int contents_of_table_num);
void addHIDreportFromTable(unsigned char serial_data_byte, unsigned char *button_table, int contents_of_table_num);
void addHIDCypressLRReportFromTable(unsigned char serial_data_byte, unsigned char serial_data_byte_left, unsigned char serial_data_byte_right);
void sendRecievedI2CDataWithUART(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size);

void setup(void) {
  for(int i = 0; i < BUTTON_NUM; i++) {
    pinMode(button_direct_pin_table[i], INPUT_PULLUP);
  }
  //pinMode(SERIAL_DEBUG_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000L);
  Gamepad.begin();
}

void loop(void) {
    data_bytes_count = 0;
    //data_bytes_count2 = 0;
    for(int i = 0; i < BUFFER_SIZE; i++) {
      serial_data_byte[i] = 0;
    }
    Wire.requestFrom(PSOC_I2C_SLAVE_ADDRESS, BUFFER_SIZE);
    button_data_byte = readDirectlyConnectedButtons(button_direct_pin_table, button_direct_logic);
    while(Wire.available() && data_bytes_count < BUFFER_SIZE) {
      serial_data_byte[data_bytes_count] = Wire.read();
      data_bytes_count++;
    }
    
    /*while(data_bytes_count2 < BUFFER_SIZE) {
      Wire.beginTransmission(ARDUINO_I2C_SLAVE_ADDRESS);
      Wire.write(serial_data_byte[data_bytes_count2]);
      data_bytes_count2++;
      Wire.endTransmission();
    }*/
    /*if(!digitalRead(SERIAL_DEBUG_PIN)) {
      sendRecievedI2CDataWithUART(serial_data_byte, BUFFER_SIZE);   
    }*/
    addHIDCypressLRReportFromTable(serial_data_byte[0], serial_data_byte[1],serial_data_byte[4]);
    addHIDaxisReportFromTable(serial_data_byte[0], axis_serial_table, 8);
    addHIDreportFromTable(button_data_byte, button_direct_table, BUTTON_NUM);
    Gamepad.write();
}

unsigned char readDirectlyConnectedButtons(int *pin_table, unsigned char pin_logic) {
  unsigned char result = 0;
  for(int i = 0; i < BUTTON_NUM; i++) {
    result |= (digitalRead(pin_table[i]) << (7 - i));
  }
  return result ^ pin_logic;
}

void addHIDCypressLRReportFromTable(unsigned char serial_data_byte, unsigned char serial_data_byte_left, unsigned char serial_data_byte_right) {
  if(serial_data_byte_left & 0b11100000) { //проверка на нажатие на первые три пикселя сенсорной панели
    if (serial_data_byte == 0b00000000){  //проверка, есть ли движение руки при этом
    flagL++;
    if(flagL == 100){ //через сколько циктов выполнения вышеописанных условий выполнится условие (можно подобрать наиболее подходящие настройки изменив число)
    Gamepad.press(L1);
    flagL = 0;
  }
  }
    else{
    Gamepad.release(L1);
    flagL = 0;
  }
  }
  else{
    Gamepad.release(L1);
    flagL = 0;
  }
  if(serial_data_byte_right & 0b00011100) {
    if (serial_data_byte == 0b00000000){
    flagR++;
    if(flagR == 100){
    Gamepad.press(R1);
    flagR = 0;
  }
  }
    else{
    Gamepad.release(R1);
    flagR = 0;
  }
  }
  else{
    Gamepad.release(R1);
    flagR = 0;
  }
}

void addHIDaxisReportFromTable(unsigned char serial_data_byte, unsigned char *button_table, int contents_of_table_num) {
  Gamepad.xAxis(0);
  Gamepad.yAxis(0);
  Gamepad.zAxis(0);
  Gamepad.rxAxis(0);
  Gamepad.ryAxis(0);
  Gamepad.rzAxis(0);
  if((serial_data_byte >> 7) & 0x01) {
    Gamepad.yAxis(MIN16);
  }
  if((serial_data_byte >> 6) & 0x01) {
    Gamepad.yAxis(MAX16);
  }
  if((serial_data_byte >> 5) & 0x01) {
    Gamepad.zAxis(MIN8);
  }
  if((serial_data_byte >> 4) & 0x01) {
    Gamepad.zAxis(MIN8);
  }
  if((serial_data_byte >> 3) & 0x01) {
    Gamepad.xAxis(MIN16);
  }
  if((serial_data_byte >> 2) & 0x01) {
    Gamepad.xAxis(MAX16);
  }
  if((serial_data_byte >> 1) & 0x01) {
    Gamepad.rxAxis(MIN16);
  }
  if((serial_data_byte >> 0) & 0x01) {
    Gamepad.rxAxis(MAX16);
  }
}

void addHIDreportFromTable(unsigned char serial_data_byte, unsigned char *button_table, int contents_of_table_num) {
  for(int i = 0; i < contents_of_table_num; i++) {
    if((serial_data_byte >> (7 - i)) & 0x01) {
      Gamepad.release(button_table[i]);
      } else {
        Gamepad.press(button_table[i]);
        }
  }
}

void sendRecievedI2CDataWithUART(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size) {
  int len = BUFFER_SIZE * 9 + 2;
  char c;
  char send_data[len];
  for(int i = 0; i < buffer_size; i++) {
    for(int j = 0; j < 9; j++) {
      if(j < 8) {
        if((serial_data_byte[i] >> (7 - j)) & 0x01) {
          c = '@';
        } else {
          c = '_';
        }
      } else {
        c = ' ';
      }
      send_data[i * 9 + j] = c;
    }
  }
  send_data[len - 2] = '\r';
  send_data[len - 1] = '\n';
  Serial.write(send_data, len);
}
