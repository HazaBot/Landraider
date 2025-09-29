#include <esp_now.h>
#include <WiFi.h>
#include <string.h>  // memcpy

typedef struct struct_message {
  int VRx;
  int VRy;
  int Button;
} struct_message;

volatile struct_message latest;
volatile bool hasUpdate = false;

void OnDataRecv(const esp_now_recv_info_t* info, const uint8_t* incomingData, int len) {
  if (len >= (int)sizeof(struct_message)) {
    // Copy into the volatile buffer
    memcpy((void*)&latest, incomingData, sizeof(struct_message));
    hasUpdate = true;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  if (hasUpdate) {
    noInterrupts();                  // begin critical section
    struct_message m;                // local non-volatile copy
    memcpy(&m, (const void*)&latest, sizeof(m));
    hasUpdate = false;
    interrupts();                    // end critical section

    Serial.printf("VRx: %d%%  VRy: %d%%  Btn: %d\n", m.VRx, m.VRy, m.Button);
  }

  delay(5);
}
