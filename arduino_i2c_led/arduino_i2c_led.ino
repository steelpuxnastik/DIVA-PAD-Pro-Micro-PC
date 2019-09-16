#include <Wire.h>
#include <FastLED.h>

#define LED_PIN     5 
#define NUM_LEDS    60 
#define BRIGHTNESS  64 
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

void takeSliderTouchedArea(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size); //обработка и получение адреса светодиода в зависимости от нажатого сигмента сенсорной панели
void receiveEvent(); //получение данных по I2C с сенсорной панели
void DefaultSliderPalette(); //палитра что используется при простое и бездействии
void FillLEDsFromPaletteColors(uint8_t colorIndex); //функция заполнения палитрой ленты + проба внедрить условия, чтоб ее приглушить для отображения чего-то другого
void fadeall(); //попытка сделать плавное приглушение света от максимума до нуля. Работоспособность не подтверждена
void fadeLight(); //попытка сделать плавное наращивание яркости от 0 до максимума. Оказалось, что внутри использована вообще не та функция 
void setPaletteBrightnessDown(); //попытка плавного управления яркостью палитры через "яркость", от максимума до 0. Собственно, с плавностью большие проблемы, так как ни циклы, не задержки не дают визуального резульатата, но создают ощутимые задержки. Уменьшаем яркость для того, чтоб отображать в это время что-то другое
void setPaletteBrightnessUp(); //попытка управления наростания яркости палитры, когда "что-то другое" показывать не нужно 
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
	 
		FastLED.show();
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
			if(touchFlag){ //если есть прикосновение, то снизить яркость палитры, или выключить ее
				//fadeall();
				setPaletteBrightnessDown();
			}
			else{ // поднять яркость палитры
				//setPaletteBrightnessUp();
			}
				uint8_t brightness = brightness_n;
				leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending); //заливка палитры
				colorIndex += 1;
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

void fadeLight()
{
	for(int i = 0; i < NUM_LEDS; i++) 
	{ 
		//leds[i].fadeToBlackBy( 64 );
		leds[i].fadeLightBy( 64 );
	} 
}

void setPaletteBrightnessDown()
{
	//for (int i = 0; i < 256; ++i)
	//{
		/*if (brightness_n == 0)
			break;
		brightness_n--;*/
		brightness_n = 0;
	//}
}

void setPaletteBrightnessUp()
{
	for (int i = 0; i <= 255; ++i)
	{
		/*if (brightness_n == 255)
			break;*/
		brightness_n++;
	}
}

void sliderMovementHighlighting(int pixels)
{
	leds[pixels].maximizeBrightness();
	leds[pixels] = CRGB::Snow;
	leds[pixels-1] = CRGB::Snow;
	FastLED.show();
	fadeall();
}