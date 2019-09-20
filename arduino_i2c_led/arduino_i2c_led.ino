#include <Wire.h>
#include <FastLED.h>

#define LED_PIN     5 
#define NUM_LEDS    60 
#define BRIGHTNESS  255 
#define LED_TYPE    WS2812B 
#define COLOR_ORDER GRB 
CRGB leds[NUM_LEDS];
 
#define UPDATES_PER_SECOND 100

//LED-Slider Map
#define ledI2CBuffer_1 15
#define ledI2CBuffer_2 31
#define ledI2CBuffer_3 47
#define ledI2CBuffer_4 63

#define ARDUINO_I2C_SLAVE_ADDRESS 0x07
#define BUFFER_SIZE 5

unsigned char serial_data_byte[BUFFER_SIZE] = {};
int data_bytes_count = 0;
int brightness_n = 255;
boolean touchFlag = false;

int fadeControl = 255;//will hold the current brightness level
int fadeDirection = 70;//change sigen to fade up or down
int fadeStep = 10;//delay between updates

void takeSliderTouchedArea(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size); //обработка и получение адреса светодиода в зависимости от нажатого сигмента сенсорной панели
void receiveEvent(); //получение данных по I2C с сенсорной панели
void DefaultSliderPalette(); //палитра что используется при простое и бездействии
void FillLEDsFromPaletteColors(uint8_t colorIndex); //функция заполнения палитрой ленты + проба внедрить условия, чтоб ее приглушить для отображения чего-то другого
void fadeall(); //попытка сделать плавное приглушение света от максимума до нуля. Работоспособность не подтверждена
void sliderMovementHighlighting(int pixels); //функция подсвечивания мест прикосновения к сенсорной панели

CRGBPalette16 currentPalette;
TBlendType    currentBlending;
 
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void setup(void) 
{
		delay(3000);// небольшая задержка для того, чтобы цепь
					// «устаканилась» после включения питания
 
		FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
		FastLED.setBrightness(BRIGHTNESS);
	
		Wire.begin(ARDUINO_I2C_SLAVE_ADDRESS);
		Wire.onReceive(receiveEvent);
		//Serial.begin(9600);
}

void loop(void) 
{
		DefaultSliderPalette();
	 
		static uint8_t startIndex = 0; 
		startIndex = startIndex + 1; // скорость движения
		takeSliderTouchedArea(serial_data_byte, BUFFER_SIZE);
		FillLEDsFromPaletteColors(startIndex);

		//FastLED.show();
		FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void takeSliderTouchedArea(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size) 
{
	int len = BUFFER_SIZE * 9 + 2;
	int led_i = 0;
	for(int i = 1; i < buffer_size; i++) {
		if(i == 1){
			led_i = ledI2CBuffer_1;
		}
		if(i == 2){
			led_i = ledI2CBuffer_2;
		}
		if(i == 3){
			led_i = ledI2CBuffer_3;
		}
		if(i == 4){
			led_i = ledI2CBuffer_4;
		}
		for(int j = 0; j < 9; j++) {
			if(j < 8) {
				if((serial_data_byte[i] >> (7 - j)) & 0x01) {
					for(int l = 0; l < 8; l++){
						if(bitRead(serial_data_byte[i], l) == 1){
							touchFlag = true; //флаг, если прикосновение к сенсорной панели существует
							sliderMovementHighlighting(led_i - (l *2)); //передача функции подсвеченивания мест прикосновения координат светодиодов
							Serial.println(touchFlag);
						}
						else{
							touchFlag = false;
						}
					}
				}
			} 
		}
	}
}

void receiveEvent(int howMany)
{
	data_bytes_count = 0;
	while(Wire.available() && data_bytes_count < BUFFER_SIZE){
	serial_data_byte[data_bytes_count] = Wire.read();
	data_bytes_count++;
	}
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
	for( int i = 0; i < NUM_LEDS; i++) {
		uint8_t brightness = brightness_n;
		leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending); //заливка палитры
		colorIndex += 1;
	}
	FastLED.setBrightness(fadeControl);
    FastLED.show(); // Update strip with new contents
    fadeControl = fadeControl + fadeDirection;//increment the brightness value
    //Serial.println(fadeControl);
    Serial.println(touchFlag);
    if (touchFlag && fadeDirection > 0)
			fadeDirection = fadeDirection * -1;//change the direction... 

    if (!touchFlag && fadeDirection < 0)
			fadeDirection = fadeDirection * -1;//change the direction... 
    if (fadeControl <= 0)
    {
      fadeControl = 0;
    }
    if (fadeControl >= 255)
    {
      fadeControl = 255;
    }

}
 
void DefaultSliderPalette()
{
		currentPalette = RainbowColors_p;
		currentBlending = LINEARBLEND;
}

void fadeall() 
{
	//for( int i = 0 ; i < 10; i++ ) {
   		for( int led = 0 ; led < NUM_LEDS ; led++ ) {
       		leds[led].nscale8( sin8(led) );
   		}
	//}
		//fadeToBlackBy( leds, NUM_LEDS, 20);
}

void sliderMovementHighlighting(int pixels)
{
	leds[pixels].maximizeBrightness();
	leds[pixels] = CRGB::Snow;
	leds[pixels-1].maximizeBrightness();
	leds[pixels-1] = CRGB::Snow;
	FastLED.show();
	fadeall();
}