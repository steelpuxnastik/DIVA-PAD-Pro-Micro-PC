#include <Wire.h>
#include <FastLED.h>

#define LED_PIN     5 //Пин на ардуине с которого подается сигнал на ленту
#define NUM_LEDS    60 //количество светодиодов в ленте
#define BRIGHTNESS  64 //яркость ленты
#define LED_TYPE    WS2812 //тип ленты
#define COLOR_ORDER GRB //цветовая модель
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

void takeSliderTouchedArea(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size);
void receiveEvent();
void ChangePalettePeriodically();
void FillLEDsFromPaletteColors(uint8_t colorIndex);

CRGBPalette16 currentPalette;
TBlendType    currentBlending;
 
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void setup(void) {
		delay(3000);// небольшая задержка для того, чтобы цепь
					// «устаканилась» после включения питания
 
		FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
		FastLED.setBrightness(BRIGHTNESS);
	 
		currentPalette = RainbowColors_p;
		currentBlending = LINEARBLEND;
	
		Wire.begin(ARDUINO_I2C_SLAVE_ADDRESS);
		Wire.onReceive(receiveEvent);
		//Serial.begin(9600);
}

void loop(void) {
		DefaultSliderPalette();
	 
		static uint8_t startIndex = 0;
		startIndex = startIndex + 1; // скорость движения
		takeSliderTouchedArea(serial_data_byte, BUFFER_SIZE);
		FillLEDsFromPaletteColors(startIndex);
	 
		FastLED.show();
		FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void takeSliderTouchedArea(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size) {
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
							brightness_n = 0;
							leds[led_i - (l *2)] = CRGB::White;
							leds[(led_i - (l *2))-1] = CRGB::White;
							FastLED.show();
						}
						else{
							brightness_n = 255;
						}

					}
				} else {
					
				}
			} 
		}
	}
	//brightness_n = 255;
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
		uint8_t brightness = brightness_n;
	 
		for( int i = 0; i < NUM_LEDS; i++) {
				leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
				colorIndex += 3;
		}
}
 
void DefaultSliderPalette()
{
		currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND;
}