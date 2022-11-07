/**
*header file for ITA v1 Blue PCB
*/

#define VERSION "V1"
//GLOBAL DEFINITIONS         


#define RFID_RST      D0
#define BUZZER        D8
#define LED_TOP       D3
#define LED_BOT       D2
#define RFID_CS       D4 
#define RFID_MOSI     MOSI //(D7)
#define RFID_MISO     MISO //(D6)
#define RFID_SCK      SCK //(D5)
#define STATUS_LED    D1 //bluetooth adn oled also uses D1 so if BT/oled required, all statusled commands must be taken out //can't do anything with status led in v1 board dan


#define ADDRESS_POD_TYPE 0

#define BATT_GOOD       3.8
#define BATT_LOW        3.75
#define BATT_OK_FLASH   2000
#define BATT_LOW_FLASH  100
// BELOW IS STATION/POD IDENTIFICATION!
#define LIST_SIZE 16
#define START_POD  0
#define FINISH_POD 1

struct podType {
  int block;
  char podID[16]; 
  int  role;
} ;

/*
struct podType podTypeList [LIST_SIZE] = {
  { 8 , "Stage 1 Start" , START_POD },
  { 9 , "Stage 1 Finish", FINISH_POD }, 
  { 12, "Stage 2 Start" , START_POD },
  { 13, "Stage 2 Finish", FINISH_POD },
  { 16, "Stage 3 Start" , START_POD },
  { 17, "Stage 3 Finish", FINISH_POD },
  { 20, "Stage 4 Start" , START_POD },
  { 21, "Stage 4 Finish", FINISH_POD },
  { 24, "Stage 5 Start" , START_POD },
  { 25, "Stage 5 Finish", FINISH_POD },
  { 28, "Stage 6 Start" , START_POD },
  { 29, "Stage 6 Finish", FINISH_POD },
  { 32, "Stage 7 Start" , START_POD },
  { 33, "Stage 7 Finish", FINISH_POD },
  { 36, "Stage 8 Start" , START_POD },
  { 37, "Stage 8 Finish", FINISH_POD },
    };*/
// variouse values
#define CONFIG_TIME_MS 10000  
// int main()
// {
//     printf("Hello World\n");
//     printf( "%i\n",STRUCTSIZE);
//     for ( int i = 0 ; i < STRUCTSIZE ; i++ ) {
//          printf("%s\n", podTypeList[i].podID);
//     }

//     return 0;
// }
