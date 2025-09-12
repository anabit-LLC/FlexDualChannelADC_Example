/********************************************************************************************************
This example Arduino sketch is meant to work with Anabit's Flex Dual Channel ADC open source reference design
Product link: 

The Flex Dual Channel ADC design uses a single eneded 14 bit 2MSPS ADC that includes a 2 channel MUX that allows you to route signals on either input channel
to the ADC using a simple digital logic level. 

This sketch deomonstrats how to use the Flex ADC to make a single measurement or to make a group or burst of measurements as fast as possible. The single versus
burst mode is set by the "#define" MODE_SINGLE_MEASUREMENT or MODE_BURST_CAPTURE, comment out the mode you don't want to use. You also need to set the correct
voltage reference for the Flex Dual Channel ADC that you are using (2.5V, 3.3V, or 4.096V), see constant "VREF_VOLTAGE." You need to set the channel
you want to measure from: +IN1 or +IN2. See constant " CH_SEL." Pay attention to the SPI settings and the chip select pin you want to use. Define the SPI 
clock frequency you are using. Remember the SPI clock speed is what sets the sample rate of the ADc. The max SPI clock rate of the ADC is 40MHz. But be aware
not ever Arduino boards supports a SPI clock rate that. 

Please report any issue with the sketch to the Anagit forum: https://anabit.co/community/forum/analog-to-digital-converters-adcs
Example code developed by Your Anabit LLC © 2025
Licensed under the Apache License, Version 2.0.
**********************************************************************************************************/

#include <SPI.h>

// ===================== CONFIGURATION =====================
// Select ONE mode:
#define MODE_SINGLE_MEASUREMENT
//#define MODE_BURST_CAPTURE

// Chip-select pin: default to the board's CS pin for portability.
#define CS_PIN 10
//This pin controls the ADC MUX to select input channel. This is controled by the 
//CH SEL pin on the Flex. by default the pin is pulled down selecting channel 1
//so you can leave unconnected. If you want to control the input channel connect
//a digital pin to CH SEL and define the pin number here
#define CH_SEL -1 //set to -1 if you don't want to use
#define CHAN_1 0 //digital logic low for channel 1
#define CHAN_2 1 //digital logic high for channel 2

// SPI clock for the ADC (reduce if your board/ADC wiring can't sustain this)
#define ADS_SPI_HZ 10000000UL  // 40MHz is the fastest rated speed of the ADC

// ADC/reference constants
#define VREF_VOLTAGE 4.096f   //2.5f or 3.3f, switch to correct vref on your Flex
#define ADC_BITS 16383.0f // 14-bit code range (0..16383)

// SPI mode/settings for the ADC (mode 0, MSB first)
SPISettings adsSettings(ADS_SPI_HZ, MSBFIRST, SPI_MODE0);
//set variable for what channel you want to use
const uint8_t chan = CHAN_1; 

// ===== Burst mode buffers =====
#if defined(MODE_BURST_CAPTURE)
const int NUM_SAMPLES = 256;
uint16_t adcRaw[NUM_SAMPLES];
float    adcVoltage[NUM_SAMPLES];
#endif
// ==========================================================

void setup() {
  Serial.begin(115200);
  delay(2500); // time to open Serial Monitor/Plotter

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH); //start CS pin high
  pinMode(CH_SEL, OUTPUT);
  digitalWrite(CH_SEL,chan); //select MUX channel you want to use, default is channel 1
  SPI.begin();

#if defined(MODE_SINGLE_MEASUREMENT)
  Serial.println(F("Single Measurement Mode (CH0 via GPIO)"));
  readADS7945(0xC000);  // pipeline dummy read
  delayMicroseconds(5);

  while (true) {
    uint16_t raw = readADS7945(0xC000)>>2; //read 16 bit value and shift 2 right to make 14 bit value
    Serial.print(F("ADC Value unsigned hex: "));     Serial.println(raw, HEX);
    Serial.print(F("ADC Value unsigned decimal: ")); Serial.println(raw);
    float voltage = convertToVoltage(raw); //convert ADC code to voltage value
    Serial.print(F("Voltage: ")); Serial.print(voltage, 4); Serial.println(F(" V"));
    Serial.println();
    delay(2500);
  }

#elif defined(MODE_BURST_CAPTURE)
  Serial.println(F("Burst Capture Mode (CH0 via GPIO)"));
  while (true) {
    captureBurstPortable();
    for (int i = 0; i < NUM_SAMPLES; i++) {
      Serial.println(adcVoltage[i], 4);  // suitable for Serial Plotter
    }
    delay(2000);
  }
#else
  #error "Please define one of the modes."
#endif
}

void loop() {
  // not used
}

// One 16-bit framed transaction to the ADC
// will need to shift result to get correct 14 bit alignment
uint16_t readADS7945(uint16_t cmd) {
  SPI.beginTransaction(adsSettings);
  digitalWrite(CS_PIN, LOW);
  uint16_t value = SPI.transfer16(cmd);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  return value;
}

//Takes code form the ADC and converts it to a voltage value
//this function uses various constants defined earlier in the sketch
//input argument is the ADC code and returns voltage as float
float convertToVoltage(int16_t raw_code) {
    raw_code = raw_code & 0x3FFF; //apply mask to only get the 14 bits of interest
    return (((float)raw_code /ADC_BITS) * VREF_VOLTAGE);
}

#if defined(MODE_BURST_CAPTURE) //only compiled for burst mode
// Portable burst capture: standard digitalWrite + SPI
void captureBurstPortable() {
  // Prime the pipeline
  readADS7945(0xC000);
  delayMicroseconds(5);

  noInterrupts(); //turn off interupts
  SPI.beginTransaction(adsSettings);

  uint32_t t0 = micros(); //timer so you can see how fast your ucontroller can sample
  for (int i = 0; i < NUM_SAMPLES; i++) {
    digitalWrite(CS_PIN, LOW);
    adcRaw[i] = SPI.transfer16(0xC000);
    digitalWrite(CS_PIN, HIGH);
  }
  uint32_t duration = micros() - t0; //get time it took

  SPI.endTransaction();
  interrupts(); //turn interupts back on
  //print sample timing information
  Serial.print(F("Operation took "));
  Serial.print(duration);
  Serial.println(F(" microseconds."));
  //print buffer of readings out as voltages
  for (int i = 0; i < NUM_SAMPLES; i++) {
    adcVoltage[i] = convertToVoltage((adcRaw[i]>>2));
  }
}
#endif
