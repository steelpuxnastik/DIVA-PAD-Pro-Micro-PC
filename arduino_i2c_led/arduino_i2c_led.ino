#include <Wire.h>

//#define ARDUINO_I2C_MASTER_ADDRESS 0x07
#define ARDUINO_I2C_SLAVE_ADDRESS 0x07
#define BUFFER_SIZE 5

unsigned char serial_data_byte[BUFFER_SIZE] = {};
int data_bytes_count = 0;

void sendRecievedI2CDataWithUART(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size);
void receiveEvent();

void setup(void) {
  Wire.begin(ARDUINO_I2C_SLAVE_ADDRESS);
  Wire.setClock(400000L);
  //Wire.onReceive(receiveEvent);
  Serial.begin(9600);
}

void loop(void) {
    //Wire.requestFrom(ARDUINO_I2C_SLAVE_ADDRESS, BUFFER_SIZE);
    //if(!digitalRead(SERIAL_DEBUG_PIN)) {
      while(Wire.available() && data_bytes_count < BUFFER_SIZE) {
      serial_data_byte[data_bytes_count] = Wire.read();
      data_bytes_count++;
    }
      sendRecievedI2CDataWithUART(serial_data_byte, BUFFER_SIZE);
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

/*void receiveEvent(int howMany)
{
}*/
