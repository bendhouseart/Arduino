#include "FastLED.h"
#define LED_PIN     13
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    62

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
int Old_Scaled_Channel[7];
int New_Scaled_Channel[7];
int i;
long baud = 1000000;

//Defining Timer to rotate through bands on a single LED strip
long previousMillis = 0;
long interval = 30000;
int band_interval = 0; //to be used in timer
long timer2 = 0;
long interval2 = 100;

// Timer Variable for Floating Average
long oldTime = 0;
long intervalAvg = 10;


void setup() {
  
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

int weighted;
int color = 0;
int n_Samples = 40;
int n_Count = 0;
float new_average[] = {0, 0, 0, 0, 0, 0, 0};
float last_average[] = {0, 0, 0, 0, 0, 0, 0};
float present_average[] = {0, 0, 0, 0, 0, 0, 0};

void loop() {
  // 10 Second Timer
  // Simply changes the color of the led strip
  // doesn't do much else.
  unsigned long currentMillis = millis();
  if ( currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    band_interval ++;
    color = color + 30;
    if (band_interval%7 == 0 && band_interval != 0){
      band_interval = 0;
    }
  }

  // Here we make an initial sampling and do our first smoothing
  // this averaging will only occur n_samples number of times
  // after which smoothing will be accomplished by simply removing
  // the previous average/n_samples and adding the new values
  if (n_Count < n_Samples){  
    Read_Frequencies();
    Scale_Band(Frequencies_One, NUM_LEDS, Scaled_Channel);
    fArry_Cpy(present_average, last_average);
    for (int j = 0; j < 7; j++){
      new_average[j] += Scaled_Channel[j];
    }
    fArry_Cpy(new_average, present_average);
    n_Count++;
    
  } else if (n_Count == n_Samples){
    for (int i = 0; i < 7; i++){
      present_average[i] = new_average[i]/n_Samples;
    }
    n_Count++;
   
  }else {
    Read_Frequencies();
    Scale_Band(Frequencies_One, NUM_LEDS, Scaled_Channel);
    for (int k = 0; k < 7; k++){
      new_average[k] = present_average[k] + Scaled_Channel[k]/n_Samples - last_average[k]/n_Samples;
      //Serial.print(new_average[k]);
      //Serial.print("\t");
    }
    //Serial.print("\n");
    fArry_Cpy(present_average, last_average);
    fArry_Cpy(new_average, present_average);
    Light(new_average, band_interval, color);
    
  }
  

//  //10 Millisecond timer
//  unsigned long now = millis();
//  if ( now - oldTime > intervalAvg) {
//    oldTime = now;
//    for (i=0; i<NUM_LEDS; i++){
//      if (i < Scaled_Channel[band_interval]){
//        leds[i] = CHSV(color, 255, 255);
//      }
//      if (i >= Scaled_Channel[band_interval]){
//        leds[i] = CRGB::Black;
//      }
//    }
//    FastLED.show();
//  }
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
    // Comment out the below line for stereo operation
    Frequencies_One[freq_amp] = (int)((Frequencies_One[freq_amp] + Frequencies_Two[freq_amp])/2.0);
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
    if (channel[band] > 70){
      Scaled_Channel[band] = (int)((1.5*(float)channel[band]/1000.0)*num_leds);
    } else {
      Scaled_Channel[band] = 0;
    }
  }
} 

// This function actually lights up all of the leds in a single array
void Light(float* Scaled_Channel, int band_interval, int color){
   for (i=0; i<NUM_LEDS; i++){
      if (i < (int)Scaled_Channel[band_interval]){
        leds[i] = CHSV(color, 255, 255);
      }
      if (i >= (int)Scaled_Channel[band_interval]){
        leds[i] = CRGB::Black;
      }
    }
    FastLED.show();
}

// This function copies the contents of one array into another
void Arry_Cpy(int* array1, int* array2){
  for (int kk = 0; kk < 7; kk++){
    array2[kk] = array1[kk];
  }
}

// This function copies a float array
void fArry_Cpy(float* array1, float* array2){
  for (int jj = 0; jj <7; jj++){
    array2[jj] = array1[jj];
  }
}






