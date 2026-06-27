# ESP32-S3 M5Stack Tab5 Keyboard HID Bridge

An ultra-low latency, self-healing hardware bridge that transforms the **M5Stack Tab5 Keyboard** into a native USB Human Interface Device (HID) using an **ESP32-S3-DevKitM-1**. 

This firmware utilizes the native USB-OTG peripheral layer of the ESP32-S3 to act as a hardware keyboard injector, running an optimized row-major matrix decoder with full **Shift (Aa)**, **Symbol (Sym)**, **Control (Ctrl)**, and **Alt** modifier support.

## ✨ Features

- **Native USB-OTG Keyboard Emulation**: Appears to computers/smartphones as a standard plug-and-play USB keyboard via TinyUSB hardware layers.
- **Row-Major Key Matrix Mapping**: Integrates the exact 70-key 14x5 structural HID layout map utilized by the physical Tab5.
- **Advanced Modifier Logic**: Full support for Shift, Sym, Ctrl, and Alt keys with latching state execution blocks.
- **Self-Healing I2C Watchdog**: Automatically detects loose wires, drops, or bus timeouts, completely resets the I2C interface, and re-initializes the keyboard smoothly without a manual restart.
- **PlatformIO Ready**: Built out for modern compilation and package handling inside Visual Studio Code.

---

## 🛠️ Hardware Requirements & Wiring

Connect your **M5Stack Tab5 Keyboard** directly to the **ESP32-S3-DevKitM-1** via its I2C rail:

| Tab5 Keyboard Pin | ESP32-S3 Pin | Description | Wire Notes |
| :--- | :--- | :--- | :--- |
| **5V** | **5V** | System Power Rail | Main power delivery from USB |
| **GND** | **G** | Ground Reference | Common ground plane connection |
| **SDA** | **GPIO 8** | Data Line | Default core S3 I2C Data Pin |
| **SCL** | **GPIO 9** | Clock Line | Default core S3 I2C Clock Pin |
| **INT** | **GPIO 10** | Interrupt Pin | Safely tracks low-signal hardware alerts |

> 💡 **Signal Integrity Note:** If you are running long wire extensions or custom cables and experience character skipping, place two external **4.7kΩ Pull-Up Resistors** (SDA to 5V and SCL to 5V) to stabilize bus voltage.

---

## 🔌 Understanding Your Board's Dual USB-C Ports

The ESP32-S3 DevKit houses two separate USB interfaces:
1. **USB-to-UART Port**: Wired to an onboard CP210x chip. Connect this to your PC to upload your firmware binary and track live telemetry logs in the PlatformIO Serial Monitor.
2. **Native USB Port (USB-OTG)**: Connected directly to the S3 core processor. Connect this to the target device (PC, Phone, or Tablet) where you want the Tab5 keyboard to type.

---

## 🚀 Getting Started (PlatformIO Installation)

### 1. Project Layout
Clone this repository and ensure your project structure matches the layout below:
```text
Tab5_S3_Bridge/
├── src/
│   └── main.cpp
└── platformio.ini
```

### 2. Configuration (`platformio.ini`)
Your environment properties must be configured exactly like this to properly allocate logging structures onto the UART chip while freeing the raw USB controller arrays:

```ini
[env:esp32-s3-devkitm-1]
platform = espressif32
board = esp32-s3-devkitm-1
framework = arduino

monitor_speed = 115200

build_flags = 
    -D ARDUINO_USB_CDC_ON_BOOT=0    ; Routes diagnostic logs away from native HID lines
    -D ARDUINO_USB_MSC_ON_BOOT=0
    -D ARDUINO_USB_DFU_ON_BOOT=0
    -D ARDUINO_USB_MODE=1          ; Activates raw hardware USB-OTG properties

src_filter = +<main.cpp>
```

### 3. Flash Instructions
1. Attach your computer to the **USB-to-UART** port of the DevKit.
2. If your device fails to link automatically, force it into ROM flashing mode manually:
   - Press and **hold the BOOT button**.
   - Tap the **RST button** once.
   - **Release the BOOT button**.
3. Inside VS Code, press `Ctrl+Alt+B` to compile the binary, then press `Ctrl+Alt+U` to upload.
4. Once completed, tap the **RST button** on the board once to exit bootloader mode and boot the application.

---

## 🔍 Verification & Diagnostics

1. Press `Ctrl+Alt+M` to open your PlatformIO Serial Monitor window via the UART port. You will see the initialization banner sequence print.
2. Plug a second USB data cable into the **Native USB** port of the DevKit and attach it to your target computer/phone interface.
3. Check the status indicator LED on the Tab5 keyboard. It should instantly switch to a **solid Blue**, confirming that the bridge has established an active connection over the I2C bus.
4. Open any text application and type. 

### 🛡️ Testing the Self-Healing Watchdog
If you disconnect a data jumper wire (such as SCL or SDA) while the device is running:
- The keyboard LED status light will immediately switch off or indicate an error.
- Your terminal will output `[WATCHDOG ALERT] Bus failure detected.`
- Simply slide the physical wire back into place. The automated recovery block resets the line registers, changes the keyboard light back to **solid Blue**, and lets you resume typing instantly without a manual board restart.

## 📄 License
This project is open-source and licensed under the MIT License.
