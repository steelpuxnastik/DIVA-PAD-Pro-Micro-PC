#include <Wire.h>
#include <FastLED.h>

#define LED_PIN     5 //Пин на ардуине с которого подается сигнал на ленту
#define NUM_LEDS    60 //количество светодиодов в ленте
#define BRIGHTNESS  64 //яркость ленты
#define LED_TYPE    WS2811 //тип ленты
#define COLOR_ORDER GRB //цветовая модель
CRGB leds[NUM_LEDS];
 
#define UPDATES_PER_SECOND 100

//LED-Slider Map
#define LED_i2c_buffer_1 0
#define LED_i2c_buffer_2 15
#define LED_i2c_buffer_3 31
#define LED_i2c_buffer_4 47

#define ARDUINO_I2C_SLAVE_ADDRESS 0x07
#define BUFFER_SIZE 5

unsigned char serial_data_byte[BUFFER_SIZE] = {};
int data_bytes_count = 0;

void sendRecievedI2CDataWithUART(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size, uint8_t colorIndex);
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
		//Wire.setClock(400000L);
		Wire.onReceive(receiveEvent);
		Serial.begin(115200);
}

void loop(void) {
		DefaultSliderPalette();
	 
		static uint8_t startIndex = 0;
		startIndex = startIndex + 3; // скорость движения
		sendRecievedI2CDataWithUART(serial_data_byte, BUFFER_SIZE, startIndex);
		FillLEDsFromPaletteColors(startIndex);
	 
		FastLED.show();
		FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void sendRecievedI2CDataWithUART(unsigned char serial_data_byte[BUFFER_SIZE], int buffer_size, uint8_t colorIndex) {
	uint8_t brightness = 255;
	int len = BUFFER_SIZE * 9 + 2;
	int led_i = 0;
	int serDataToLed[8] = {};
	int serDataToLed_i = 0;
	for(int i = 1; i < buffer_size; i++) {
		for(int j = 0; j < 9; j++) {
			if(j < 8) {
				if((serial_data_byte[i] >> (7 - j)) & 0x01) {
					if(i == 1){
						led_i = LED_i2c_buffer_1;
					}
					if(i == 2){
						led_i = LED_i2c_buffer_2;
					}
					if(i == 3){
						led_i = LED_i2c_buffer_3;
					}
					if(i == 4){
						led_i = LED_i2c_buffer_4;
					}
					for(int l = 0; l < 8; l++){
						if(bitRead(serial_data_byte[i], l) == 1){
							serDataToLed[serDataToLed_i] = (l * 2) + led_i;
							serDataToLed_i++;
						}
					}
					leds[led_i]= ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
					leds[led_i+1]= ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
					colorIndex += 3;
				} else {
					//c = '_';
				}
			} 
		}
	}
	for (int i = 0; i < sizeof(serDataToLed); ++i)
	{
		Serial.println(serDataToLed[i]);
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
		uint8_t brightness = 255;
	 
		for( int i = 0; i < NUM_LEDS; i++) {
				leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
				colorIndex += 3;
		}
}
 
// Этот фрагмент скетча демонстрирует несколько разных
// цветовых палитр. В библиотеку FasLED уже встроено несколько
// палитровых шаблонов: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p и
// PartyColors_p.
//
// Кроме того, вы можете создавать собственные палитры или даже
// написать код, создающий палитры прямо на ходу.
// Ниже продемонстрировано, как все это сделать.
 
void DefaultSliderPalette()
{
		/*uint8_t secondHand = (millis() / 1000) % 60;
		static uint8_t lastSecond = 99;*/

		currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND;
	 
		/*if( lastSecond != secondHand) {
				lastSecond = secondHand;
				if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
				//if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
				//if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
				//if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
				//if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
				//if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
				//if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
				//if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
				//if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
				//if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
				//if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
		}*/
}
 
// Эта функция заполняет палитру совершенно случайными цветами.
 
/*void SetupTotallyRandomPalette()
{
		for( int i = 0; i < 16; i++) {
				currentPalette[i] = CHSV( random8(), 255, random8());
		}
}*/
 
// Эта функция делает палитру из черных и белых линий.
// Поскольку палитра – это, в сущности, массив
// из шестнадцати CRGB-цветов, для ее создания можно использовать
// различные функции fill_* – вроде fill_solid(), fill_gradient(),
// fill_rainbow() и т.д.
 
/*void SetupBlackAndWhiteStripedPalette()
{
		// сначала делаем все фрагменты черными...
		fill_solid( currentPalette, 16, CRGB::Black);
		// ...а потом делаем каждый четвертый фрагмент белым:
		currentPalette[0] = CRGB::White;
		currentPalette[4] = CRGB::White;
		currentPalette[8] = CRGB::White;
		currentPalette[12] = CRGB::White;
	 
}*/
 
// Эта функция заполняет палитру фиолетовыми и зелеными полосами.
 
/*void SetupPurpleAndGreenPalette()
{
		CRGB purple = CHSV( HUE_PURPLE, 255, 255);
		CRGB green  = CHSV( HUE_GREEN, 255, 255);
		CRGB black  = CRGB::Black;
	 
		currentPalette = CRGBPalette16(
																	 green,  green,  black,  black,
																	 purple, purple, black,  black,
																	 green,  green,  black,  black,
																	 purple, purple, black,  black );
}*/
 
// Фрагмент кода ниже показывает, как создать статичную палитру,
// хранящуюся в памяти PROGMEM (т.е. во flash-памяти).
// Этот тип памяти, как правило, просторней, чем RAM.
// Статичная палитра вроде той, создание которой показано ниже,
// занимает, как правило, 64 байта flash-памяти.
 
/*const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
		CRGB::Red,
		CRGB::Gray, // белый – слишком яркий свет
								// по сравнению с красным и синим
		CRGB::Blue,
		CRGB::Black,
	 
		CRGB::Red,
		CRGB::Gray,
		CRGB::Blue,
		CRGB::Black,
	 
		CRGB::Red,
		CRGB::Red,
		CRGB::Gray,
		CRGB::Gray,
		CRGB::Blue,
		CRGB::Blue,
		CRGB::Black,
		CRGB::Black
};*/
