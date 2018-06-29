float voltSense(const unsigned int input, unsigned long sampleInterval, unsigned long numSamples) {

  const float vpc = 4.8828125;  
  unsigned long sampleAccAV = 0;
  unsigned int count = 0;
  unsigned long prevMicros = micros() - sampleInterval;

  while (count < numSamples) {
    
    if (micros() - prevMicros >= sampleInterval) {

      int sample = analogRead(input);
      sampleAccAV += (unsigned long)(sample);
      ++count;
      prevMicros += sampleInterval;     
    }

  }

  float sampleAV = (float)sampleAccAV / (float)numSamples;
  
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
  //float sampleAV = (float)sampleAccAV / (float)numSamples;
  float voltEff = sampleRMS * vpc;

return (voltEff);
  
}
