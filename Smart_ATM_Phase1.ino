// Smart_ATM.ino
// Complete starter implementation.

#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER A1
#define GREEN_LED A2
#define RED_LED A3
#define SERVO_PIN A4

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo atmServo;

const byte ROWS=4,COLS=4;
char keys[ROWS][COLS]={{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
byte rowPins[ROWS]={2,3,4,5};
byte colPins[COLS]={6,7,8,A0};
Keypad keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS);

struct User{byte uid[4];char pin[5];long balance;};

User users[2]={
{{0x55,0x89,0x0B,0x07},"1234",5000},
{{0xB9,0x84,0x40,0x99},"4321",3000}
};

int currentUser=-1;

bool compareUID(byte*a,byte*b){for(byte i=0;i<4;i++)if(a[i]!=b[i])return false;return true;}
void beep(){digitalWrite(BUZZER,HIGH);delay(80);digitalWrite(BUZZER,LOW);}
void ok(){digitalWrite(GREEN_LED,HIGH);delay(200);digitalWrite(GREEN_LED,LOW);}
void err(){digitalWrite(RED_LED,HIGH);delay(200);digitalWrite(RED_LED,LOW);}
void loadEEPROM(){for(int i=0;i<2;i++){User t;EEPROM.get(i*sizeof(User),t);if(t.pin[0]==0xFF||t.pin[0]=='\0')EEPROM.put(i*sizeof(User),users[i]);else users[i]=t;}}
void saveUser(int id){EEPROM.put(id*sizeof(User),users[id]);}

bool scanCard(){
 if(!mfrc522.PICC_IsNewCardPresent())return false;
 if(!mfrc522.PICC_ReadCardSerial())return false;
 currentUser=-1;
 for(int i=0;i<2;i++)if(compareUID(mfrc522.uid.uidByte,users[i].uid)){currentUser=i;break;}
 mfrc522.PICC_HaltA();
 if(currentUser==-1){Serial.println("Access Denied");err();beep();return false;}
 ok();beep();Serial.println("Card Verified");return true;
}

bool verifyPIN(){
 char pin[5];byte idx=0;Serial.println("Enter PIN:");
 while(idx<4){char k=keypad.getKey();if(k>='0'&&k<='9'){pin[idx++]=k;Serial.print('*');beep();}}
 pin[4]='\0';
 if(strcmp(pin,users[currentUser].pin)==0){ok();return true;}
 err();return false;
}

int readNumber(){
 String s="";
 while(true){
  char k=keypad.getKey();
  if(!k)continue;
  if(k=='#')break;
  if(k>='0'&&k<='9'){s+=k;Serial.print(k);beep();}
 }
 Serial.println();
 return s.toInt();
}

void balance(){Serial.print("Balance: ");Serial.println(users[currentUser].balance);}
void changePIN(){
 char np[5];byte i=0;
 Serial.println("New PIN:");
 while(i<4){char k=keypad.getKey();if(k>='0'&&k<='9'){np[i++]=k;Serial.print('*');}}
 np[4]='\0';strcpy(users[currentUser].pin,np);saveUser(currentUser);Serial.println("\nPIN Updated");
}
void withdraw(){
 Serial.println("Amount then #:");
 int amt=readNumber();
 if(amt<=0)return;
 if(amt>users[currentUser].balance){Serial.println("Insufficient Balance");err();return;}
 users[currentUser].balance-=amt;
 saveUser(currentUser);
 atmServo.write(90);delay(1500);atmServo.write(0);
 balance();
}
void menu(){
 while(true){
  Serial.println("A Withdraw  B Balance  C Change PIN  D Exit");
  char k=keypad.getKey();
  if(!k)continue;
  if(k=='A')withdraw();
  else if(k=='B')balance();
  else if(k=='C')changePIN();
  else if(k=='D'){currentUser=-1;Serial.println("Logged Out");return;}
 }
}

void setup(){
 Serial.begin(9600);
 SPI.begin();
 mfrc522.PCD_Init();
 pinMode(BUZZER,OUTPUT);pinMode(GREEN_LED,OUTPUT);pinMode(RED_LED,OUTPUT);
 atmServo.attach(SERVO_PIN);atmServo.write(0);
 loadEEPROM();
 Serial.println("SMART ATM");
 Serial.println("Scan RFID Card");
}

void loop(){
 if(scanCard()){
  if(verifyPIN()){
   menu();
   Serial.println("Scan RFID Card");
  }else{
   Serial.println("Wrong PIN");
  }
 }
}
