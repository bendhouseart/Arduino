// Inclusion of libraries and definition of uno hardward variables
// and settings

#include "FastLED.h"
#define LED_PIN		13
#define COLOR_ORDER	GRB
#define CHIPSET		WS2811
#define NUM_LEDS	62
#define FRAMES_PER_SECOND 60
#define threshold 80

// Connections to Spectrum Analyser
#define STROBE 4
#define RESET  5
#define DC_One A0
#define DC_Two A1

// Define Total number of leds in Strip
CRGB leds[NUM_LEDS];

// Sample rate
long baud = 1000000;

// Define the size of the led band (ie the number of leds used to display 
// a frequency response). Example: You have 70 total leds (NUM_LEDS = 70)
// and you wish to view 7 bands at a time responding, your band size could
// be calculated to be 10 by;
int bandSize = NUM_LEDS/7;


// Initialize Array for Left and Right Channels of Frequency Anaylser
int frequenciesLeft[7];
int frequenciesRight[7];

// Creating Function to read frequencies from Frequency Analyser when Called
// and read those frequencies (values ranging from 0 to 1023 into their respective
// left and right channels. The noise floor of the Frequency Analyser appears
// to be 70 to 80, so values below that floor will be discarded and set to a
// floor equal to the threshold value.
void Read_Frequencies() {
	for (int i = 0; i < 7; i++){
		frequenciesLeft[i] = analogRead(DC_One);
		frequenciesRight[i] = analogRead(DC_Two);
		// discarding noise
		if (frequenciesLeft[i] < threshold){
			frequenciesLeft[i] = threshold;
		}
		if (frequenciesRight[i] < threshold){
			frequenciesRight[i] = threshold;
		}	
		// making them mono
		//frequenciesLeft[i] = (frequenciesLeft[i] + frequenciesRight[i])/2;
	}
	
	digitalWrite(STROBE, HIGH);
	digitalWrite(STROBE, LOW);
}

void Print_Frequencies(int* frequenciesLeft,  int* frequenciesRight) {
	for (int i = 0; i < 7; i++){
		Serial.print(frequenciesLeft[i]);
		Serial.print('\t');
	}
	for (int i = 0; i < 7; i++){
		Serial.print(frequenciesRight[i]);
		Serial.print('\t');
	}
	Serial.print('\n');

}
// Here we scale the frequencies read by the Freq. Anal
// to something representable by our led strip.
// each band will be divided into it's own second of leds
// of width size bandSize calculated via the total number
// of leds in the strip. 
int scaledLeft[7];
int scaledRight[7];
void Scale_Frequencies(int* leftIn, int* rightIn, int* leftOut, int* rightOut, int bandSize){
	for (int band=0; band < 7; band++){
		float tempLeft;
		float tempRight;
		if (leftIn[band] > threshold){
			tempLeft = (((float)leftIn[band] - (float)threshold)/(1023.0 - (float)threshold))*bandSize;
			leftOut[band] = (int)tempLeft;
		} else {
			leftOut[band] = 0;
		}
		
		if (rightIn[band] > threshold){
			tempRight = (((float)(rightIn[band] - threshold))/((float)(1023 - threshold)))*bandSize;
			rightOut[band] = (int)tempRight;
		} else {
			rightOut[band] = 0;
		}
	}
} 

void Light(int* scaledLeft, int* scaledRight, int bandSize){
	int step = bandSize;
	int color = 0;
	for (int i = 0; i < 7; i++){
		color = i*32;
		for (int j=(i*bandSize); j < ((i+1)*bandSize); j++){
			if (j < (scaledLeft[i] + (i*bandSize))){
				leds[j] = CHSV(color, 255, 255);
			}
			if (j >= (scaledLeft[i] + (i*bandSize))){
				leds[j] = CRGB::Black;
			}
			Serial.print(i);
			Serial.print('\t');
			Serial.print(scaledLeft[i]);
			Serial.print('\t');
			Serial.print(j);
			Serial.print('\n');
		}
	
	FastLED.show();
	}


}

void setup() {
	// Set Freq. Anal. pin configs.
	pinMode(STROBE, OUTPUT);
	pinMode(RESET, OUTPUT);
	pinMode(DC_One, INPUT);
	pinMode(DC_Two, INPUT);
	digitalWrite(STROBE, HIGH);
	digitalWrite(RESET, HIGH);
	
	// Initialize Spectrum Analysers
	digitalWrite(STROBE, LOW);
	delay(1);
	digitalWrite(RESET, HIGH);
	delay(1);
	digitalWrite(STROBE, HIGH);
	delay(1);
	digitalWrite(STROBE, LOW);
	delay(1);
	digitalWrite(RESET, LOW);
	delay(300);

	FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness( 70 );
	Serial.begin(baud);
	randomSeed(analogRead(0));
}

void loop() {
	Read_Frequencies();
	//Print_Frequencies(frequenciesLeft, frequenciesRight);
	Scale_Frequencies(frequenciesLeft, frequenciesRight, scaledLeft, scaledRight, bandSize);
	Serial.print('\n');
	//Print_Frequencies(scaledLeft, scaledRight);
	Light(scaledLeft, scaledRight, bandSize); 
} 