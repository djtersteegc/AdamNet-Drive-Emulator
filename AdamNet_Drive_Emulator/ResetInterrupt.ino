void ResetInterrupt(){                                             // Reset from the Adam
  unsigned long resettimeout = 0;
  if (!DisableNextReset){
    if ((millis() - TimetoByte > 200) && (ResetFlag == 0)){
      TimetoByte = millis();               // Set the time that the reset came in
      while (((RESET_PORT) == 0) && (resettimeout <= 10000)){ // Wait for AdamNetReset to go HIGH, This gets out of the reset.
        resettimeout++;
      }
      ResetFlag = 1;                       // Set the reset flag
    }
  }
  else{
    Serial.println(F("Skipping this reset"));
    Serial.println(F("Compatible Block Mode Enabled"));
    CompMode = true;                       // Turn on Compatible Block Mode
    DisableNextReset=false;
  }

  EIFR = bit (RESET_INT);                      // Clear flag for any interrupts on AdamNetReset that were triggered while in the ISR
}
