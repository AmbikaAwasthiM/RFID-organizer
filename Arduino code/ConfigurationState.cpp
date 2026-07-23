#include <MFRC522.h>
#include <SPI.h>
#include <vector>
#include <array>

#define SS_PIN   10
#define RST_PIN  9

class Tag {
public:
  String name;
  bool inBag;
  std::array<byte, 4> uid;
};

std::array<byte, 4> scannedUID;
String pendingName;
bool promptShown = false;
int idx = -1;

enum State { IDLE, WAITING, REASSIGN };
State currentState = IDLE;

std::vector<Tag> tags;

MFRC522 mfrc522(SS_PIN, RST_PIN);

int findTagIndex(std::array<byte, 4> uid) {
  for (int i = 0; i < tags.size(); i++) {
    if (tags[i].uid == uid) {
      return i;
    }
  }
  return -1;
}

void dumbass(int idx) 
{
  if (!promptShown) {
    Serial.println("Tag already assigned, reassign? (y/n)");
    promptShown = true;
  }
  if (Serial.available() > 0) 
  {
    char answer = tolower(Serial.read());
    while (Serial.available() > 0) { Serial.read(); }
    if (answer == 'y') 
    {
      currentState = WAITING;
      promptShown = false;
      return;
    } 
    else if (answer == 'n') 
    {
      Serial.print("current name is: ");
      Serial.println(tags[idx].name);
      currentState = IDLE;
      promptShown = false;
      return;
    }
    return;
  }
}

void setup() 
{
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() 
{

  if (currentState == REASSIGN) 
  {
    dumbass(idx);
  }

  if (currentState == WAITING) 
  {
    if (!promptShown)
     {
      Serial.println("What object does this NFC sticker belong to?");
      promptShown = true;
    }
    if (Serial.available() > 0) 
    {
      pendingName = Serial.readStringUntil('\n');
      pendingName.trim(); 
      if (idx != -1) 
      {
        Serial.println("Tag name is: ");
        Serial.print(pendingName);
        tags[idx].name = pendingName;
      } 
      else 
      {
        tags.push_back({pendingName, false, scannedUID});
      }
      currentState = IDLE;
      promptShown = false;
      idx = -1;
    }
  }

  if (currentState == IDLE && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) 
  {
    Serial.println("UID scanned");
    memcpy(scannedUID.data(), mfrc522.uid.uidByte, 4);
    for (int i = 0; i < 4; i++) 
    {
      Serial.print(scannedUID[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    mfrc522.PICC_HaltA();

    idx = findTagIndex(scannedUID);
    currentState = (idx != -1) ? REASSIGN : WAITING;
  }
}
