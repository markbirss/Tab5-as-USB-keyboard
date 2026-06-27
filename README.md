# ESP32-S3 M5Stack Tab5 Keyboard HID Bridge

An ultra-low latency, self-healing hardware bridge that transforms the **M5Stack Tab5 Keyboard** into a native USB Human Interface Device (HID) using an **ESP32-S3-DevKitM-1**. 

<img width="3213" height="1832" alt="20260627_160042" src="https://github.com/user-attachments/assets/13cd1b84-2139-4877-8e5e-62c2dc9acf33" />

| ESP32-S3 DevKit Pinout Reference | M5Stack Tab5 Mechanical Unit |
| :---: | :---: |
| [![ESP32-S3 DevKit Layout](https://github.com/user-attachments/assets/41c6e95d-75cd-426c-9e0f-f802caf7abae)](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitm-1/user_guide.html#hardware-reference) | [![M5Stack Tab5 Layout](https://github.com/user-attachments/assets/afd5b47d-35e6-4fc1-bbc6-6e8a60b8b79c)](https://docs.m5stack.com/en/tab5/Tab5_Keyboard) |
| [Espressif Hardware Specs](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitm-1/user_guide.html#hardware-reference) | [M5Stack Product Documentation](https://docs.m5stack.com/en/tab5/Tab5_Keyboard) |

---

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
| **3V3** | **3V3** | Power Rail | Tab5 power |
| **GND** | **G** | Ground Reference | Common ground plane connection |
| **SDA** | **GPIO 8** | Data Line | Default core S3 I2C Data Pin |
| **SCL** | **GPIO 9** | Clock Line | Default core S3 I2C Clock Pin |
| **INT** | **GPIO 10** | Interrupt Pin | Safely tracks low-signal hardware alerts |

> 💡 **Signal Integrity Note:** If you are running long wire extensions or custom cables and experience character skipping, place two external **4.7kΩ Pull-Up Resistors** (SDA to 3V3 and SCL to 3V3) to stabilize bus voltage.

### 📐 2x5 Interface Pinout (P1 Connector)
When breaking out connections without the Tab5 mainboard, you will be tapping directly into the dual-row **2x5 interface connector (P1)** located on the keyboard module alignment slot. To trace the trace lines or find specific pin arrays, verify with the official wiring diagrams available via the [M5Stack Tab5 Keyboard Schematic Sheet](https://docs.m5stack.com/en/tab5/Tab5_Keyboard).

You are only connecting VCC_3V3, GND, SCL, SDA and INT.
in my top picture i used
3V3	Purple
GND	White and Green

SDA	Yellow
SCL	Orange
INT	Blue


<img width="565" height="367" alt="image" src="https://github.com/user-attachments/assets/11e6cd90-2d4c-434f-9c31-fe18a08fe2b3" />

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

#### ⚡ Command Line Flash & Monitor
To quickly compile, upload via your designated Linux/macOS serial device handler port (`/dev/ttyUSB0`), and instantly switch into terminal monitoring tracking loops via `tio`, execute:

```bash
pio run -e esp32-s3-devkitm-1 -t upload --upload-port /dev/ttyUSB0; tio /dev/ttyUSB0
```

*(Note: Windows users using VS Code can compile by hitting `Ctrl+Alt+B` and upload with `Ctrl+Alt+U`.)*

---

## 🔍 Verification & Diagnostics

1. Launch your terminal log tracking stack. You will see the initialization banner sequence print.
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
