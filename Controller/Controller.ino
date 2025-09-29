#include <esp_now.h>
#include <WiFi.h>
#include <math.h>

const int pinX = 1; 
const int pinY = 2; 
const int pinSW = 3;
int midX = 0;
int midY = 0;

uint8_t broadcastAddress[] = {0xD0,0xCF,0x13,0x07,0xE8,0x78};

// Needs to match with the reciever. 
//Typedef creates a alias for future decleration
typedef struct struct_message {
  int VRx;
  int VRy;
  int Button;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

int normalisation(int raw, int mid, int deadzone){
  int ph = raw - mid;
  if (abs(ph) <= deadzone){
    return 0; 
  }
  if (ph > 0) {
    // remove deadzone, scale upper side to +100
    float num = (float)(ph - deadzone);
    float den = (float)(4095 - mid - deadzone);
    if (den < 1) den = 1;
    float pct = 100.0f * num / den;
    if (pct > 100) pct = 100;
    return (int)roundf(pct);
  } else {
    // remove deadzone, scale lower side to -100
    float num = (float)(-ph - deadzone);
    float den = (float)(mid - deadzone);
    if (den < 1) den = 1;
    float pct = 100.0f * num / den;
    if (pct > 100) pct = 100;
    return -(int)roundf(pct);
  }
}

void getMidPoint (){
  int phx = 0;
  int phy = 0;
  for (int i = 0; i < 20; i++){
    phx += analogRead(pinX);
    phy += analogRead(pinY);
  }
  midX = phx / 20;
  midY = phy / 20;
}

void setup() {
  Serial.begin(115200);
  pinMode(pinSW, INPUT_PULLUP);
  getMidPoint();
  WiFi.mode(WIFI_STA);
  esp_now_init();
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
  }

void loop() {
  int rawX = analogRead(pinX);
  int rawY = analogRead(pinY);
  myData.VRx = normalisation(rawX, midX, 30);
  myData.VRy = normalisation(rawY, midY, 30);


  myData.Button = digitalRead(pinSW); 
  Serial.printf("VRx: %d%%  VRy: %d%%  Btn: %d\n", myData.VRx, myData.VRy, myData.Button);

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(500);
}