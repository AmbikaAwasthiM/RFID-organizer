#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <deprecated.h>
#include <require_cpp11.h>
#include <iostream>
#include <string>
#include <SPI.h>
#include <vector>

#define SS_PIN   5
#define RST_PIN  4
#define SCK_PIN  18
#define MISO_PIN 19
#define MOSI_PIN 23

class Tag //where all the cool stickers are
{
public:
    String name; 
    bool inBag;
    std::array<byte, 4> uid;
};


  std::array<byte, 4> scannedUID;
  String pendingName;
  bool promptShown = false;
  int idx;

  enum State { IDLE, WAITING, REASSIGN};
  State currentState = IDLE; 

  
  std::vector<Tag> tags; //arraylist of tags basically

  MFRC522 mfrc522(SS_PIN, RST_PIN);


void dumbass(int idx)
{
  Serial.println("Tag already assigned, do you want to reassign? (y/n)");
  if(Serial.available() > 0)
  {
    char answer = Serial.read();
    answer = tolower(answer);
    if (answer == 'y') 
    {
      if(Serial.available() > 0)
      {
        pendingName = Serial.readStringUntil('\n');
        tags[idx].name = pendingName;
        currentState = IDLE;
        return;
      }
      
    } 
    else
    {
      Serial.println("current name is: ");
      Serial.println(tags[idx].name);
      Serial.println("");
      currentState = IDLE;
      return;
    }
  }
  

}

int findTagIndex(std::array<byte,4> uid) 
{
  for (int i = 0; i < tags.size(); i++) 
  {
    if (tags[i].uid == uid) 
    { 
      return i;
    }
  }
  return -1;
}



void setup() 
{
  Serial.begin(115200);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
}

void loop() 
{
  if(currentState == REASSIGN)
  {
    dumbass(idx);
  }
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() && currentState == IDLE) //only scan if like nothing is happening
  {
    Serial.println("UID scanned");
    memcpy(scannedUID.data(), mfrc522.uid.uidByte, 4);
    idx = findTagIndex(scannedUID); //double checks if it has been scanned before
    for (int i = 0; i < 4; i++)  //print UID
    {
      Serial.print(scannedUID[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    mfrc522.PICC_HaltA(); //don't reprint until it is removed and placed again
    currentState = WAITING; 
    if (idx != -1) 
    {
      currentState = REASSIGN;
       dumbass(idx);   
    }
    else
    {
      if (currentState == WAITING) 
      {
        if (!promptShown) {
        Serial.println("What object does this NFC sticker belong to?");
        promptShown = true;
      }
      if (Serial.available() > 0) 
      {
        pendingName = Serial.readStringUntil('\n');
        tags.push_back({pendingName, false, scannedUID});
        currentState = IDLE;
        promptShown = false;  // reset for next tag
      }

    }

    
  }
}}
