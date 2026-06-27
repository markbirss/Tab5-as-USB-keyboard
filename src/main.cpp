#include <Arduino.h>
#include <Wire.h>
#include "USB.h"
#include "USBHIDKeyboard.h"

// Tab5 Hardware Address Protocols
#define KEYBOARD_I2C_ADDR   0x6D
#define REG_KEYBOARD_MODE   0x10
#define REG_NORMAL_EVENT    0x20  

#define MODE_NORMAL         0x00  // Standard raw matrix mode (Blue LED)
#define KEY_COUNT           70

// Target ESP32-S3 DevKit Pins
#define S3_SDA_PIN 8   
#define S3_SCL_PIN 9   
#define S3_INT_PIN 10  

USBHIDKeyboard ComputerKeyboard;

// Struct layout definition to hold your key data configurations
struct HidMapping {
    uint8_t usageId;
    uint8_t modifier;
};

// Global software modifier state switches
bool shiftActive = false;
bool symActive   = false;
bool ctrlActive  = false; 
bool altActive   = false; 

// 1. YOUR BASE LOOKUP TABLE
constexpr HidMapping KEY_MATRIX_HID_BASE[KEY_COUNT] = {
    // Row 0: Esc 1 2 3 4 5 6 7 8 9 0 - + Del
    {0x29, 0x00}, {0x1E, 0x00}, {0x1F, 0x00}, {0x20, 0x00}, {0x21, 0x00}, {0x22, 0x00}, {0x23, 0x00}, {0x24, 0x00}, {0x25, 0x00}, {0x26, 0x00}, {0x27, 0x00}, {0x2D, 0x00}, {0x2E, 0x02}, {0x4C, 0x00},
    // Row 1: ` ! @ # $ % ^ & * ( ) [ ] backslash
    {0x35, 0x00}, {0x1E, 0x02}, {0x1F, 0x02}, {0x20, 0x02}, {0x21, 0x02}, {0x22, 0x02}, {0x23, 0x02}, {0x24, 0x02}, {0x25, 0x02}, {0x26, 0x02}, {0x27, 0x02}, {0x2F, 0x00}, {0x30, 0x00}, {0x31, 0x00},
    // Row 2: Tab q w e r t y u i o p ; ' Backspace
    {0x2B, 0x00}, {0x14, 0x00}, {0x1A, 0x00}, {0x08, 0x00}, {0x15, 0x00}, {0x17, 0x00}, {0x1C, 0x00}, {0x18, 0x00}, {0x0C, 0x00}, {0x12, 0x00}, {0x13, 0x00}, {0x33, 0x00}, {0x34, 0x00}, {0x2A, 0x00},
    // Row 3: Sym Aa a s d f g h j k l ↑ _ Enter
    {0x00, 0x00}, {0x00, 0x00}, {0x04, 0x00}, {0x16, 0x00}, {0x07, 0x00}, {0x09, 0x00}, {0x0A, 0x00}, {0x0B, 0x00}, {0x0D, 0x00}, {0x0E, 0x00}, {0x0F, 0x00}, {0x52, 0x00}, {0x2D, 0x02}, {0x28, 0x00},
    // Row 4: Ctrl Alt z x c v b n m . ← ↓ → Space
    {0x00, 0x00}, {0x00, 0x00}, {0x1D, 0x00}, {0x1B, 0x00}, {0x06, 0x00}, {0x19, 0x00}, {0x05, 0x00}, {0x11, 0x00}, {0x10, 0x00}, {0x37, 0x00}, {0x50, 0x00}, {0x51, 0x00}, {0x4F, 0x00}, {0x2C, 0x00}
};

// 2. YOUR SYM LOOKUP TABLE
constexpr HidMapping KEY_MATRIX_HID_SYM[KEY_COUNT] = {
    // Row 0
    {0x29, 0x00}, {0x1E, 0x00}, {0x1F, 0x00}, {0x20, 0x00}, {0x21, 0x00}, {0x22, 0x00}, {0x23, 0x00}, {0x24, 0x00}, {0x25, 0x00}, {0x26, 0x00}, {0x27, 0x00}, {0x2D, 0x00}, {0x2E, 0x02}, {0x4C, 0x00},
    // Row 1
    {0x35, 0x02}, {0x38, 0x02}, {0x1F, 0x02}, {0x20, 0x02}, {0x21, 0x02}, {0x22, 0x02}, {0x23, 0x02}, {0x24, 0x02}, {0x38, 0x00}, {0x36, 0x02}, {0x37, 0x02}, {0x2F, 0x02}, {0x30, 0x02}, {0x31, 0x02},
    // Row 2
    {0x2B, 0x00}, {0x14, 0x00}, {0x1A, 0x00}, {0x08, 0x00}, {0x15, 0x00}, {0x17, 0x00}, {0x1C, 0x00}, {0x18, 0x00}, {0x0C, 0x00}, {0x12, 0x00}, {0x13, 0x00}, {0x33, 0x02}, {0x34, 0x02}, {0x2A, 0x00},
    // Row 3
    {0x00, 0x00}, {0x00, 0x00}, {0x04, 0x00}, {0x16, 0x00}, {0x07, 0x00}, {0x09, 0x00}, {0x0A, 0x00}, {0x0B, 0x00}, {0x0D, 0x00}, {0x0E, 0x00}, {0x0F, 0x00}, {0x52, 0x00}, {0x2E, 0x00}, {0x28, 0x00},
    // Row 4
    {0x00, 0x00}, {0x00, 0x00}, {0x1D, 0x00}, {0x1B, 0x00}, {0x06, 0x00}, {0x19, 0x00}, {0x05, 0x00}, {0x11, 0x00}, {0x10, 0x00}, {0x36, 0x00}, {0x50, 0x00}, {0x51, 0x00}, {0x4F, 0x00}, {0x2C, 0x00}
};

bool setKeyboardMode(uint8_t mode) {
  Wire.beginTransmission(KEYBOARD_I2C_ADDR);
  Wire.write(REG_KEYBOARD_MODE); 
  Wire.write(mode);              
  return (Wire.endTransmission(true) == 0);
}

// Watchdog recovery utility to automatically reset the I2C bus state
void checkAndRecoverBus(uint8_t errorCode) {
  Serial.printf("[WATCHDOG ALERT] Bus failure detected. Code: %d. Cycling connection rails...\n", errorCode);
  
  // Clear layout tracking states to avoid ghost locks
  shiftActive = false;
  symActive   = false;
  ctrlActive  = false;
  altActive   = false;
  ComputerKeyboard.releaseAll();

  // Dismantle the stuck I2C interface hardware blocks completely
  Wire.end();
  delay(100);

  // Restart the physical wire layers on core definitions
  Wire.begin(S3_SDA_PIN, S3_SCL_PIN, 100000);
  delay(200);

  // Attempt to re-latch communication mode registers
  if (setKeyboardMode(MODE_NORMAL)) {
    Serial.println("[WATCHDOG SUCCESS] Bus communication recovered cleanly. LED returned to Blue.");
  } else {
    Serial.println("[WATCHDOG RETRY] Keyboard not responding yet. Wires might still be detached.");
  }
}

void setup() {
  Serial.begin(115200);
  
  // Start the physical I2C connection channel
  Wire.begin(S3_SDA_PIN, S3_SCL_PIN, 100000); 
  pinMode(S3_INT_PIN, INPUT_PULLUP);

  delay(1000); 
  setKeyboardMode(MODE_NORMAL); 
  delay(500); 

  // Initialize Native Keyboard Simulation Stack
  ComputerKeyboard.begin();
  USB.begin();

  Serial.println("\n--- Tab5 Layout Array Bridge Online with Self-Healing Watchdog ---");
}

void loop() {
  Wire.beginTransmission(KEYBOARD_I2C_ADDR);
  Wire.write(REG_NORMAL_EVENT);
  uint8_t busStatus = Wire.endTransmission(true); 

  // Monitor transaction results
  if (busStatus == 0) {
    // Read the single packed event byte from the device
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1); 
    
    if (Wire.available() >= 1) {
      uint8_t eventByte = Wire.read();

      // Skip empty queue flags
      if (eventByte != 0xFF) {
        
        // Extract properties using the hardware bitmasks
        bool isPressed = (eventByte & 0x80) != 0;      // Bit 7 tracks state
        uint8_t row    = (eventByte & 0x70) >> 4;      // Bits 6-4 extract Row (0-4)
        uint8_t column = (eventByte & 0x0F);           // Bits 3-0 extract Column (0-13)

        // Compute row-major tracking index
        uint8_t keyIndex = (row * 14) + column;

        if (keyIndex < KEY_COUNT) {
          
          Serial.printf("[EVENT] Press: %d | Index: %d | Row: %d | Col: %d\n", isPressed, keyIndex, row, column);

          // Intercept physical modifier keys on press actions
          if (isPressed) {
            // Sym Key Hook (Row 3, Column 0 -> Index 42)
            if (keyIndex == 42) {
              symActive = !symActive;
              Serial.printf("[MODIFIER] Sym State: %s\n", symActive ? "ON" : "OFF");
              return;
            }
            // Aa / Shift Key Hook (Row 3, Column 1 -> Index 43)
            if (keyIndex == 43) {
              shiftActive = !shiftActive;
              Serial.printf("[MODIFIER] Shift State: %s\n", shiftActive ? "ON" : "OFF");
              return;
            }
            // Ctrl Key Hook (Row 4, Column 0 -> Index 56)
            if (keyIndex == 56) {
              ctrlActive = !ctrlActive;
              Serial.printf("[MODIFIER] Ctrl State: %s\n", ctrlActive ? "ON" : "OFF");
              return;
            }
            // Alt Key Hook (Row 4, Column 1 -> Index 57)
            if (keyIndex == 57) {
              altActive = !altActive;
              Serial.printf("[MODIFIER] Alt State: %s\n", altActive ? "ON" : "OFF");
              return;
            }

            // Extract target mappings from your configuration structure
            HidMapping targetKey;
            if (symActive) {
              targetKey = KEY_MATRIX_HID_SYM[keyIndex];
              symActive = false; // Auto-clear modifier latch
            } else {
              targetKey = KEY_MATRIX_HID_BASE[keyIndex];
            }

            if (targetKey.usageId != 0) {
              // Construct standard raw USB human interface descriptor payload packets
              KeyReport report;
              memset(&report, 0, sizeof(KeyReport));
              
              uint8_t activeModifiers = 0x00;

              // Apply pre-baked shift modifier flags or manual shift active toggle
              if (targetKey.modifier == 0x02 || shiftActive) {
                activeModifiers |= 0x02; // Left Shift bitmask
                shiftActive = false;     // Auto-clear layer modifier
              }

              // Apply Left Control bitmask flag if the modifier state is set
              if (ctrlActive) {
                activeModifiers |= 0x01; // Left Control bitmask
                ctrlActive = false;      // Auto-clear modifier after combining with character
              }

              // Apply Left Alt bitmask flag if the modifier state is set
              if (altActive) {
                activeModifiers |= 0x04; // Left Alt bitmask
                altActive = false;       // Auto-clear modifier after combining with character
              }
              
              report.modifiers = activeModifiers;
              report.keys[0] = targetKey.usageId; 
              ComputerKeyboard.sendReport(&report);
            }
          } 
          // Release keys cleanly on release notifications
          else {
            // Do not force clearance loops on modifier structural addresses
            if (keyIndex != 42 && keyIndex != 43 && keyIndex != 56 && keyIndex != 57) {
              ComputerKeyboard.releaseAll();
            }
          }
        }
      }
    }
  } else {
    // Trap bus failure immediately and pass along status code to trigger recovery
    checkAndRecoverBus(busStatus);
    delay(500); // Throttling delay to prevent terminal spam while wire remains loose
  }

  delay(15); 
}
