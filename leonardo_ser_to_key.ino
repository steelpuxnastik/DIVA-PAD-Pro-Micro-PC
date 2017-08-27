#include "HID-Project.h"
#include <Wire.h>

#define PSOC_I2C_SLAVE_ADDRESS 0x08
#define BUFFER_SIZE 5

#define CIRCLE KEY_K
#define CROSS KEY_J
#define SQUARE KEY_F
#define TRIANGLE KEY_D

#define LL KEY_E
#define LR KEY_R
#define RL KEY_U
#define RR KEY_I

#define LU KEY_UP_ARROW
#define LD KEY_DOWN_ARROW

#define UP KEY_UP_ARROW
#define DOWN KEY_DOWN_ARROW
#define LEFT KEY_LEFT_ARROW
#define RIGHT KEY_RIGHT_ARROW

#define L1 KEY_3
#define L2 KEY_4
#define R1 KEY_8
#define R2 KEY_7

#define L3 KEY_5
#define R3 KEY_6

#define SHARE KEY_G
#define OPTION KEY_H

#define TOUCH KEY_T
#define PS KEY_Y

#define CIRCLE_PIN 4
#define CROSS_PIN 5
#define SQUARE_PIN 6
#define TRIANGLE_PIN 7

#define TIMING_CHECK_PIN 8
#define ENABLE_PIN 9

#define BUTTON_NUM 4

const KeyboardKeycode button_serial_table[8] = {
	LU, LD, L2, R2, LL, LR, RL, RR
};

const KeyboardKeycode button_direct_table[8] = {
	TRIANGLE, SQUARE, CROSS, CIRCLE, 0, 0, 0, 0
};

const int button_direct_pin_table[BUTTON_NUM] = {
	TRIANGLE_PIN, SQUARE_PIN, CROSS_PIN, CIRCLE_PIN
};

unsigned char button_data_byte = 0;

int data_bytes_count = 0;
unsigned char serial_data_byte[BUFFER_SIZE] = {};
char c;

unsigned char readDirectlyConnectedButtons(int *pin_table);
void addHIDreportFromTable(unsigned char serial_data_byte, KeyboardKeycode *button_table, int contents_of_table_num);
void sendRecievedI2CDataWithUART(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size);

void setup(void) {
	for(int i = 0; i < BUTTON_NUM; i++) {
		pinMode(button_direct_pin_table[i], INPUT_PULLUP);
	}
	pinMode(ENABLE_PIN, INPUT_PULLUP);
	pinMode(TIMING_CHECK_PIN, OUTPUT);
	Serial.begin(115200);
	Wire.begin(); //このボードをI2Cマスターとして設定
	NKROKeyboard.begin();
}

void loop(void) {
	digitalWrite(TIMING_CHECK_PIN, 1);
	if(!digitalRead(ENABLE_PIN)) {
		data_bytes_count = 0;
		for(int i = 0; i < BUFFER_SIZE; i++) {
			serial_data_byte[i] = 0;
		}
		Wire.requestFrom(PSOC_I2C_SLAVE_ADDRESS, BUFFER_SIZE);
		button_data_byte = readDirectlyConnectedButtons(button_direct_pin_table);
		while(Wire.available() && data_bytes_count < BUFFER_SIZE) { // シリアルで何らかの信号を受け取ったとき...
			serial_data_byte[data_bytes_count] = Wire.read();
			data_bytes_count++;
		}
		sendRecievedI2CDataWithUART(serial_data_byte, BUFFER_SIZE);
		digitalWrite(TIMING_CHECK_PIN, 0);
		addHIDreportFromTable(serial_data_byte[0], button_serial_table, 8);
		addHIDreportFromTable(button_data_byte, button_direct_table, BUTTON_NUM);
	} else {
		NKROKeyboard.releaseAll();
	}
	NKROKeyboard.send();
}

unsigned char readDirectlyConnectedButtons(int *pin_table) {
	unsigned char result = 0;
	for(int i = 0; i < BUTTON_NUM; i++) {
		result |= ((!digitalRead(pin_table[i])) << (7 - i));
	}
	return result;
}

void addHIDreportFromTable(unsigned char serial_data_byte, KeyboardKeycode *button_table, int contents_of_table_num) {
	for(int i = 0; i < contents_of_table_num; i++) {
		if((serial_data_byte >> (7 - i)) & 0x01) {
			NKROKeyboard.add(button_table[i]);
		} else {
			NKROKeyboard.remove(button_table[i]);
		}
	}
}

void sendRecievedI2CDataWithUART(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size) {
	for(int i = 0; i < buffer_size; i++) {
		for(int j = 0; j < 8; j++) {
			if((serial_data_byte[i] >> (7 - j)) & 0x01) {
				c = '@';
			} else {
				c = '_';
			}
			Serial.write(c);
		}
		Serial.write(' ');
	}
	Serial.write('\r');
	Serial.write('\n');
}
