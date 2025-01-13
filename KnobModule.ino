//include libraries
#include <esp_now.h>
#include <WiFi.h>

// Pin definitions for rotary encoder
#define CLK 4
#define DT 5
#define BUTTON_PIN 13
// Rotary encoder and button state variables
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
unsigned long lastButtonPress = 0;
int prevbuttonstate = 0;

// THE RECEIVER'S MAC Address
uint8_t broadcastAddress[] = {0xd8, 0x3b, 0xda, 0xa4, 0xcc, 0xe8};

// Structure to send data
// Must match the receiver structure
typedef struct struct_message {
    int id; // Must be unique for each sender board
    int x;  // Rotary encoder direction
    int y;  // Separate button state
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);

  // Set up the button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  lastStateCLK = digitalRead(CLK);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register for Send Callback
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

//Encoder function for scrolling direction
void EncoderRotation() {
  currentStateCLK = digitalRead(CLK);
  myData.id = 2;

  if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      counter--;
      currentDir = "CCW";
      myData.x = -1;
    } else {
      // Encoder is rotating CW so increment
      counter++;
      currentDir = "CW";
      myData.x = 1;
    }
    //for debugging
    Serial.print("Direction: ");
    Serial.print(currentDir);
    Serial.println(myData.x);
    Serial.print(" | Counter: ");
    Serial.println(counter);
    //transmitting data to receiver
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }
  } else {
    myData.x = 0;
  }

  // Remember last CLK state
  lastStateCLK = currentStateCLK;
}
//function for encoder button press and transmission
void button() {
  myData.id = 2;
  int btnState = digitalRead(BUTTON_PIN); // Read the separate button's state

  if (btnState == LOW && prevbuttonstate == 0) { // Button pressed
    if (millis() - lastButtonPress > 50) { // Debouncing
      Serial.println("Separate button pressed!");
      myData.y = 1;
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
      if (result == ESP_OK) {
        Serial.println("Sent with success");
      } else {
        Serial.println("Error sending the data");
      }
      lastButtonPress = millis();
      prevbuttonstate = 1;
    }
  } else if (btnState == HIGH && prevbuttonstate == 1) { // Button released
    myData.y = 0;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }
    prevbuttonstate = 0;
  }
}
//Loop that executes both functions 
void loop() {
  EncoderRotation(); // Handle rotary encoder
  button();          // Handle the separate button
  delay(10);         // Small delay for stability
}
