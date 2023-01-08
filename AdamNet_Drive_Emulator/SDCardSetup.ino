void SDCardSetup(){                                                // Setup the SD Card
  pinMode(SS, OUTPUT);                     // This makes sure that the default hardware "Slave Select" pin is set to output, even if a different pin is used for ChipSelect
  if (!sd.begin(ChipSelect, SD_SCK_MHZ(50))) {// Initialize at the highest speed supported by the board that is not over 50 MHz. Try a lower speed if SPI errors occur.
    lcd.clear();
#ifdef PRO_MINI_OLED_BOARD
    lcd.draw1x2String(0,0, String(F("   No SD Card")).c_str());
#else  
    lcd.setCursor(0,0);
    lcd.print(F("   No SD Card"));
#endif    
    sd.initErrorHalt();
  }         
  Serial.print(F("Changing to the Initial Dir: "));
  Serial.println(CurrentDirectory);
  sd.chdir(CurrentDirectory.c_str(),true);
}
