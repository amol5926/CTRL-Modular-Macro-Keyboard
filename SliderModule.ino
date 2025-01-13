#include <WiFi.h>
#include <esp_now.h>

#define POT_PIN 13  // Slider on GPIO 13 (Analog pin)

int potValue = 0; //variable for pot Value
int scaledValue = 0; //variable for scaled pot Value
int lastScaledValue = -1;  // To store the last scaled value

// Define the original range (from 20 to 2740) (Slider reaches highest value before end position so a resistor was added--> max value is now lower than the 12Bit max)
int oldMin = 17;
int oldMax = 2740;

// Define the new range (0 to 100)
int newMin = 0;
int newMax = 100;

// Moving average variables
const int numSamples = 3; // Number of samples for averaging
int potSamples[numSamples] = {0};
int sampleIndex = 0;

// Hysteresis threshold (percentage of the range)
const int hysteresisThreshold = 1; // 1% threshold
const int hysteresisValue = (newMax - newMin) * hysteresisThreshold / 100;

// THE RECEIVER'S MAC Address
uint8_t broadcastAddress[] = {0xd8, 0x3b, 0xda, 0xa4, 0xcc, 0xe8};

// Structure to send data
// Must match the receiver structure
typedef struct struct_message {
    int id; // Must be unique for each sender board
    int x; //scaled value
    int y; //not used
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

//Calculating average value to clean up signal
int calculateAverage() {
    int sum = 0;
    for (int i = 0; i < numSamples; i++) {
        sum += potSamples[i];
    }
    return sum / numSamples;
}

void setup() {
  Serial.begin(115200);  // Start serial communication at 115200 baud rate

  // Set the ADC width to 12 bits (range: 0 to 4095)
  analogReadResolution(12);

  // Configure the potentiometer pin as input
  pinMode(POT_PIN, INPUT);
  
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

void loop() {
  // Read the analog value from the potentiometer
  potValue = analogRead(POT_PIN);

  // Add the current reading to the moving average buffer
  potSamples[sampleIndex] = potValue;
  sampleIndex = (sampleIndex + 1) % numSamples;

  // Calculate the average of the last readings
  int avgValue = calculateAverage();

  // Map the averaged value from the original range (17 to 2740) to the new range (0 to 100)
  scaledValue = map(avgValue, oldMin, oldMax, newMin, newMax);

  // Ensure the scaled value is within the valid range (0 to 100)
  scaledValue = constrain(scaledValue, newMin, newMax);

  // Apply hysteresis: only send if the value changes significantly
  if (abs(scaledValue - lastScaledValue) > hysteresisValue) {
    Serial.print("Scaled Value: ");
    myData.id = 3;  
    myData.x = scaledValue;
    Serial.println(myData.x);

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }

    // Update the last scaled value
    lastScaledValue = scaledValue;
  }

  delay(100);  // Short delay for stability
}


