//https://www.youtube.com/watch?v=ON7neIqPC2A
/*
              RFID    NODEMCU   BLUEBOARD   RED/GREEN BOARD    5110 LCD(can't use with Nodemcu)
                        3.3v        raw       raw
              3.3v      vin      vcc      vcc
              RST       D0        D9        A1                 RST D7
              GND       GND       GND       GND                DIN D5
              MISO      D6        D12       D12                DC D6
              MOSI      D7        D11       D11                CE D8
              SCK       D5        D13       D13                CLK D4
              SDA       D4        D10       D10
  topled                D3        D3        D3
  bottom led            D2        A0        A0
  buzzer                D8        D2        D2
  config                D?        A3        A3
  status led            D1        D1        D
*/

//LIBRARIES
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>  //Library extends MFRC522.h to support RATS for ISO-14443-4 PICC.
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <DNSServer.h>
#include "pitches.h"     //if error message involving pitches.h comes up then make sure pitches.h file is in the same directory as the sketch
#include "iTimeAfricaV1.h" //comment out if using V2 (Green/Red) PCB
//#include "iTimeAfricaV2.h" //comment out if using V2 (Blue) PCB
//#include "Direct_Connection.h" //No PCB, Direct connection
#include <EEPROM.h>
#include <WiFiClientSecure.h>

//***********WIFI SSIDS & PASSWORDS*******************

//const char* ssid = "IRide_Africa2";
//const char* password = "Rideyourbike!";
//const char* ssid = "VodafoneMobileWiFi-AEB532"; //Connect to Mobile Wifi Unit 1 49.79MB (0.1MB per 4 read?)
//const char* password = "3626171564";
//const char* ssid = "VodafoneMobileWiFi-A18988"; //Connect to Mobile Wifi Unit 1 49.79MB (0.1MB per 4 read?)
//const char* password = "2424922092";
const char* ssid = "itimeafricaupload"; //Connect to Phone Wifi Hotspot
const char* password = "itimeafricaupload";

//POD NAME HQ, Intermediate1/Intermediate2 etc  NO SPACES!
const String id = "hq"; // Name of Upload POD any name without spaces and special characters or else server will not be found!

//https://script.google.com/a/irideafrica.com/macros/s/AKfycbwJNLSHBI5jlGY5xDybQPKSnwc7GPFbyuC29AoB/exec
//https://script.google.com/macros/s/AKfycbwC3Bc_g4UStz1MN77_YgVzKUjUwAgGONyveRoHwjG2CdnjRa0/exec
//https://script.google.com/macros/s/AKfycbwxJnqwWPcjRVNCwSx6VWAy82my3On3bbji5wE6svoNLWXu_YbjOT1reGqtCOazw329/exec
String GOOGLE_SCRIPT_ID = "AKfycbw6A7h3FqeRhUOE6yqRZNKSNdY1t4_2IAx7GEnZWOU1-SweWep9heuh9StR_ERf_hTtXA"; //this is the script ID from the "iTime Africa Scripts-Race Day Results" sheet above when it is deployed as a webapp

// RFID COMPONENT Create MFRC522 instance
MFRC522 mfrc522(RFID_CS, RFID_RST);                          // PH pins from PCB header file
MFRC522::MIFARE_Key key;

//WEB/WIFI/SCRIPTS COMPONENT
WiFiServer server(80); // Set web server port number to 80
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint  = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6"; // for https
WiFiClientSecure client;
WiFiEventHandler wifiConnectHandler;  
WiFiEventHandler wifiDisconnectHandler;

// variables used for converting epoch time to readable time
unsigned long temp0 = 0, temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0,
              temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0, temp9 = 0,
              temp10 = 0, hours, mins, secs, MilliS;

//epoch time storage variables
unsigned long ss1Start = 0, ss1Finish = 0, SS1Time = 0,
              ss2Start = 0, ss2Finish = 0, SS2Time = 0,
              ss3Start = 0, ss3Finish = 0, SS3Time = 0,
              ss4Start = 0, ss4Finish = 0, SS4Time = 0,
              ss5Start = 0, ss5Finish = 0, SS5Time = 0,
              ss6Start = 0, ss6Finish = 0, SS6Time = 0,
              ss7Start = 0, ss7Finish = 0, SS7Time = 0,
              ss8Start = 0, ss8Finish = 0, SS8Time = 0,
              SS1TimeMilliS = 0, SS2TimeMilliS = 0, SS3TimeMilliS = 0,
              SS4TimeMilliS = 0, SS5TimeMilliS = 0, SS6TimeMilliS = 0,
              SS7TimeMilliS = 0, SS8TimeMilliS = 0,
              totalRaceTime = 0;

byte readCard[4];  //is this needed?
byte buffer[18];

char ss1ST[17];                               //declare character array for ss1 Start Time
char ss1FT[17];                               //declare character array for ss1 Finish Time
char ss2ST[17];                               //declare character array for ss2 Start Time
char ss2FT[17];                               //declare character array for ss2 Finish Time
char ss3ST[17];                               //declare character array for ss3 Start Time
char ss3FT[17];                               //declare character array for ss3 Finish Time
char ss4ST[17];                               //declare character array for ss4 Start Time
char ss4FT[17];                               //declare character array for ss4 Finish Time
char ss5ST[17];                               //declare character array for ss5 Start Time
char ss5FT[17];                               //declare character array for ss5 Finish Time
char ss6ST[17];                               //declare character array for ss6 Start Time
char ss6FT[17];                               //declare character array for ss6 Finish Time
char ss7ST[17];                               //declare character array for ss7 Start Time
char ss7FT[17];                               //declare character array for ss7 Finish Time
char ss8ST[17];                               //declare character array for ss8 Start Time
char ss8FT[17];                               //declare character array for ss8 Finish Time

// Variables will change:
char myRaceno[17];

void setup() {

  //Register event handlers
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  Wire.begin();           // Init I2C buss (not needed?)
  Serial.begin(115200);   // PH to use status LED serial needs to be disbaled
  initIoPins();
  initWiFi();
  initMfrc522();
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  ConnectedBeep();
  delay(1000);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {

  Serial.println("Connected to Wi-Fi sucessfully.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BOT, LOW);
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {

  Serial.println("Disconnected from Wi-Fi, trying to connect...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  digitalWrite(LED_BOT, HIGH);
}

// All Genral Purpouse IO pin Setup stuffs
void initIoPins() {
  // Input Pins
      //No input pins

  // Output Pins
  pinMode(LED_TOP, OUTPUT);
  pinMode(LED_BOT, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  // set pins start condition: LOW = LED on, HIGH = LED off
  digitalWrite(LED_TOP, HIGH);
  digitalWrite(LED_BOT, HIGH);
  digitalWrite(STATUS_LED, LOW);   //can't do anything with status led in v1 board dan
}

// All RFID init Stuffs
void initMfrc522() {
  SPI.begin();                                          // Init SPI bus
  mfrc522.PCD_Init();                                   // Init MFRC522 card
  //  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);       // Enhance the MFRC522 Receiver Gain to maximum value of some 48 dB (default is 33dB) turn off if scan doesn't seem to work (too powerful to read close up)
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  Serial.println(F("*** READY ***"));    //shows in serial that it is ready to read
  for (byte i = 0; i < 6; i++) {                // Prepare the key (used both as key A and as key B) using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    key.keyByte[i] = 0xFF;
  }
}

void ConnectedBeep() {
  int notecount = 4;
  int melody[] = {NOTE_D7, NOTE_E7, NOTE_F7, NOTE_G7};
  int noteDurations[] = {10, 8, 6, 4};                        // note durations 4 = quarter note, 8 = eighth note, etc.::
  beepsLights ( notecount, melody, noteDurations );
}

void scannedBeep() {
  int notecount = 3;
  int melody[] = {NOTE_G7, NOTE_G7, NOTE_G7};        // these are the only clear notes with buzzer used!
  int noteDurations[] = {12, 12, 12};                        // note durations 4 = quarter note, 8 = eighth note, etc.::
  beepsLights ( notecount, melody, noteDurations );
  digitalWrite(LED_BOT, HIGH);
}

void SuccessfulUploadBeep() {
  int notecount = 2;
  int melody[] = {NOTE_C7, NOTE_G7};
  int noteDurations[] = {12, 8};                        // note durations 4 = quarter note, 8 = eighth note, etc.::
  beepsLights ( notecount, melody, noteDurations );
}

// Beeps and flash leds for succesfull scan
void beepsLights(int notecount, int melody[], int noteDurations[]  )    {
  for (int thisNote = 0; thisNote < notecount; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    tone(BUZZER, melody[thisNote], noteDuration);
    digitalWrite(LED_TOP, LOW);                             // turn  LED on
    //      digitalWrite(LED_BOT, LOW);                             // turn  LED on

    int pauseBetweenNotes = noteDuration * 1.30;            // the note's duration + 30% seems to work well:
    delay(pauseBetweenNotes);

    noTone(BUZZER);                                         // stop the tone playing:
    digitalWrite(LED_TOP, HIGH);
    //      digitalWrite(LED_BOT, HIGH);
  }
}

void loop() {
  //  updateOLED();  //can't add in OLED not enough digital pins for other stuffs so have to use D1 and D2
  if ( ! mfrc522.PICC_IsNewCardPresent())     // Look for new cards
    return;

  if ( ! mfrc522.PICC_ReadCardSerial())       // Select one of the cards
    return;
  readCardData();                         // Card is present and readble: invoke readCardData function

  mfrc522.PICC_HaltA();                       // Halt PICC
  mfrc522.PCD_StopCrypto1();                  // Stop encryption on PCD
}


void readCardData()
{

  byte size = sizeof(buffer);
  byte status;
  byte block;

  String string0;
  String string1;
  String string2;
  String string3;
  String string4;
  String string5;
  String string6;
  String string7;
  String string8;
  String string9;
  String string10;
  String string11;
  String string12;
  String string13;
  String string14;
  String string15;
  String fullstring;

  char ss1Time[17] = "DNS/DNF";                           //declare character array for ss1 Calculated Time and zero it for each scan
  char ss2Time[17] = "DNS/DNF";                           //declare character array for ss2 Calculated Time and zero it for each scan
  char ss3Time[17] = "DNS/DNF";                           //declare character array for ss3 Calculated Time and zero it for each scan
  char ss4Time[17] = "DNS/DNF";                           //declare character array for ss4 Calculated Time and zero it for each scan
  char ss5Time[17] = "DNS/DNF";                           //declare character array for ss5 Calculated Time and zero it for each scan
  char ss6Time[17] = "DNS/DNF";                           //declare character array for ss6 Calculated Time and zero it for each scan
  char ss7Time[17] = "DNS/DNF";                           //declare character array for ss7 Calculated Time and zero it for each scan
  char ss8Time[17] = "DNS/DNF";                           //declare character array for ss8 Calculated Time and zero it for each scan

  block = 4;

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //     Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  string0 = char(buffer[0]);
  string1 = char(buffer[1]);
  string2 = char(buffer[2]);
  string3 = char(buffer[3]);
  string4 = char(buffer[4]);
  string5 = char(buffer[5]);
  string6 = char(buffer[6]);
  string7 = char(buffer[7]);
  string8 = char(buffer[8]);
  string9 = char(buffer[9]);
  string10 = char(buffer[10]);
  string11 = char(buffer[11]);
  string12 = char(buffer[12]);
  string13 = char(buffer[13]);
  string14 = char(buffer[14]);
  string15 = char(buffer[15]);

  fullstring = (string0 + string1 + string2 + string3 + string4 + string5 + string6 + string7 + string8 + string9 + string10 + string11 + string12 + string13 + string14 + string15);


  fullstring.toCharArray(myRaceno, 17);                                   // copy fullstring content into myString
  Serial.print(F("Race Number:"));
  Serial.println(myRaceno);

  /*
     STAGE 1 START TIME  // reads SS1 Start Time from PICC  block 8 and stores in buffer
  */

  block = 8;

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //        Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store( still need to write to excel cell)


  buffer2epoch();
  ss1Start = temp10;


  /*
     STAGE 1 FINISH TIME
     // reads SS1 Finish Time from PICC  block 9, then subtracts ss1 finish from ss1 start and calculates SS1 Time
     // and then downloads SS1 Time to Excel cells
  */

  block = 9;

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss1Finish = temp10;

  writeSS1rawData();

  if (ss1Start > ss1Finish && ss1Finish != 0)                                       // if ss1Start bigger than ss1Finish and SS1Finish is not equal to zero, then write 'error'
  {
    SS1TimeMilliS = 0;                                                            // set stage time to zero for total count
    char ss1Time[17] = "ERROR";             //populates google sheet with error text (tagged end before start)
    Serial.print("SS1 Time: ");
    Serial.println(ss1Time);
  }
  else if (ss1Start != 0 && ss1Finish != 0)
  {
    SS1TimeMilliS = ss1Finish - ss1Start;
    MilliS = SS1TimeMilliS % 1000;
    SS1Time = (SS1TimeMilliS - MilliS) / 1000;
    hours = (SS1Time / 60 / 60);
    mins = (SS1Time - (hours * 60 * 60)) / 60;
    secs = SS1Time - (hours * 60 * 60) - (mins * 60);

    String timestamp;

    if (hours < 10) {
      timestamp += "0";
    }
    timestamp += String(hours);
    timestamp += ":";
    if (mins < 10) {
      timestamp += "0";
    }
    timestamp += String(mins);
    timestamp += ":";
    if (secs < 10) {
      timestamp += "0";
    }
    timestamp += String(secs);
    timestamp += ".";
    if (MilliS  < 10) {
      timestamp += "00";
      timestamp += String(MilliS);
    }
    else if (MilliS > 9 && MilliS < 100) {
      timestamp += "0";
      timestamp += String(MilliS);
    }
    else timestamp += String(MilliS);
    timestamp.toCharArray(ss1Time, 17);                          // copy fullstring content into myString
    Serial.print("SS1 Time: ");
    Serial.println(ss1Time);
  }
  else
  {
    SS1TimeMilliS = 0;                                       // set stage time to zero for total count
    char ss1Time17[] = "DNS/DNF";                            //populates google sheet with DNS/DNF text
    Serial.print("SS1 Time: ");
    Serial.println(ss1Time);
  }
  /*
     STAGE 2 START TIME // reads SS2 Start Time from PICC  block 12 and stores in buffer
  */

  block = 12;
  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //        Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss2Start = temp10;

  /*
     STAGE 2 FINISH TIME
     // reads SS2 Finish Time from PICC  block 13, then subtracts ss2 finish from ss2 start and calculates SS2 Time
     // and then downloads SS2 Time to Excel cells
  */

  block = 13;

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //        Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss2Finish = temp10;

  writeSS2rawData();

  if (ss2Start > ss2Finish && ss2Finish != 0)                                       // if ss2Start bigger than ss2Finish and SS2Finish is not equal to zero, then write 'error'
  {

    SS2TimeMilliS = 0;                                                            // set stage time to zero for total count
    char ss2Time[17] = "ERROR"; //populates google sheet with error text (tagged end before start)
    Serial.print("SS2 Time: ");
    Serial.println(ss2Time);
  }
  else if (ss2Start != 0 && ss2Finish != 0)
  {
    SS2TimeMilliS = ss2Finish - ss2Start;
    MilliS = SS2TimeMilliS % 1000;
    SS2Time = (SS2TimeMilliS - MilliS) / 1000;
    hours = (SS2Time / 60 / 60);
    mins = (SS2Time - (hours * 60 * 60)) / 60;
    secs = SS2Time - (hours * 60 * 60) - (mins * 60);


    String timestamp;

    if (hours < 10) {
      timestamp += "0";
    }
    timestamp += String(hours);
    timestamp += ":";
    if (mins < 10) {
      timestamp += "0";
    }
    timestamp += String(mins);
    timestamp += ":";
    if (secs < 10) {
      timestamp += "0";
    }
    timestamp += String(secs);
    timestamp += ".";
    if (MilliS  < 10) {
      timestamp += "00";
      timestamp += String(MilliS);
    }

    else if (MilliS > 9 && MilliS < 100) {
      timestamp += "0";
      timestamp += String(MilliS);
    }
    else timestamp += String(MilliS);

    //           char ss2Time[17];   //already defined
    timestamp.toCharArray(ss2Time, 17);                                      // copy fullstring content into myString
    Serial.print("SS2 Time: ");
    Serial.println(ss2Time);

  }
  else
  {

    SS2TimeMilliS = 0;             // set stage time to zero for total count
    char ss2Time17[] = "DNS/DNF";                            //populates google sheet with DNS/DNF text
    Serial.print("SS2 Time: ");
    Serial.println(ss2Time);
  }

  /*
     STAGE 3 START TIME  // reads SS3 Start Time from PICC  block 16 and stores in buffer
  */

  block = 16;  // Stage 3 Start time

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss3Start = temp10;

  /*
     STAGE 3 FINISH TIME
     // reads SS3 Finish Time from PICC  block 17, then subtracts ss1 finish from ss3 start and calculates SS3 Time
     // and then downloads SS3 Time to Excel cells
  */

  block = 17;  // Stage 3 Finish time

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //        Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss3Finish = temp10;

  writeSS3rawData();

  if (ss3Start > ss3Finish && ss3Finish != 0)                                       // if ss3Start bigger than ss3Finish and SS3Finish is not equal to zero, then write 'error'
  {
    SS3TimeMilliS = 0;
    char ss3Time[17] = "ERROR";             //populates google sheet with error text (tagged end before start)
    Serial.print("SS3 Time: ");
    Serial.println(ss3Time);// set stage time to zero for total count
  }
  else

    if (ss3Start != 0 && ss3Finish != 0)
    {
      SS3TimeMilliS = ss3Finish - ss3Start;
      MilliS = SS3TimeMilliS % 1000;
      SS3Time = (SS3TimeMilliS - MilliS) / 1000;
      hours = (SS3Time / 60 / 60);
      mins = (SS3Time - (hours * 60 * 60)) / 60;
      secs = SS3Time - (hours * 60 * 60) - (mins * 60);


      String timestamp;

      if (hours < 10) {
        timestamp += "0";
      }
      timestamp += String(hours);
      timestamp += ":";
      if (mins < 10) {
        timestamp += "0";
      }
      timestamp += String(mins);
      timestamp += ":";
      if (secs < 10) {
        timestamp += "0";
      }
      timestamp += String(secs);
      timestamp += ".";
      if (MilliS  < 10) {
        timestamp += "00";
        timestamp += String(MilliS);
      }

      else if (MilliS > 9 && MilliS < 100) {
        timestamp += "0";
        timestamp += String(MilliS);
      }
      else timestamp += String(MilliS);

      timestamp.toCharArray(ss3Time, 17);                                    // copy fullstring content into myString
      Serial.print("SS3 Time: ");
      Serial.println(ss3Time);
    }
    else
    {

      SS3TimeMilliS = 0;                                             // set stage time to zero for total count
      char ss3Time17[] = "DNS/DNF";                            //populates google sheet with DNS/DNF text
      Serial.print("SS3 Time: ");
      Serial.println(ss3Time);
    }

  /*
     STAGE 4 START TIME  // reads SS4 Start Time from PICC  block 20 and stores in buffer
  */

  block = 20;  // Stage 4 Start time

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //        Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss4Start = temp10;

  /*
     STAGE 4 FINISH TIME
     // reads SS4 Finish Time from PICC  block 21, then subtracts ss4 finish from ss4 start and calculates SS4 Time
     // and then downloads SS4 Time to Excel cells
  */

  block = 21;   // Stage 4 Finish time

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //        Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss4Finish = temp10;

  writeSS4rawData();

  if (ss4Start > ss4Finish && ss4Finish != 0)                                       // if ss4Start bigger than ss4Finish and SS4Finish is not equal to zero, then write 'error'
  {
    SS4TimeMilliS = 0;                                                                 // set stage time to zero for total count
    char ss4Time[17] = "ERROR"; //populates google sheet with error text (tagged end before start)
    Serial.print("SS4 Time: ");
    Serial.println(ss4Time);
  }
  else

    if (ss4Start != 0 && ss4Finish != 0)
    {
      SS4TimeMilliS = ss4Finish - ss4Start;
      MilliS = SS4TimeMilliS % 1000;
      SS4Time = (SS4TimeMilliS - MilliS) / 1000;
      hours = (SS4Time / 60 / 60);
      mins = (SS4Time - (hours * 60 * 60)) / 60;
      secs = SS4Time - (hours * 60 * 60) - (mins * 60);

      String timestamp;

      if (hours < 10) {
        timestamp += "0";
      }
      timestamp += String(hours);
      timestamp += ":";
      if (mins < 10) {
        timestamp += "0";
      }
      timestamp += String(mins);
      timestamp += ":";
      if (secs < 10) {
        timestamp += "0";
      }
      timestamp += String(secs);
      timestamp += ".";
      if (MilliS  < 10) {
        timestamp += "00";
        timestamp += String(MilliS);
      }

      else if (MilliS > 9 && MilliS < 100) {
        timestamp += "0";
        timestamp += String(MilliS);
      }
      else timestamp += String(MilliS);

      timestamp.toCharArray(ss4Time, 17);                                    // copy fullstring content into myString
      Serial.print("SS4 Time: ");
      Serial.println(ss4Time);

    }
    else
    {

      SS4TimeMilliS = 0;                                                          // set stage time to zero for total count
      char ss4Time17[] = "DNF/DNS";                                     //populates google sheet with DNS/DNF text
      Serial.print("SS4 Time: ");
      Serial.println(ss4Time);
    }

  /*
     STAGE 5 START TIME  // reads SS5 Start Time from PICC  block 24 and stores in buffer
  */

  block = 24;  // Stage 5 Start time

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //        Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss5Start = temp10;

  /*
     STAGE 5 FINISH TIME
     // reads SS5 Finish Time from PICC  block 25, then subtracts ss5 finish from ss5 start and calculates SS5 Time
     // and then downloads SS5 Time to Excel cells
  */

  block = 25;     // Stage 5 Finish time

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //        Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss5Finish = temp10;

  writeSS5rawData();

  if (ss5Start > ss5Finish && ss5Finish != 0)                                       // if ss5Start bigger than ss5Finish and SS5Finish is not equal to zero, then write 'error'
  {
    SS5TimeMilliS = 0;                                                                 // set stage time to zero for total count
    char ss5Time[17] = "ERROR"; //populates google sheet with error text (tagged end before start)
    Serial.print("SS5 Time: ");
    Serial.println(ss5Time);
  }
  else

    if (ss5Start != 0 && ss5Finish != 0)
    {
      SS5TimeMilliS = ss5Finish - ss5Start;
      MilliS = SS5TimeMilliS % 1000;
      SS5Time = (SS5TimeMilliS - MilliS) / 1000;
      hours = (SS5Time / 60 / 60);
      mins = (SS5Time - (hours * 60 * 60)) / 60;
      secs = SS5Time - (hours * 60 * 60) - (mins * 60);

      String timestamp;

      if (hours < 10) {
        timestamp += "0";
      }
      timestamp += String(hours);
      timestamp += ":";
      if (mins < 10) {
        timestamp += "0";
      }
      timestamp += String(mins);
      timestamp += ":";
      if (secs < 10) {
        timestamp += "0";
      }
      timestamp += String(secs);
      timestamp += ".";
      if (MilliS  < 10) {
        timestamp += "00";
        timestamp += String(MilliS);
      }

      else if (MilliS > 9 && MilliS < 100) {
        timestamp += "0";
        timestamp += String(MilliS);
      }
      else timestamp += String(MilliS);


      timestamp.toCharArray(ss5Time, 17);                        // copy fullstring content into myString
      Serial.print("SS5 Time: ");
      Serial.println(ss5Time);


    }
    else
    {

      SS5TimeMilliS = 0;                                      // set stage time to zero for total count
      char ss5Time17[] = "DNF/DNS";                          //populates google sheet with DNS/DNF text
      Serial.print("SS5 Time: ");
      Serial.println(ss5Time);
    }

  /*
     STAGE 6 START TIME  // reads SS6 Start Time from PICC  block 28 and stores in buffer
  */

  block = 28;

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    //        Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store( still need to write to excel cell)


  buffer2epoch();
  ss6Start = temp10;


  /*
     STAGE 6 FINISH TIME
     // reads SS6 Finish Time from PICC  block 29, then subtracts ss6 finish from ss6 start and calculates SS16Time
     // and then downloads SS6 Time to Excel cells
  */

  block = 29;

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss6Finish = temp10;

  writeSS6rawData();

  if (ss6Start > ss6Finish && ss6Finish != 0)                                       // if ss6Start bigger than ss6Finish and SS6Finish is not equal to zero, then write 'error'
  {
    SS6TimeMilliS = 0;                                                            // set stage time to zero for total count
    char ss6Time[17] = "ERROR"; //populates google sheet with error text (tagged end before start)
    Serial.print("SS6 Time: ");
    Serial.println(ss6Time);
  }
  else if (ss6Start != 0 && ss6Finish != 0)
  {
    SS6TimeMilliS = ss6Finish - ss6Start;
    MilliS = SS6TimeMilliS % 1000;
    SS6Time = (SS6TimeMilliS - MilliS) / 1000;
    hours = (SS6Time / 60 / 60);
    mins = (SS6Time - (hours * 60 * 60)) / 60;
    secs = SS6Time - (hours * 60 * 60) - (mins * 60);


    String timestamp;

    if (hours < 10) {
      timestamp += "0";
    }
    timestamp += String(hours);
    timestamp += ":";
    if (mins < 10) {
      timestamp += "0";
    }
    timestamp += String(mins);
    timestamp += ":";
    if (secs < 10) {
      timestamp += "0";
    }
    timestamp += String(secs);
    timestamp += ".";
    if (MilliS < 10) {
      timestamp += "00";
      timestamp += String(MilliS);
    }

    else if (MilliS > 9 && MilliS < 100) {
      timestamp += "0";
      timestamp += String(MilliS);
    }
    else timestamp += String(MilliS);
    timestamp.toCharArray(ss6Time, 17);                                    // copy fullstring content into myString
    Serial.print("SS6 Time: ");
    Serial.println(ss6Time);
  }
  else
  {
    SS6TimeMilliS = 0;                                                          // set stage time to zero for total count
    char ss6Time17[] = "DNS/DNF";                            //populates google sheet with DNS/DNF text
    Serial.print("SS6 Time: ");
    Serial.println(ss6Time);
  }

  /*
     STAGE 7 START TIME  // reads SS7 Start Time from PICC 32 block  and stores in buffer
  */

  block = 32;

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss7Start = temp10;

  /*
     STAGE 7 FINISH TIME
     // reads SS7 Finish Time from PICC  block 33, then subtracts ss6 finish from ss6 start and calculates SS6 Time
     // and then downloads SS6 Time to Excel cells
  */

  block = 33;

  /// Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss7Finish = temp10;

  writeSS7rawData();

  if (ss7Start > ss7Finish && ss7Finish != 0)                                       // if ss7Start bigger than ss7Finish and SS7Finish is not equal to zero, then write 'error'
  {

    SS7TimeMilliS = 0;                                                             // set stage time to zero for total count
    char ss7Time[17] = "ERROR"; //populates google sheet with error text (tagged end before start)
    Serial.print("SS7 Time: ");
    Serial.println(ss7Time);
  }
  if (ss7Start != 0 && ss7Finish != 0)
  {
    SS7TimeMilliS = ss7Finish - ss7Start;
    MilliS = SS7TimeMilliS % 1000;
    SS7Time = (SS7TimeMilliS - MilliS) / 1000;
    hours = (SS7Time / 60 / 60);
    mins = (SS7Time - (hours * 60 * 60)) / 60;
    secs = SS7Time - (hours * 60 * 60) - (mins * 60);

    String timestamp;

    if (hours < 10) {
      timestamp += "0";
    }
    timestamp += String(hours);
    timestamp += ":";
    if (mins < 10) {
      timestamp += "0";
    }
    timestamp += String(mins);
    timestamp += ":";
    if (secs < 10) {
      timestamp += "0";
    }
    timestamp += String(secs);
    timestamp += ".";
    if (MilliS < 10) {
      timestamp += "00";
      timestamp += String(MilliS);
    }
    else if (MilliS > 9 && MilliS < 100) {
      timestamp += "0";
      timestamp += String(MilliS);
    }
    else timestamp += String(MilliS);
    timestamp.toCharArray(ss7Time, 17);                                    // copy fullstring content into myString
    Serial.print("SS7 Time: ");
    Serial.println(ss7Time);
  }
  else
  {

    SS7TimeMilliS = 0;                                                          // set stage time to zero for total count
    char ss7Time17[] = "DNS/DNF";                            //populates google sheet with DNS/DNF text
    Serial.print("SS7 Time: ");
    Serial.println(ss7Time);
  }

  /*
     STAGE 8 START TIME  // reads SS8 Start Time from PICC 36 block  and stores in buffer
  */

  block = 36;

  // Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss8Start = temp10;

  /*
     STAGE 8 FINISH TIME
     // reads SS87 Finish Time from PICC  block 37, then subtracts ss6 finish from ss6 start and calculates SS6 Time
     // and then downloads SS6 Time to Excel cells
  */

  block = 37;

  /// Authentication

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    return;
  }

  // Read data from the block into buffer

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  { Serial.println(F("Download Failed"));
    return;
  }

  // Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss8Finish = temp10;

  writeSS8rawData();

  if (ss8Start > ss8Finish && ss8Finish != 0)                                       // if ss8Start bigger than ss8Finish and SS8Finish is not equal to zero, then write 'error'
  {

    SS8TimeMilliS = 0;                                                             // set stage time to zero for total count
    char ss8Time[17] = "ERROR"; //populates google sheet with error text (tagged end before start)
    Serial.print("SS8 Time: ");
    Serial.println(ss8Time);
  }
  if (ss8Start != 0 && ss8Finish != 0)
  {
    SS8TimeMilliS = ss8Finish - ss8Start;
    MilliS = SS8TimeMilliS % 1000;
    SS8Time = (SS8TimeMilliS - MilliS) / 1000;
    hours = (SS8Time / 60 / 60);
    mins = (SS8Time - (hours * 60 * 60)) / 60;
    secs = SS8Time - (hours * 60 * 60) - (mins * 60);


    String timestamp;

    if (hours < 10) {
      timestamp += "0";
    }
    timestamp += String(hours);
    timestamp += ":";
    if (mins < 10) {
      timestamp += "0";
    }
    timestamp += String(mins);
    timestamp += ":";
    if (secs < 10) {
      timestamp += "0";
    }
    timestamp += String(secs);
    timestamp += ".";
    if (MilliS < 10) {
      timestamp += "00";
      timestamp += String(MilliS);
    }
    else if (MilliS > 9 && MilliS < 100) {
      timestamp += "0";
      timestamp += String(MilliS);
    }
    else timestamp += String(MilliS);
    timestamp.toCharArray(ss8Time, 17);                                    // copy fullstring content into myString
    Serial.print("SS8 Time: ");
    Serial.println(ss8Time);
  }
  else
  {

    SS8TimeMilliS = 0;                                                          // set stage time to zero for total count
    char ss8Time17[] = "DNS/DNF";                            //populates google sheet with DNS/DNF text
    Serial.print("SS8 Time: ");
    Serial.println(ss8Time);
  }

  /*
     TOTAL TIME
     Add all stage times and print the total to Google Sheets using google scripts (called 'rfid') to import data
  */

  char totalTime[17];
  totalRaceTime = SS1TimeMilliS + SS2TimeMilliS + SS3TimeMilliS + SS4TimeMilliS + SS5TimeMilliS + SS6TimeMilliS + SS7TimeMilliS + SS8TimeMilliS;
  MilliS = totalRaceTime % 1000;                                            //storing remainder of totalracetime in milliseconds
  totalRaceTime = (totalRaceTime - MilliS) / 1000;                         //converting totalracetime to whole seconds
  hours = (totalRaceTime / (60 * 60));
  mins = (totalRaceTime - (hours * 60 * 60)) / 60;
  secs = totalRaceTime - (hours * 60 * 60) - (mins * 60);

  String timestamp;

  if (hours < 10) {
    timestamp += "0";
  }
  timestamp += String(hours);
  timestamp += ":";
  if (mins < 10) {
    timestamp += "0";
  }
  timestamp += String(mins);
  timestamp += ":";
  if (secs < 10) {
    timestamp += "0";
  }
  timestamp += String(secs);
  timestamp += ".";
  if (MilliS < 10) {           //adding leading 00 for milliseconds
    timestamp += "00";
    timestamp += String(MilliS);
  }

  else if (MilliS > 9 && MilliS < 100) {
    timestamp += "0";                     //adding leading 0 for milliseconds
    timestamp += String(MilliS);
  }
  else timestamp += String(MilliS);       // no need to add leading zeros


  timestamp.toCharArray(totalTime, 17);                                    // copy fullstring content into myString
  Serial.print("Total Race Time: ");
  Serial.println(totalTime);

  /*
     NUMBER OF STAGES
     Count number of succesful stages and add up
  */
  int StageCount = 0;

  if (SS1TimeMilliS != 0) {                                       // if ss1Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
  }
  else {
    StageCount = StageCount + 0;                                                 // set stage time to zero for total count
  }
  if (SS2TimeMilliS != 0) {                                       // if ss2Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
  }
  else {
    StageCount = StageCount + 0;                                                 // set stage time to zero for total count
  }
  if (SS3TimeMilliS != 0) {                                       // if ss3Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
  }
  else {
    StageCount = StageCount + 0;                                                 // set stage time to zero for total count
  }
  if (SS4TimeMilliS != 0) {                                       // if ss4Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
  }
  else {
    StageCount = StageCount + 0;                                                 // set stage time to zero for total count
  }
  if (SS5TimeMilliS != 0) {                                       // if ss5Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
  }
  else {
    StageCount = StageCount + 0;                                                 // set stage time to zero for total count
  }
  if (SS6TimeMilliS != 0) {                                       // if ss6Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
  }
  else {
    StageCount = StageCount + 0;                                                 // set stage time to zero for total count
  }
  if (SS7TimeMilliS != 0) {                                       // if ss7Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
  }
  else {
    StageCount = StageCount + 0;                                                 // set stage time to zero for total count
  }
  if (SS8TimeMilliS != 0) {                                       // if ss8Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
  }
  else {
    StageCount = StageCount + 0;                                                 // set stage time to zero for total count
  }
  Serial.print("Stage Count: ");
  Serial.print(StageCount);
  Serial.print("\t") ;             //tab separator
  Serial.println();
  scannedBeep();

  /*
    POST to GOGGLE SHEETS
    Put all block data into a string in http format
  */
  String data = sendData("id=" + id + "&uid=" + myRaceno + "&stages=" + StageCount + "&stage1time=" + ss1Time + "&stage2time=" + ss2Time + "&stage3time=" + ss3Time + "&stage4time=" + ss4Time
                         + "&stage5time=" + ss5Time + "&stage6time=" + ss6Time + "&stage7time=" + ss7Time + "&stage8time=" + ss8Time  + "&totaltime=" + totalTime +  "&ss1startraw=" + ss1ST +
                         "&ss1endraw=" + ss1FT +  "&ss2startraw=" + ss2ST + "&ss2endraw=" + ss2FT +  "&ss3startraw=" + ss3ST + "&ss3endraw=" + ss3FT +  "&ss4startraw=" + ss4ST + "&ss4endraw=" + ss4FT +
                         "&ss5startraw=" + ss5ST + "&ss5endraw=" + ss5FT +  "&ss6startraw=" + ss6ST + "&ss6endraw=" + ss6FT +  "&ss7startraw=" + ss7ST + "&ss7endraw=" + ss7FT +  "&ss8startraw=" + ss8ST
                         + "&ss8endraw=" + ss8FT, NULL); // names in inverted commas link to the variables in  script code called "rfid"
  // HandleDataFromGoogle(data);

  digitalWrite(LED_BOT, LOW); //turn bottom led on to signify ready to scan again
}


void buffer2epoch() {
  //  byte size = sizeof(buffer);       // byte size = number of digits in the buffer
  temp0 = buffer[0] - '0';        //writes 1st digit of epoch time from buffer to temp0
  temp1 = buffer[1] - '0';        //writes 2nd digit of epoch time from buffer to temp1
  temp2 = buffer[2] - '0';        //writes 3rd digit of epoch time from buffer to temp2
  temp3 = buffer[3] - '0';        //writes 4th digit of epoch time from buffer to temp3
  temp4 = buffer[4] - '0';        //writes 5th digit of epoch time from buffer to temp4
  temp5 = buffer[5] - '0';        //writes 6th digit of epoch time from buffer to temp5
  temp6 = buffer[6] - '0';        //writes 7th digit of epoch time from buffer to temp6
  temp7 = buffer[7] - '0';        //writes 8th digit of epoch time from buffer to temp7
  temp8 = buffer[8] - '0';        //writes 9th digit of epoch time from buffer to temp8
  temp9 = buffer[9] - '0';        //writes 10th digit of epoch time from buffer to temp9
  // use this for times with 7 digits. ie times written to watches between 00h00 and 02h42 include only hours,minutes, seconds and millis (initAfricaTimer()function in Timing POD code baseTimeS definition)
  //        temp10 = ((temp0*1000000) + (temp1*100000) + (temp2*10000) + (temp3*1000) + (temp4*100) + (temp5*10) + temp6);
  // use this for times with 8 digits. ie times written to watches  include only hours,minutes, seconds and millis (initAfricaTimer()function in Timing POD code baseTimeS definition)
  temp10 = ((temp0 * 10000000) + (temp1 * 1000000) + (temp2 * 100000) + (temp3 * 10000) + (temp4 * 1000) + (temp5 * 100) + (temp6 * 10) + temp7);
  // use this for times with 9 digits. ie times written to watches  include day,hours,minutes, seconds and millis (initAfricaTimer()function in Timing POD code baseTimeS definition)
  //        temp10 = ((temp0*100000000) + (temp1*10000000) + (temp2*1000000) + (temp3*100000) + (temp4*10000) + (temp5*1000) + (temp6*100) + (temp7*10) + temp8);
}

void writeSS1rawData()
{
  String ss1StartString;
  String ss1FinishString;
  ss1StartString = String(ss1Start);           //converts ss1Start epoch time to string type and store in ss1StartString
  ss1FinishString = String(ss1Finish);         //converts ss1Finish epoch time to string type and store in ss1FinishString
  ss1StartString.toCharArray(ss1ST, 17);                           //convert ss1StartString to charater array ss1ST
  ss1FinishString.toCharArray(ss1FT, 17);                          //convert ss1FinishString to charater array ss1FT

  Serial.print(F("SS1 Start Time:"));
  Serial.println(ss1ST);
  Serial.print(F("SS1 End Time:"));
  Serial.println(ss1FT);

}

void writeSS2rawData()
{
  String ss2StartString;
  String ss2FinishString;
  ss2StartString = String(ss2Start);           //converts ss2Start epoch time to string type and store in ss1StartString
  ss2FinishString = String(ss2Finish);         //converts ss2Finish epoch time to string type and store in ss1FinishString
  ss2StartString.toCharArray(ss2ST, 17);                           //convert ss2StartString to charater array ss2ST
  ss2FinishString.toCharArray(ss2FT, 17);                          //convert ss2FinishString to charater array ss2FT

  Serial.print(F("SS2 Start Time:"));
  Serial.println(ss2ST);
  Serial.print(F("SS2 End Time:"));
  Serial.println(ss2FT);
}

void writeSS3rawData()
{
  String ss3StartString;
  String ss3FinishString;
  ss3StartString += String(ss3Start);           //converts ss3Start epoch time to string type and store in ss3StartString
  ss3FinishString += String(ss3Finish);         //converts ss3Finish epoch time to string type and store in ss3FinishString
  ss3StartString.toCharArray(ss3ST, 17);                           //convert ss3StartString to charater array ss3ST
  ss3FinishString.toCharArray(ss3FT, 17);                          //convert ss3FinishString to charater array ss3FT

  Serial.print(F("SS3 Start Time:"));
  Serial.println(ss3ST);
  Serial.print(F("SS3 End Time:"));
  Serial.println(ss3FT);
}

void writeSS4rawData()
{
  String ss4StartString;
  String ss4FinishString;
  ss4StartString += String(ss4Start);           //converts ss4Start epoch time to string type and store in ss1StartString
  ss4FinishString += String(ss4Finish);         //converts ss4Finish epoch time to string type and store in ss1FinishString
  ss4StartString.toCharArray(ss4ST, 17);                           //convert ss4StartString to charater array ss4ST
  ss4FinishString.toCharArray(ss4FT, 17);                          //convert ss4FinishString to charater array ss4FT

  Serial.print(F("SS4 Start Time:"));
  Serial.println(ss4ST);
  Serial.print(F("SS4 End Time:"));
  Serial.println(ss4FT);
}

void writeSS5rawData()
{
  String ss5StartString;
  String ss5FinishString;
  ss5StartString += String(ss5Start);           //converts ss5Start epoch time to string type and store in ss1StartString
  ss5FinishString += String(ss5Finish);         //converts ss5Finish epoch time to string type and store in ss1FinishString
  ss5StartString.toCharArray(ss5ST, 17);                           //convert ss5StartString to charater array ss5ST
  ss5FinishString.toCharArray(ss5FT, 17);                          //convert ss5FinishString to charater array ss5FT

  Serial.print(F("SS5 Start Time:"));
  Serial.println(ss5ST);
  Serial.print(F("SS5 End Time:"));
  Serial.println(ss5FT);
}

void writeSS6rawData()
{
  String ss6StartString;
  String ss6FinishString;
  ss6StartString += String(ss6Start);           //converts ss6Start epoch time to string type and store in ss1StartString
  ss6FinishString += String(ss6Finish);         //converts ss6Finish epoch time to string type and store in ss1FinishString
  ss6StartString.toCharArray(ss6ST, 17);                           //convert ss7StartString to charater array ss7ST
  ss6FinishString.toCharArray(ss6FT, 17);                          //convert ss7FinishString to charater array ss7FT

  Serial.print(F("SS6 Start Time:"));
  Serial.println(ss6ST);
  Serial.print(F("SS6 End Time:"));
  Serial.println(ss6FT);
}

void writeSS7rawData()
{
  String ss7StartString;
  String ss7FinishString;
  ss7StartString += String(ss7Start);           //converts ss7Start epoch time to string type and store in ss7StartString
  ss7FinishString += String(ss7Finish);         //converts ss7Finish epoch time to string type and store in ss7FinishString
  ss7StartString.toCharArray(ss7ST, 17);                           //convert ss7StartString to charater array ss7ST
  ss7FinishString.toCharArray(ss7FT, 17);                          //convert ss7FinishString to charater array ss7FT

  Serial.print(F("SS7 Start Time:"));
  Serial.println(ss7ST);
  Serial.print(F("SS7 End Time:"));
  Serial.println(ss7FT);
}

void writeSS8rawData()
{
  String ss8StartString;
  String ss8FinishString;
  ss8StartString += String(ss8Start);           //converts ss8Start epoch time to string type and store in ss8StartString
  ss8FinishString += String(ss8Finish);         //converts ss8Finish epoch time to string type and store in ss8FinishString
  ss8StartString.toCharArray(ss8ST, 17);                           //convert ss8StartString to charater array ss8ST
  ss8FinishString.toCharArray(ss8FT, 17);                          //convert ss8FinishString to charater array ss8FT

  Serial.print(F("SS8 Start Time:"));
  Serial.println(ss8ST);
  Serial.print(F("SS8 End Time:"));
  Serial.println(ss8FT);
}

String sendData(String params, char* domain) {
  //google scripts requires two get requests
  bool needRedir = false;
  if (domain == NULL)
  {
    domain = (char*)host;
    needRedir = true;
    params = "/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + params;
  }

  Serial.println(*domain);
  String result = "";
  client.setInsecure();
  Serial.print("connecting to ");
  Serial.println(host);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return "";
  }

  if (client.verify(fingerprint, domain)) {
  }

  Serial.print("requesting URL: ");
  Serial.println(params);

  client.print(String("GET ") + params + " HTTP/1.1\r\n" +
               "Host: " + domain + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {

    String line = client.readStringUntil('\n');
    //Serial.println(line);
    if (needRedir) {

      int ind = line.indexOf("/macros/echo?user");
      if (ind > 0)
      {
        Serial.println(line);
        line = line.substring(ind);
        ind = line.lastIndexOf("\r");
        line = line.substring(0, ind);
        Serial.println(line);
        result = line;
      }
    }

    if (line == "\r") {
      Serial.println("headers received");
      SuccessfulUploadBeep();
      break;
    }

  }
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (!needRedir)
      if (line.length() > 5)
        result = line;
    //Serial.println(line);

  }
  if (needRedir)
    return sendData(result, "script.googleusercontent.com");
  else return result;
}
