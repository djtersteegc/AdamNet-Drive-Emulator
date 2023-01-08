void LCDRefresh(){                                                 // Refresh the LCD Display
  String temptext = "";
  byte LCDLength;
  //Move updating the LCD from a SDCommand to here since the OLED screen is to slow and throws off the timing inside the command block
  if (LCDWriteCommandNeeded) {
    lcd.draw1x2String(0,0, LCDCommandTopText);
    lcd.draw1x2String(0,2, LCDCommandBottomText);
    LCDWriteCommandNeeded = false;
  }
  if (LCDScrollOn && LCDTopDelay == 0){
    LCDScroll();                           // Scroll the bottom line of the LCD
  }
  if (LCDTopDelay == 1){
    if ((BootDiskMounted == 1) && (DeviceDisplayed == 4)){
      temptext = "D1:" + BootDisk;
    }
    else{
      sd.chdir(CurrentDirectory.c_str(),true);
      temptext = EepromStringRead((DeviceDisplayed * 400) + 5);
      temptext = String("D") + (DeviceDisplayed - 3) + String(":") + temptext;
    }
#ifdef PRO_MINI_OLED_BOARD
    lcd.draw1x2String(0,0, temptext.substring(0, 16).c_str());
    for (int i = temptext.length(); i<16; i++) {
      lcd.draw1x2String(i,0, " ");
    }
#else    
    lcd.setCursor(0,0);
    LCDLength = temptext.length();
    if (LCDLength > 16){
      LCDLength = 16;
    }
    for (int i = 0; i < LCDLength; i++) {
      lcd.print(temptext.substring(i,i+1));
    }
    for (int i = LCDLength; i <= 15; i++) {
      lcd.print(" ");
    }
#endif
    LCDTopDelay = 0;
  }
  else if(LCDTopDelay > 0){
    LCDTopDelay--;
  }
  if (LCDBottomDelay == 1){
    sd.chdir(CurrentDirectory.c_str(),true);
    LCDBottomText  = GetFileName(CurrentFile,LCDNameLength);
#ifdef PRO_MINI_OLED_BOARD
    lcd.draw1x2String(0,2, LCDBottomText.substring(0, 16).c_str());
    for (int i = LCDBottomText.length(); i<16; i++) {
      lcd.draw1x2String(i,2, " ");
    }    
#else    
    lcd.setCursor(0,1);
    LCDLength = LCDBottomText.length();
    if (LCDLength > 16){
      LCDLength = 16;
    }
    for (int i = 0; i < LCDLength; i++) {
      lcd.print(LCDBottomText.substring(i,i+1));
    }
    for (int i = LCDLength; i <= 16; i++) {
      lcd.print(" ");
    }
#endif
    CurrentLCDDelay = LCDScrollDelayStart;
    LCDScrollLocation = 0;
    LastScrollLCD = millis();
    LCDScrollOn = true;
    LCDBottomDelay = 0;
  }
  else if(LCDBottomDelay > 1){
    LCDBottomDelay--;
  }
}
