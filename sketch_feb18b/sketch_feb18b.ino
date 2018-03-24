#include "FastLED.h"
#define LED_PIN     13
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    80

#define FRAMES_PER_SECOND 60

//Spectrum Field Pin Connections
#define STROBE 4
#define RESET  5
#define DC_One A0
#define DC_Two A1

//Define LED's
CRGB leds[NUM_LEDS];

int bright_max = 100;

//Define spectrum variables
int freq_amp;
int Frequencies_One[7];
int Frequencies_Two[7];
int Scaled_Channel[7];
int i;
long baud = 1000000;

void setup() {
  // put your setup code here, to run once:

  //Set Spectrum shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);
  digitalWrite(STROBE, HIGH);
  digitalWrite(RESET, HIGH);

  //Initialize Spectrum Analyzers
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

long randNumber;
int weighted;

void loop() {
  Read_Frequencies();
  //Print_Frequencies(Frequencies_One);
  Scale_Band(Frequencies_One, NUM_LEDS, Scaled_Channel);
  for (i=0; i<NUM_LEDS; i++){
    if (i<(Scaled_Channel[1]) - 6){
      leds[i] = CRGB::Red;
    }
    if (i>=Scaled_Channel[1] - 6){
      leds[i] = CRGB::Black;
    }
  }

  FastLED.show();
  delay(40);
}

// This Function grabs the frequencies at any given time
// when called by the program and places the values between
// 0 and ~1000 into a 1x7 integer array for both left and 
// right channels. These arrays were previously initialized
// as Frequencies_One and Frequencies_Two at the top of
// this program.
void Read_Frequencies() {
  //Read frequencies for each band
  for (freq_amp = 0; freq_amp < 7; freq_amp++) {
    Frequencies_One[freq_amp] = analogRead(DC_One);
    Frequencies_Two[freq_amp] = analogRead(DC_Two);
    digitalWrite(STROBE, HIGH);
    digitalWrite(STROBE, LOW);
  }
}

// This function merely prints the values of the samplings
// taken from the eq chip and allows them to be printed to 
// the arduino console or serial plotter. It is mainly used
// for debugging and learning purposes.
void Print_Frequencies(int* channel) {
  for (int band = 0; band < 7; band++) {
    Serial.print(channel[band]);
    Serial.print('\t');
  }
  Serial.print('\n');
}

// This function will scale the inputs from the 7 band
// chip from its raw values represented in Frequencies_One
// and Frequencies_Two (ie from 0 to 1000) to a value between
// 0 and num_leds, where num_leds corresponds to the number of
// leds available per band on each frequency.
void Scale_Band(int* channel, int num_leds, int* Scaled_Channel){
  for (int band=0; band<7; band++){
    Scaled_Channel[band] = (int)(((float)channel[band]/1000.0)*num_leds);
  }
  for (int band=0; band < 7; band++){
    Serial.print(Scaled_Channel[band]);
    Serial.print('\t');
  }
  Serial.print('\n');

}





