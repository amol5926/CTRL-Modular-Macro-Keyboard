#include <esp_now.h>
#include <WiFi.h>
#include "USB.h"
#include "USBHIDMouse.h"
#include "USBHIDKeyboard.h"

USBHIDMouse mouse;
USBHIDKeyboard keyboard;


// Define modifiers for shortcuts
#define MOD_CTRL 0x01
#define MOD_SHIFT 0x02
#define MOD_ALT 0x04

int layer = 1;
int slider = 100;

// Define shortcuts
typedef struct {
  uint8_t modifiers; // Modifier keys (Ctrl, Shift, Alt)
  uint8_t key;       // Main key (HID scan code)
} Shortcut;

// Predefine a lists of shortcuts

Shortcut shortcuts_set2[] = {                   //resolve
  {MOD_CTRL, 'c'},                              //copy
  {MOD_ALT |MOD_SHIFT, 'c'},                    //auto color
  {MOD_SHIFT, 'c'},                             //curve
  {MOD_CTRL, 'f'},                              // full screen
  {MOD_CTRL, 'v'},                              // paste
  {MOD_CTRL, 'b'},                              // razor
  {MOD_CTRL | MOD_ALT, KEY_LEFT_ARROW},         // clip left
  {MOD_CTRL | MOD_ALT, KEY_RIGHT_ARROW},        //clip right
  {MOD_CTRL, 'x'},                              //cut
  {MOD_CTRL, 's'},                              //save
  {MOD_CTRL, 'w'},                              //undo
  {MOD_CTRL | MOD_SHIFT, 'w'},                  //redo  
};

Shortcut shortcuts[] = {                        //affinity
  {MOD_CTRL, 'c'},                              // copy
  {MOD_CTRL, 'g'},                              // group
  {MOD_CTRL, 'n'},                              //new file
  {MOD_CTRL |MOD_ALT |MOD_SHIFT, 's'},          // export 
  {MOD_CTRL, 'v'},                              //paste
  {0, 'q'},                                     // node
  {0, 'p'},                                     // pen
  {0, 'i'},                                     //color
  {MOD_CTRL,'x'},                               //cut
  {MOD_CTRL, 's'},                              // save          
  {MOD_CTRL, 'w'},                              //undo
  {MOD_CTRL | MOD_SHIFT, 'w'},                  //redo
};

Shortcut shortcuts_set3[] = {                   //blender
  {MOD_CTRL, 'c'},                              //copy
  {MOD_ALT |MOD_SHIFT, 'q'},                    //autokey
  {0, 'i'},                                     // +keyframe
  {MOD_ALT, 'i'},                               // -keyframe
  {MOD_CTRL, 'v'},                              //paste
  {0, 0x40},                                    // cam
  {MOD_CTRL, KEY_F12},                          // render animation
  {0, KEY_F12},                                 // render frame 
  {MOD_CTRL,'x'},                               //cut
  {MOD_CTRL, 's'},                              // save          
  {MOD_CTRL, 'w'},                              //undo
  {MOD_CTRL | MOD_SHIFT, 'w'},                  //redo 
};

//switching between apps (used by AutoHotkey)
Shortcut shortcuts_set4[] = { 
  {MOD_CTRL | MOD_ALT, 'b'},
  {MOD_CTRL | MOD_ALT, 'r'},
  {MOD_ALT, 'd'},
  {MOD_CTRL | MOD_ALT, 's'},
};


// Structure for ESP-NOW message
typedef struct struct_message {          
  int id;  // Board ID
  int x;   // Data 1
  int y;   // Data 2
} struct_message;

// Global Variables
struct_message myData;
struct_message boardsStruct[3];

// Callback for receiving ESP-NOW data
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %d: Received data: x=%d, y=%d\n", myData.id, myData.x, myData.y);
  
  // Update the corresponding board structure
  if (myData.id > 0 && myData.id <= 3) {
    boardsStruct[myData.id - 1] = myData;
  }
}

void setup() {
  // Initialize Serial, USB, Mouse, and Keyboard
  Serial.begin(115200);
  USB.begin();
  mouse.begin();
  keyboard.begin();

  // Initialize WiFi and ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register the ESP-NOW receive callback
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

}

void loop() {
  // Get the current states for board 1
  int board1X = boardsStruct[0].x; // Layer switcher
  int board1Y = boardsStruct[0].y; // Key
  int board2X = boardsStruct[1].x; // Rotation
  int board2Y = boardsStruct[1].y; // Button
  int board3X = boardsStruct[2].x; // Slider position
  int board3Y = boardsStruct[2].y; // Unused

//Slider function for opacity in Affinity Designer
  if (layer ==1){
    if (board3X != slider && board3X >=10 ){
      typeNumberAsText(board3X);
      delay(450);
      slider = board3X;
    }
  }
//Slider function for horizontal zoom Davinci
  if (layer ==3){
    if (board3X != slider){
        int amount = board3X-slider;
        keyboard.pressRaw(HID_KEY_ALT_LEFT); // Hold down the Ctrl key
        mouse.move(0, 0, amount);  // Scroll down
        Serial.println("Scrolling Down");
        keyboard.releaseRaw(HID_KEY_ALT_LEFT); // Hold down the Ctrl key
      slider = board3X;
    }
  }
//function for (horizontal) zoom Blender
  if (layer ==2){
    if (board3X != slider){
        int amount = board3X-slider;
        mouse.move(0, 0, amount);  // Scroll down
        Serial.println("Scrolling Down");
      slider = board3X;
    }
  }
//Encoder scrubbing in Davinci + play/pause
  if (layer == 3) {
      if (board2X == 1 || board2X == -1) {
          keyboard.pressRaw(HID_KEY_CONTROL_LEFT); // Hold down the Ctrl key
          if (board2X == 1) {
              mouse.move(0, 0, -1);
          } else if (board2X == -1) {
              mouse.move(0, 0, 1);   // Scroll
          }
      } else {
          keyboard.releaseRaw(HID_KEY_CONTROL_LEFT);
      }

      if (board2Y == 1) {
          keyboard.pressRaw(HID_KEY_SPACE);
          keyboard.releaseRaw(HID_KEY_SPACE);
      }

      // Reset input state
      boardsStruct[1].x = 0;
      boardsStruct[1].y = 0;
  }
//Encoder Scrubbing in Blender + play/pause
  if (layer == 2){
    if (board2X == 1) {
      keyboard.pressRaw(HID_KEY_ARROW_LEFT); 
      keyboard.releaseRaw(HID_KEY_ARROW_LEFT);
    }
    if (board2X == -1) {
      keyboard.pressRaw(HID_KEY_ARROW_RIGHT); 
      keyboard.releaseRaw(HID_KEY_ARROW_RIGHT);
    }
    if (board2Y== 1){
      keyboard.pressRaw(HID_KEY_SPACE); 
      keyboard.releaseRaw(HID_KEY_SPACE); 
    }
    boardsStruct[1].x = 0;
    boardsStruct[1].y = 0;
  }
//Encoder Zoom and canvas rotation in Affinity Designer
  if (layer == 1) {
    if (board2Y ==0)  {
      if (board2X == 1 || board2X == -1) {
          keyboard.pressRaw(HID_KEY_CONTROL_LEFT); // Hold down the Ctrl key
          if (board2X == 1) {
              mouse.move(0, 0, -1);  // Scroll
          } else if (board2X == -1) {
              mouse.move(0, 0, 1);   // Scroll
          }
      } else {
          keyboard.releaseRaw(HID_KEY_CONTROL_LEFT);
      }
    }
    if (board2Y == 1) {
      if (board2X == 1 || board2X == -1) {
        keyboard.pressRaw(HID_KEY_ALT_LEFT); // Hold down the Ctrl key
          if (board2X == 1) {
              mouse.move(0, 0, 3);  // Scroll down
              Serial.println("Scrolling Down");
              keyboard.releaseRaw(HID_KEY_ALT_LEFT);
          } else if (board2X == -1) {
                mouse.move(0, 0, -3);   // Scroll up
                Serial.println("Scrolling Up");
                keyboard.releaseRaw(HID_KEY_ALT_LEFT);
          }
      } 
    }

      // Reset input state
  boardsStruct[1].x = 0;
  boardsStruct[1].y = 0;
  }

// Layer selection and shortcut selection
  if (board1Y == 1) { 
    if (board1X == 0){
      sendShortcut(shortcuts_set4[0]);
    // Ensure the shortcut index is valid
    } else if (board1X > 0) {
      sendShortcut(shortcuts_set2[board1X-1]); // Send the shortcut
    } else {
      Serial.println("Invalid shortcut index.");
    }
    // Reset board1Y to stop repeating the shortcut
    layer = 3;
    boardsStruct[0].y = 0;
  }

// Layer selection and shortcut selection
  if (board1Y == 2) {
    if (board1X == 0){
      sendShortcut(shortcuts_set4[1]);
    } else if (board1X > 0) {
      sendShortcut(shortcuts[board1X - 1]); // Send the shortcut
    } else {
      Serial.println("Invalid shortcut index.");
    }
    // Reset board1Y to stop repeating the shortcut
    layer =1;
    boardsStruct[0].y = 0;
  }
// Layer selection and shortcut selection
  if (board1Y == 3) {
    if (board1X == 0){
      sendShortcut(shortcuts_set4[2]);
    } else if (board1X > 0) {
      sendShortcut(shortcuts_set3[board1X-1]); // Send the shortcut
    } else {
      Serial.println("Invalid shortcut index.");
    }
    // Reset board1Y to stop repeating the shortcut
    layer = 2;
    boardsStruct[0].y = 0;
  }
}

// Function to send a keyboard shortcut
void sendShortcut(Shortcut shortcut) {
  // Log the shortcut being sent
  Serial.printf("Sending shortcut: Modifiers=%02x, Key=%02x\n", shortcut.modifiers, shortcut.key);

  // Press modifiers
  if (shortcut.modifiers & MOD_CTRL) keyboard.press(KEY_LEFT_CTRL);
  if (shortcut.modifiers & MOD_SHIFT) keyboard.press(KEY_LEFT_SHIFT);
  if (shortcut.modifiers & MOD_ALT) keyboard.press(KEY_LEFT_ALT);
  // exceptions due to language difference of library
  if (shortcut.key == 0x40) {  // Numpad 0
    keyboard.pressRaw(0x62);   // Press the Numpad 0 key
  } else if (shortcut.key == 0xDE) {
    keyboard.pressRaw(0xDE);
  } else {
    keyboard.press(shortcut.key);  // Press other keys
  }
  keyboard.releaseAll();  // Release all modifiers
}
//function for slider output to Affinity Designer (needed because of different keyboard layout)
void typeNumberAsText(int board3X) {
    char buffer[4]; // Buffer to hold the string (3 digits + null terminator)
    itoa(board3X, buffer, 10); // Convert integer to string in base 10

    // Conversion to Hex
    uint8_t numpadKeyCodes[] = {
        0x62,  // Numpad 0
        0x59,  // Numpad 1
        0x5A,  // Numpad 2
        0x5B,  // Numpad 3
        0x5C,  // Numpad 4
        0x5D,  // Numpad 5
        0x5E,  // Numpad 6
        0x5F,  // Numpad 7
        0x60,  // Numpad 8
        0x61,  // Numpad 9
    };
    // Send each character of the string, but stop before the null terminator
    for (int i = 0; buffer[i] != '\0'; i++) {
        uint8_t keyCode;
        // Map numeric characters to Numpad keycodes
        if (buffer[i] >= '0' && buffer[i] <= '9') {
            keyCode = numpadKeyCodes[buffer[i] - '0'];
        } else {
            continue; // Ignore unexpected characters
        }
        keyboard.pressRaw(keyCode);  // Press the numpad key
        keyboard.releaseAll(); // Release the numpad key
    }
}