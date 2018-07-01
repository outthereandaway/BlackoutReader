float voltSense(const unsigned int input, unsigned long sampleInterval, unsigned long numSamples) {

/* function the sense the voltage of ZMCT103C 5A Range Single Phase AC Active Output Current Transformer Module */

  const float vpc = 4.8828125; // increase/decrease of mV per step on Arduino analogue input ((5 / 1024) * 1000)
  unsigned long sampleAccAV = 0; // average of accumulated samples
  unsigned int count = 0; // counter for accumulations
  unsigned long prevMicros = micros() - sampleInterval; // storage for microseconds of last sample taken 

  // calculation of average (arithmetic mean) of samples over sample interval. Since we're dealing with a somewhat periodic signal, the average defines the 0-value of the signal.
  while (count < numSamples) {
    
    if (micros() - prevMicros >= sampleInterval) {

      int sample = analogRead(input);
      sampleAccAV += (unsigned long)(sample);
      ++count;
      prevMicros += sampleInterval;     
    }

  }
  
  float sampleAV = (float)sampleAccAV / (float)numSamples;

  // calculation of rms value over sample interval. Every sample taken is corrected by the before calculated average (0-value).
  unsigned long sampleAccRMS = 0;
  count = 0;
  prevMicros = micros() - sampleInterval;

  while (count < numSamples) {

    if (micros() - prevMicros >= sampleInterval) {

      int sample = analogRead(input);
      sampleAccRMS += (unsigned long)(sq(sample - sampleAV));
      ++count;
      prevMicros += sampleInterval;
      
    }
  }

  float sampleRMS = sqrt((float)sampleAccRMS / (float)numSamples);
  
  float voltEff = sampleRMS * vpc; // conversion of measured value (0 - 1023) to mV value

return (voltEff);
  
}
