#include <SPI.h>
#include <string.h>
#define MAX_STRING_LEN  25

String content = "";
char   character;
int    rx=0;
int    new_Vdes;

const byte numChars = 25;     //used for serial recieve
char receivedBytes[numChars]; //used for serial recieve
boolean newData = false;      //used for serial recieve
char *record1;                //used for serial recieve
char *p, *i;                  //used for serial recieve
unsigned long startTime = 0;   //inactivity timer start time
bool updateSinceReset = false; //tracks if parameters have been updated since last reset

// set up the speed, data order and data mode
//SettingsB is AD5270, <50 MHz, MSB first, sample on falling clock
SPISettings settingsB(5000000, MSBFIRST, SPI_MODE1); 

void setup() {
  Serial.begin(38400); // Initialize serial communication at 38400 bits per second

  pinMode(0, INPUT); // Fixes RX pin weirdness

  pinMode(2, OUTPUT);  //CS1 for U6, rheostat for Vfil
  digitalWrite(2,HIGH);
  pinMode(3, OUTPUT);  //Fil_enable
  digitalWrite(3,LOW); //Start off disabled
  pinMode(4, OUTPUT);  //gridV_disable
  digitalWrite(4,HIGH); //Start off disabled
  pinMode(6, OUTPUT);  //CS2 for U11, rheostat for V grid bus
  digitalWrite(6,HIGH);
  pinMode(5, OUTPUT);  //Filament slow-start bypass enable flag
  digitalWrite(5,LOW);  //Filament slow-start enabled at power-on
  
  SPI.begin(); // initialize SPI
  delay(2000);

  /////////
  //Set output voltage to minimum
  //enable rheostat
  byte high = B00011100;  //command 7 0x1C
  byte lo = B00000010;    // 0x02
  SPI.beginTransaction(settingsB);
  digitalWrite (2, LOW);
  SPI.transfer(high); 
  SPI.transfer(lo); 
  digitalWrite (2, HIGH);
  SPI.endTransaction();
  //////////////////

  //update rheostat
  //control bits: B000001 (6bits)
  //followed by 10 bits
  new_Vdes = 1023; //10bits
  lo = new_Vdes & B11111111; //low 8 bits
  high = B00000100 | ((new_Vdes>>8)&B11); //control bits plus high 2 bits
  SPI.beginTransaction(settingsB);
  digitalWrite (2, LOW);
  SPI.transfer(high); 
  SPI.transfer(lo);         
  digitalWrite (2, HIGH);
  SPI.endTransaction();
  /////////////////

  //update rheostat
  //enable rheostat
  high = B00011100;  //command 7 0x1C
  lo = B00000010;    // 0x02
  SPI.beginTransaction(settingsB);
  digitalWrite (6, LOW);
  SPI.transfer(high); 
  SPI.transfer(lo); 
  digitalWrite (6, HIGH);
  SPI.endTransaction();
  //////////////////
  //control bits: B000001 (6bits)
  //followed by 10 bits
  new_Vdes = 1023; //10bits
  lo = new_Vdes & B11111111; //low 8 bits
  high = B00000100 | ((new_Vdes>>8)&B11); //control bits plus high 2 bits
  SPI.beginTransaction(settingsB);
  digitalWrite (6, LOW);
  SPI.transfer(high); 
  SPI.transfer(lo);         
  digitalWrite (6, HIGH);
  SPI.endTransaction();
  /////////////////
}


// The loop routine runs over and over again forever:
void loop() {
    recvWithStartEndBytes(); //handles serial
    record1 = receivedBytes; //bitches about this. char to *char    
    processNewData();        //parse into variables, check fault, update DACs
    resetIfIdle();           //watchdog disables system if communications d/c
}


void processNewData() {
  if (newData == true) {
    startTime = millis();   // reset idle timer if new data is received
    //store parced data in variables
    String temp_S;

    //Fifth number shall be filament slow-start bypass flag. (1 = BYPASS) 
    temp_S = String(subStr(record1, " ", 5));
    boolean startup_bypass = temp_S.toInt();
    if (startup_bypass) {
      digitalWrite(5, HIGH);
    }
    else {
      digitalWrite(5, LOW);
    }

    //First number is the counts for U6, The filV
    temp_S = String(subStr(record1, " ", 1));
    new_Vdes = temp_S.toInt(); //in counts

    //update rheostat
    //control bits: B000001 (6bits)
    //followed by 10 bits
    //new_Vdes = 50; //10bits
    byte lo = new_Vdes & B11111111; //low 8 bits
    byte high = B00000100 | ((new_Vdes>>8)&B11); //control bits plus high 2 bits
    SPI.beginTransaction(settingsB);
    digitalWrite (2, LOW);
    SPI.transfer(high); 
    SPI.transfer(lo);         
    digitalWrite (2, HIGH);
    SPI.endTransaction();
    /////////////////

    //second number is 1 or 0 for fil enable or disable
    temp_S = String(subStr(record1, " ", 2));
    boolean fil_enable =  temp_S.toInt();  //in counts
    if (fil_enable) {
      digitalWrite(3,HIGH);
    }
    else {
      digitalWrite(3,LOW);
    }
    
    //Third number is the counts for U11, The gridV feedback
    temp_S = String(subStr(record1, " ", 3));
    new_Vdes = temp_S.toInt(); //in counts

    //update rheostat
    //control bits: B000001 (6bits)
    //followed by 10 bits
    //new_Vdes = 50; //10bits
    lo = new_Vdes & B11111111; //low 8 bits
    high = B00000100 | ((new_Vdes>>8)&B11); //control bits plus high 2 bits
    SPI.beginTransaction(settingsB);
    digitalWrite (6, LOW);
    SPI.transfer(high); 
    SPI.transfer(lo);         
    digitalWrite (6, HIGH);
    SPI.endTransaction();
    /////////////////

    //Fourth number is the grid disable
    temp_S = String(subStr(record1, " ", 4));
    boolean grid_disable  = temp_S.toInt(); 
    if (grid_disable) {
      digitalWrite(4,HIGH);
    }
    else {
      digitalWrite(4,LOW);
    }

    updateSinceReset = true;

    Serial.println(analogRead(A0)); //Vbus cond
    Serial.println(analogRead(A1)); //Vfil
    Serial.println(analogRead(A2)); //Ifil
    Serial.println(analogRead(A3)); //Vgrid
    newData = false;
  }
}

//Function to disable unit if no serial input containing start byte is received for >10s
void resetIfIdle() {
  if (millis() - startTime > 10000 && updateSinceReset)
  {
    //code to reset FS/GP outputs
    digitalWrite(3, LOW);         // filament disable
    digitalWrite(4, HIGH);        // grid disable

    //update filament & grid voltage rheostats to minimum
    //control bits: B000001 (6bits)
    //followed by 10 bits
    new_Vdes = 1023; //10 bits
    byte lo = new_Vdes & B11111111; //low 8 bits
    byte high = B00000100 | ((new_Vdes >> 8)&B11); //control bits plus high 2 bits
    SPI.beginTransaction(settingsB);
    digitalWrite (2, LOW);
    SPI.transfer(high);
    SPI.transfer(lo);
    digitalWrite (2, HIGH);
    SPI.endTransaction();
    SPI.beginTransaction(settingsB);
    digitalWrite (6, LOW);
    SPI.transfer(high);
    SPI.transfer(lo);
    digitalWrite (6, HIGH);
    SPI.endTransaction();
    /////////////////

    digitalWrite(5, LOW); // Disable filament slow-start bypass

    updateSinceReset = false;
  }
}

//UTILITY FUNCTION
//Serial recieving function
void recvWithStartEndBytes() {
  static boolean recvInProgress = false;
  static byte ndx = 0;                      
  char startByte = '!';                // <- start byte  is a '!'
  char endByte = ',';                  // <- stop byte   is a ','
  char rb;                            
  while (Serial.available() > 0 && newData == false) {
   rb = Serial.read();
   if (recvInProgress == true) {
      if (rb != endByte) {
        receivedBytes[ndx] = rb;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedBytes[ndx] = '\0';    // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }
    else if (rb == startByte) {
        recvInProgress = true;
    }
  }
}

//UTILITY FUNCTION
// Function to return a substring defined by a delimiter at an index
char* subStr (char* str, char *delim, int index) {
  char *act, *sub, *ptr;
  static char copy[MAX_STRING_LEN];
  int i;
  strcpy(copy, str);
  for (i = 1, act = copy; i <= index; i++, act = NULL) {
     sub = strtok_r(act, delim, &ptr);
     if (sub == NULL) break;
  }
  return sub;
}
