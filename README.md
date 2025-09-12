# FlexDualChannelADC_Example
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
