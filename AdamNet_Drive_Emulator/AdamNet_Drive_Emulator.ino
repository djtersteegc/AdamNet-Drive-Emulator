#include <util/delay.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SdFat.h> //Does not work with 2.x versions, use 1.1.4

#define PRO_MINI_OLED_BOARD //Uncomment this line to use the with https://github.com/djtersteegc/coleco-adam-ade-pro-embed-shield

#ifdef PRO_MINI_OLED_BOARD
#include <U8x8lib.h>
#else
#include <LiquidCrystal.h>
#endif

//============================================================================================================================================
//==================================             AdamNet Drive Emulator (ADE)   v0.91                 ========================================
//============================================================================================================================================
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓   Only modify the following variables   ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
const byte Version[3] =  {0,9,2};          // ADE Version Number
const byte StatusLEDState = LOW;           // Initial state for the status LED's 
                                           // LOW = Normally off, on for activity   HIGH = Normally on, off for activity (ADE Lite = HIGH)
const byte EnableAnalogButtons = false;     // For the 1602 Keypad Shield Buttons, leave this as 'true'.  (ADE Pro / ADE Lite = false)
                                           // If you are using individual digital buttons set it to 'false'.
const unsigned int StartupDelay = 0;       // Additional delay on startup in ms. 
                                           // This can help the Adam get to SmartWriter before the ADE finishes booting.
const unsigned int MaxFiles = 300;         // The maximum number of file names to load into the directory index array.
const byte LCDNameLength = 99;             // Maximum length of file name to display on LCD.
                                           // 17 will disable scrolling. 12 or less will display 8.3 filenames (max = 99)
const byte StatusLed[4] = {13,13,13,13};   // Pins for Status LED. These can be combined or 1 for each device. 13 is the internal LED on the Mega
const String BootDisk = "boot.dsk";        // Name of disk to auto-mount on Device 4. This will override the eeprom file. Set to "" for no boot disk
const String BootDiskDir = "/";            // Directory that the boot disk is in.
const byte BootDiskType = 10;              // Set the file type for the boot disk. 10 = Disk, 11 = DDP. No other type is valid.
const byte AnalogButtonSensitivity = 100;  // This will change the analog button sensitivity. This is a percentage of the default values.
const unsigned int LCDScrollDelay = 300;   // How many milliseconds between the bottom LCD line scrolls
const unsigned int LCDScrollDelayStart = 2000;// Delay for the bottom LCD line to start scrolling
const byte EnableFACE = true;              // True will enable the default Adam FACE command for formatting.
                                           // True will cause problems with disk images greater than 64,205 (0xFACD) blocks.
                                           // False will disable the FACE command and treat 0x0000FACE as a normal block.
                                           // False will not allow normal Adam format programs to function.
const long IODelay = 0;                    // Delay in microseconds the loading and saving of each block. This can help to match the timing of an actual drive.
                                           // A real Adam disk drive is 300,000 - 500,000 (0.3-0.5 sec), however 0 works for everything I tried.
const unsigned long LCDDelay = 40000;      // How long a message will stay on the LCD until it reverts to regular display.
                                           // Setting LCDScrollOn = false will disable this.
const unsigned long LongKey = 100000;      // Delay to determine what is a long key press. This is loop timing not ms.
const byte RepeatKeyDelay = 180;           // How long to wait before repeating key press. Only Up and Down will repreat if held.(ms)
String CurrentDirectory = "/";             // The initial directory for the LCD display and the SD commands.
//↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
//↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
#ifdef PRO_MINI_OLED_BOARD
const byte AdamNetRx = 3;                 // AdamNet Receive pin.(3 = INT5 = PE5). Do not change this. It is hard coded.
#define RX_PORT PINE & _BV(PE5)
const byte RX_INT = INTF5;
const byte AdamNetTx = 19;                 // AdamNet Transmit pin. (19 = INT2 = PD2). Do not change this. It is hard coded.
const byte TX_PORT = PD2;
const byte AdamResetPin = 2;              // Pin for optional output to AdamNet reset line. (2 = INT4 = PE4). Do not change this. It is hard coded.
#define RESET_PORT PINE & _BV(PE4)
const byte RESET_INT = INTF4;
//U8X8_SSD1306_128X32_UNIVISION_SW_I2C lcd(/* clock=D21*/ 21, /* data=D20*/ 20);  //Uses 103 bytes less of dynamic memory, but is slower to refresh
U8X8_SSD1306_128X32_UNIVISION_HW_I2C lcd(/* clock=D21*/ 21, /* data=D20*/ 20);
const int RightButtonPin = 35;             // Pin for optional Right digital button.
const int UpButtonPin = 39;                // Pin for optional Up digital button.
const int DownButtonPin = 33;              // Pin for optional Down digital button.
const int LeftButtonPin = 41;              // Pin for optional Left digital button.
const int SelectButtonPin = 43;            // Pin for optional Select digital button.
const int SwapButtonPin = 37;              // Pin for optional Swap digital button
#else
const byte AdamNetRx = 19;                 // AdamNet Receive pin.(19 = INT2 = PD2). Do not change this. It is hard coded.
#define RX_PORT PIND & _BV(PD2)
const byte RX_INT = INTF2;
const byte AdamNetTx = 21;                 // AdamNet Transmit pin. (21 = INT0 = PD0). Do not change this. It is hard coded.
const byte TX_PORT = PD0;
const byte AdamResetPin = 20;              // Pin for optional output to AdamNet reset line. (20 = INT1 = PD1). Do not change this. It is hard coded.
#define RESET_PORT PIND & _BV(PD1)
const byte RESET_INT = INTF1;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);       // Pins that the LCD are on, Default is 1602 Keypad Shield, buttons are on A0
const int RightButtonPin = 25;             // Pin for optional Right digital button.
const int UpButtonPin = 23;                // Pin for optional Up digital button.
const int DownButtonPin = 24;              // Pin for optional Down digital button.
const int LeftButtonPin = 26;              // Pin for optional Left digital button.
const int SelectButtonPin = 22;            // Pin for optional Select digital button.
const int SwapButtonPin = 42;              // Pin for optional Swap digital button
#endif
const byte ChipSelect = 53;                // Chip select (CS) pin for the SD Card (53),Connect SD Card Shield: (MISO = 50, MOSI = 51, SCK = 52)
                                           // If SD card shield has 3.3v regulator then connect 5v, else 3.3v. Don't forget the GND.
const byte Interleave = 5;                 // This is the Interleave used for the disk image file layout. Not sure if changing it will actually work.
unsigned int CurrentLCDDelay = LCDScrollDelay;// The current LCD scroll delay
byte Device4;                              // Enable for Device 4 - Eeprom Byte 4
byte Device5;                              // Enable for Device 5 - Eeprom Byte 5
byte Device6;                              // Enable for Device 6 - Eeprom Byte 6
byte Device7;                              // Enable for Device 7 - Eeprom Byte 7
byte DebugMode;                            // Enable Debug Mode for the LCD - Eeprom Byte 33
byte CompMode = true;                      // Compatability mode. If true, disables the high word block address.
byte Status[4][6];                         // Status array. This holds the current status byte for each drive.
byte BlockBuffer[4][1028];                 // Block buffer array : 3 byte header + 1024 byte buffer + 1 byte checksum, Checksum is only for the 1024 bytes
byte DeviceDisplayed;                      // The currently displayed device number on the top row of the LCD
unsigned long LoadedBlock[4] = {0xFFFFFFFF,
                               0xFFFFFFFF,
                               0xFFFFFFFF,
                               0xFFFFFFFF};// The current block number that is loaded in the buffer array
unsigned int FilesIndex[MaxFiles + 1];     // Index for the files on the SD card
byte TypeIndex[MaxFiles + 1];              // Index for the file type: 
                                           // 0 = Back to Root, 1 = Back Dir, 2 = Dir
                                           // 10 = DSK, 11 = DDP, 12 = ROM,COL,BIN
                                           // 22 = DS2, 23 = DS3, 24 = DS4
unsigned int NumberofFiles = 0;            // The number of files in the current dir including directories
unsigned int CurrentFile = 1;              // The current file index number that being displayed
unsigned long LastButtonTime;              // Timer for last button push
unsigned long LastCommandTime;             // Timer for last command processed. This is for the keypress disable while commands are coming in.
unsigned long LastScrollLCD;               // Timer for scrolling the LCD
unsigned long TimetoByte = 0xFFFFFFFF;     // Timer for reset, used to determine Hard or Soft reset
byte UnknownCommand = 0;
byte AdamnetDisconnected =0;
byte LCDScrollOn = true;                   // Turn off the scroll in the program. Used when writing to LCD. Turned back on with refreshscreen.
unsigned int LCDScrollLocation = 0;        // Current location for scrolling the LCD
unsigned long LCDTopDelay = 1;             // Delay for changing the LCD Top Text
unsigned long LCDBottomDelay = 1;          // Delay for changing the LCD Bottom Text
String LCDBottomText = "";                 // Text for the LCD Bottom
char LCDCommandTopText[17];                // Text from SDCommand handler that needs to be written to the LCD on next refresh
char LCDCommandBottomText[17];             // Text from SDCommand handler that needs to be written to the LCD on next refresh
bool LCDWriteCommandNeeded = false;        // Do we have text from SDCommand that needs to be written?
byte BootDiskExists = 0;                   // Flag for Boot Disk
byte BootDiskMounted = 0;                  // Flag for Mounted Boot Disk
byte BootDiskEnabled = 0;                  // Flag to enable the boot disk
unsigned int BootDiskIndex = 0;            // The Fat32 index number for the boot disk.
unsigned long WantedBlock = 0x00000000;    // The wanted block number
byte WantedDevice = 0x00;                  // The wanted device for the wanted block
byte LoadBufferArrayFlag = 0;              // Flag for the main loop to load the buffer with the wanted block
byte SaveBufferArrayFlag = 0;              // Flag for the main loop to save the buffer in the wanted block
byte ResetFlag = 0;                        // Flag for the main loop to process a reset interrupt
byte DisableNextReset = false;             // When set to true the next reset will not reset the devices.
byte DoubleReset = false;                  // Detects if we get 2 consecutive soft resets, treats the second as a hard reset.
byte RepeatKeyFlag = 0;                    // Flag to prevent repeats on certain buttons.
byte TooMany = 0;                          // Flag to indicate too many files to load in the current directory
byte SDCommandFAConfirm = 0;               // Confirmation byte for the SD Command FA
byte SDCommandF6Confirm = 0;               // Confirmation byte for the SD Command F6
int AnalogTriggerRight = 50.0*(AnalogButtonSensitivity/100.0);
int AnalogTriggerUp = 250.0*(AnalogButtonSensitivity/100.0);
int AnalogTriggerDown = 450.0*(AnalogButtonSensitivity/100.0);
int AnalogTriggerLeft = 650.0*(AnalogButtonSensitivity/100.0);
int AnalogTriggerSelect = 850.0*(AnalogButtonSensitivity/100.0);
SdFat sd;                                  // Setup SD Card
SdFile file;                               // Setup SD Card
void setup(){
  Serial.begin(1000000);
#ifdef PRO_MINI_OLED_BOARD
  lcd.begin();
  lcd.setFont(u8x8_font_pressstart2p_r);
  lcd.clear();
  lcd.draw1x2String(0,0, String(F("ADE      v")).c_str());
  lcd.draw1x2String(10,0, String(Version[0]).c_str());
  lcd.draw1x2String(11,0, ".");
  lcd.draw1x2String(12,0, String(Version[1]).c_str());
  lcd.draw1x2String(13,0, String(Version[2]).c_str());
  lcd.draw1x2String(0,2, String(F("by: Sean Myers")).c_str());
#else
  lcd.begin(16, 2);                        // Start the LCD screen
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("ADE      v"));
  lcd.print(String(Version[0]) + "." + String(Version[1]) + String(Version[2]));
  lcd.setCursor(0,1);  
  lcd.print(F("by: Sean Myers"));
#endif
  delay(2000);
  delay(StartupDelay);
  Serial.print(F("Starting: ADE v"));
  Serial.println(String(Version[0]) + "." + String(Version[1]) + String(Version[2]));
  for(int t=0; t<=3;t++){
    pinMode(StatusLed[t],OUTPUT);          // Set the Status LED's as output
    digitalWrite(StatusLed[t],StatusLEDState); // Set the Status LED  
  }
  pinMode(RightButtonPin,INPUT_PULLUP);    // Set the RightButtonPin to Input Pullup
  pinMode(UpButtonPin,INPUT_PULLUP);       // Set the UpButtonPin to Input Pullup
  pinMode(DownButtonPin,INPUT_PULLUP);     // Set the DownButtonPin to Input Pullup
  pinMode(LeftButtonPin,INPUT_PULLUP);     // Set the LeftButtonPin to Input Pullup
  pinMode(SelectButtonPin,INPUT_PULLUP);   // Set the SelectButtonPin to Input Pullup
  pinMode(SwapButtonPin,INPUT_PULLUP);     // Set the SwapButtonPin to Input Pullup
  byte InitialKeypress = ReadButtons();    // See if any buttons are being pushed
  if (InitialKeypress == 6){               // Swap Button Held - Device 5 is disabled. If Device 5 is already disabled, it is enabled
    if (EEPROM.read(5) == 1){
      Serial.println(F("Disabling Device 5")); 
      EEPROM.write(5,0);                   // Disable Device 5
      Serial.println(F("Enabling Device 4")); 
      EEPROM.write(4,1);                   // Enable Device 4 
    }
    else if (EEPROM.read(5) == 0){         // Enable all Devices for normal operation and correct any config errors that could make devices lost for good.
      Serial.println(F("Enabling all Devices"));
      EEPROM.write(4,1);                   // Enable Device 4
      EEPROM.write(5,1);                   // Enable Device 5
      EEPROM.write(6,1);                   // Enable Device 6
      EEPROM.write(7,1);                   // Enable Device 7
    }
  }
  if (InitialKeypress == 95){              // Select Button Held - Entering Configuration Mode
    Serial.println(F("Entering Configuration Mode"));
    ConfigMode();
  }
  if (InitialKeypress == 91){              // Right Button Held - Entering Voltage Test Mode
    Serial.println(F("Entering Voltage Test Mode"));
    VoltageRead();
  }
  SDCardSetup();                           // Initialize the SD card
  SDCardGetDir(1);                         // Load the root directory
  DeviceSetup();                           // Initialize the Device and Device Display
  Serial.print(F("Free SRAM: "));          // Print the amount of Free SRAM
  Serial.println(FreeMemory());
  LastCommandTime = millis();              // Setup initial time for last command in ms
  LastScrollLCD = millis();                // Setup initial time for LCD scrolling
  LastButtonTime = millis();
  TimetoByte = millis();                   // Setup initial time for Reset Timing
  pinMode(AdamNetRx,INPUT_PULLUP);         // Setup AdamNetRx to input
  pinMode(AdamNetTx, OUTPUT);              // Setup AdamNetTx to Ouput
  digitalWrite(AdamNetTx, HIGH);           // Set the AdamNetTx to High
  pinMode(AdamResetPin,INPUT_PULLUP);      // Set the Adam reset to input
  EIFR = bit (RX_INT);                      // Clear flag for interrupt (AdamNet Receive)
  EIFR = bit (RESET_INT);                      // Clear flag for interrupt (AdamNet Reset)
  attachInterrupt(digitalPinToInterrupt(AdamNetRx), CommandInterrupt, RISING);  // Setup the Receive Interrupt
  attachInterrupt(digitalPinToInterrupt(AdamResetPin), ResetInterrupt, FALLING);  // Setup the Reset Interrupt
}
void loop(){
  if (SaveBufferArrayFlag == 1){           // Is there a pending buffer save?
    digitalWrite(StatusLed[WantedDevice-4], !digitalRead(StatusLed[WantedDevice-4]));// Turn on the Status LED
    noInterrupts();                        // Disable Interrupts
    BufferSaveBlock();                     // Process the save command
    LastCommandTime = millis();            // Reset the Last Command timer
    AdamNetIdle();                         // Wait for AdamNet to go Idle
    SaveBufferArrayFlag = 0;               // Reset the flag
    EIFR = bit (RX_INT);                    // Clear flag for interrupt 2 (AdamNet Receive)
    interrupts();                          // Enable Interrupts
    digitalWrite(StatusLed[WantedDevice-4], !digitalRead(StatusLed[WantedDevice-4]));// Turn off the Status LED
  }
  if (LoadBufferArrayFlag == 1){           // Is there a pending buffer load?
    digitalWrite(StatusLed[WantedDevice-4], !digitalRead(StatusLed[WantedDevice-4]));// Turn on the Status LED
    noInterrupts();                        // Disable Interrupts
    BufferLoadBlock();                     // Process the load command
    LastCommandTime = millis();            // Reset the Last Command timer
    AdamNetIdle();                         // Wait for AdamNet to go Idle
    LoadBufferArrayFlag = 0;               // Reset the flag
    EIFR = bit (RX_INT);                    // Clear flag for interrupt 2 (AdamNet Receive)
    interrupts();                          // Enable Interrupts
    digitalWrite(StatusLed[WantedDevice-4], !digitalRead(StatusLed[WantedDevice-4]));;// Turn off the Status LED
  }
  if (ResetFlag == 1){
    ProcessReset();                        // Process the Reset
    LastCommandTime = millis();            // Reset the Last Command timer
    AdamNetIdle();                         // Wait for AdamNet to go Idle
    ResetFlag = 0;                         // Reset the flag
    EIFR = bit (RESET_INT);                    // Clear flag for any interrupts on AdamNetReset that were triggered while in the ISR
  }
  if ((millis() - LastCommandTime) >= 500){ //Has it been at least 500 ms since the drive processed a command?
    ProcessKeys();                         // Has the keypress delay passed and is there a keypress?
    LCDRefresh();
    if (AdamnetDisconnected == 1){
      attachInterrupt(digitalPinToInterrupt(AdamNetRx), CommandInterrupt, RISING);  // Setup the Receive Interrupt
      AdamnetDisconnected = 0;
    }
  }
}
