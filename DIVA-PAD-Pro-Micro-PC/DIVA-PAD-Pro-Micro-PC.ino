#include <HID-Project.h>
#include "Adafruit_NeoPixel.h"
#include <Wire.h>

#define PSOC_I2C_SLAVE_ADDRESS 0x08
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

#define CIRCLE_PIN 6
#define CROSS_PIN 5
#define SQUARE_PIN 4
#define TRIANGLE_PIN 7
#define L_PIN 8
#define R_PIN 9
#define START_PIN 15

#define CIRCLE_LED_PIN 21
#define CROSS_LED_PIN 20
#define SQUARE_LED_PIN 19
#define TRIANGLE_LED_PIN 18

#define LED_STRIP_PIN 0
#define LED_NUM 4
#define LED_STRIP_NUM 30

#define BUTTON_NUM 7

#define MIN16 -32768
#define MAX16 32767
#define MIN8 -128
#define MAX8 127

const unsigned char axis_serial_table[8] = {
  LU, LD, L2, R2, LL, LR, RL, RR
};

const unsigned char button_direct_table[8] = {
  TRIANGLE, SQUARE, CROSS, CIRCLE, OPTION, L2, R2, 0
};

const unsigned char button_direct_logic = 0b11111110;

const int button_direct_pin_table[BUTTON_NUM] = {
  TRIANGLE_PIN, SQUARE_PIN, CROSS_PIN, CIRCLE_PIN, START_PIN, L_PIN, R_PIN
};

const int led_direct_pin_table[LED_NUM] = {
  TRIANGLE_LED_PIN, SQUARE_LED_PIN, CROSS_LED_PIN, CIRCLE_LED_PIN
};

unsigned char button_data_byte = 0;

int data_bytes_count = 0;

int flagL = 0;
int flagR = 0;

unsigned char serial_data_byte[BUFFER_SIZE] = {};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_STRIP_NUM, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

void selfTestLEDs(int *led_table);
unsigned char readDirectlyConnectedButtons(int *pin_table, unsigned char pin_logic);
void writeDirectlyConnectedLEDs(int *led_table, unsigned char led_data_byte);
void addHIDaxisReportFromTable(unsigned char serial_data_byte, unsigned char *button_table, int contents_of_table_num);
void addHIDCypressLRReportFromTable(unsigned char serial_data_byte, unsigned char serial_data_byte_left, unsigned char serial_data_byte_right);
void addHIDreportFromTable(unsigned char serial_data_byte, unsigned char *button_table, int contents_of_table_num);
void sendRecievedI2CDataWithUART(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size);

void setup(void) {
  for(int i = 0; i < BUTTON_NUM; i++) {
    pinMode(button_direct_pin_table[i], INPUT_PULLUP);
  }
  for(int i = 0; i < LED_NUM; i++) {
    pinMode(led_direct_pin_table[i], OUTPUT);
  }
  //Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000L);
  Gamepad.begin();
  strip.begin();
  strip.setBrightness(255);
  strip.show();
  selfTestLEDs(led_direct_pin_table);
}

void loop(void) {
    data_bytes_count = 0;
    for(int i = 0; i < BUFFER_SIZE; i++) {
      serial_data_byte[i] = 0;
    }
    Wire.requestFrom(PSOC_I2C_SLAVE_ADDRESS, BUFFER_SIZE);
    button_data_byte = readDirectlyConnectedButtons(button_direct_pin_table, button_direct_logic);
    writeDirectlyConnectedLEDs(led_direct_pin_table, button_data_byte);
    while(Wire.available() && data_bytes_count < BUFFER_SIZE) {
      serial_data_byte[data_bytes_count] = Wire.read();
      data_bytes_count++;
    }
    //sendRecievedI2CDataWithUART(serial_data_byte, BUFFER_SIZE);
    addHIDaxisReportFromTable(serial_data_byte[0], axis_serial_table, 8);
    addHIDreportFromTable(button_data_byte, button_direct_table, BUTTON_NUM);
        int touched = 0;
    for(int i = 0; i < BUFFER_SIZE - 1; i++) {
      for(int j = 0; j < 8; j++) {
        if((serial_data_byte[i + 1] >> (7 - j)) & 0x01) {
          strip.setPixelColor(i * 8 + j, 0xFF, 0xFF, 0xFF);
          touched = 1;
        } else {
          strip.setPixelColor(i * 8 + j, 1, 1, 1);
        }
      }
    }
    if(touched == 0) {
      for(int i = 0; i < LED_STRIP_NUM; i++) {
        strip.setPixelColor(i, 3, 5, 5);
      }
    }
    strip.show();
    Gamepad.write();
}

void selfTestLEDs(int *led_table) {
  unsigned char data_byte;
  for(int i = 0; i < LED_NUM + 1; i++) {
    data_byte = 0x01 << (7 - i);
    writeDirectlyConnectedLEDs(led_direct_pin_table, data_byte);
    delay(200);
  }
  for(int i = 0; i < 3; i++){
    for(int j = 0; j < LED_NUM; j++){
    digitalWrite(led_table[j], LOW);
    }
    delay(200);
    for(int j = 0; j < LED_NUM; j++)
    digitalWrite(led_table[j], HIGH);
    delay(200);
  }
  for(int i = 0; i < LED_STRIP_NUM; i++) {
    for(int j = 0; j < LED_STRIP_NUM; j++) {
      strip.setPixelColor(j, 0);
    }
    strip.setPixelColor(i, 0xFF, 0xFF, 0xFF);
    strip.show();
    delay(50);
  }
  for(int i = LED_STRIP_NUM; i > 0; i--) {
    strip.setPixelColor(i, 0xFF, 0xFF, 0xFF);
    strip.show();
    delay(50);
  }
  for(int i = 0; i < LED_STRIP_NUM; i++) {
    strip.setPixelColor(i, 3, 5, 5);
    strip.show();
    delay(50);
  }
}

unsigned char readDirectlyConnectedButtons(int *pin_table, unsigned char pin_logic) {
  unsigned char result = 0;
  for(int i = 0; i < BUTTON_NUM; i++) {
    result |= (digitalRead(pin_table[i]) << (7 - i));
  }
  return result ^ pin_logic;
}

void writeDirectlyConnectedLEDs(int *led_table, unsigned char led_data_byte) {
  for(int i = 0; i < LED_NUM; i++) {
    digitalWrite(led_table[i], !((led_data_byte >> (7 - i)) & 0x01));
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
      Gamepad.press(button_table[i]);
    } else {
      Gamepad.release(button_table[i]);
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
