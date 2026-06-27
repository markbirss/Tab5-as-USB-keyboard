# ESP32-S3 M5Stack Tab5 Keyboard HID Bridge

An ultra-low latency, self-healing hardware bridge that transforms the **M5Stack Tab5 Keyboard** into a native USB Human Interface Device (HID) using an **ESP32-S3-DevKitM-1**. 

<img width="3213" height="1832" alt="Project Banner" src="https://github.com/user-attachments/assets/13cd1b84-2139-4877-8e5e-62c2dc9acf33" />

### 🗺️ Hardware Reference Map

| ESP32-S3 DevKit Pinout Reference | M5Stack Tab5 Mechanical Unit |
| :---: | :---: |
| [![ESP32-S3 DevKit Layout](https://github.com/user-attachments/assets/41c6e95d-75cd-426c-9e0f-f802caf7abae)](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitm-1/user_guide.html#hardware-reference) | [![M5Stack Tab5 Layout](https://github.com/user-attachments/assets/afd5b47d-35e6-4fc1-bbc6-6e8a60b8b79c)](https://docs.m5stack.com/en/tab5/Tab5_Keyboard) |
| [Espressif Hardware Specs](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitm-1/user_guide.html#hardware-reference) | [M5Stack Product Documentation](https://docs.m5stack.com/en/tab5/Tab5_Keyboard) |

---

This firmware utilizes the native USB-OTG peripheral layer of the ESP32-S3 to act as a hardware keyboard injector, running an optimized row-major matrix decoder with full **Shift (Aa)**, **Symbol (Sym)**, **Control (Ctrl)**, and **Alt** modifier support.

## ✨ Features

- **Plug-and-Play HID**: Emulates standard hardware keyboards using native TinyUSB stacks.
- **Row-Major Key Mapping**: Replicates the physical 70-key 14x5 Tab5 matrix layout.
- **Latching Modifier Logic**: Full state execution for Shift, Sym, Ctrl, and Alt keys.
- **Self-Healing Watchdog**: Automatically resets and re-initializes crashed or disconnected I2C buses.
- **PlatformIO Integrated**: Configured for modern compilation and flashing inside VS Code.

---

## 🛠️ Hardware Requirements & Wiring

Connect the **M5Stack Tab5 Keyboard** directly to the **ESP32-S3-DevKitM-1** I2C rail:

| Tab5 Keyboard Pin | ESP32-S3 Pin | Description | Wire Notes |
| :--- | :--- | :--- | :--- |
| **3V3** | **3V3** | Power Rail | Main power supply |
| **GND** | **G** | Ground | Common ground plane connection |
| **SDA** | **GPIO 8** | Data Line | Default core S3 I2C Data Pin |
| **SCL** | **GPIO 9** | Clock Line | Default core S3 I2C Clock Pin |
| **INT** | **GPIO 10** | Interrupt Pin | Tracks hardware alerts |

> ⚠️ **Signal Integrity Note:** For long wires, add two external **4.7kΩ Pull-Up Resistors** (SDA to 3V3 and SCL to 3V3) to prevent character skipping.

### 📐 2x5 Interface Pinout (P1 Connector)

When bypassing the Tab5 mainboard, tap directly into the dual-row **2x5 interface connector (P1)** on the keyboard module alignment slot. Verify pin traces using the [M5Stack Tab5 Keyboard Schematic Sheet](https://docs.m5stack.com/en/tab5/Tab5_Keyboard). 

*Only connect the following 5 lines: 3V3, GND, SCL, SDA, and INT.*

<img width="565" height="367" alt="P1 Connector Pinout" src="https://github.com/user-attachments/assets/11e6cd90-2d4c-434f-9c31-fe18a08fe2b3" />

---

## 🔌 Port Configuration

The ESP32-S3 DevKit houses two separate micro USB interfaces:

* **USB-to-UART Port (CP210x):** Used for firmware flashing and tracking PlatformIO Serial Monitor logs.
* **Native USB Port (USB-OTG):** Used to inject keystrokes directly into the target device (PC, Phone, Tablet).

---

## 🚀 Getting Started (PlatformIO)

### 1. Project Structure
Clone this repository and verify your structural tree layout matches below:
```text
Tab5_S3_Bridge/
├── src/
│   └── main.cpp
└── platformio.ini
```

### 2. Properties Configuration (`platformio.ini`)
Isolate diagnostic logging from raw USB hardware controller arrays using this specific config:

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
1. Connect your PC to the **USB-to-UART** DevKit port.
2. Force manual ROM flashing mode if the automatic upload link fails:
   - Press and **hold the BOOT button**.
   - Tap the **RST button** once.
   - **Release the BOOT button**.

#### ⚡ Command Line Flash & Monitor
Compile, upload via the serial interface handler, and spin up an active terminal monitor sequence:

```bash
pio run -e esp32-s3-devkitm-1 -t upload --upload-port /dev/ttyUSB0; tio /dev/ttyUSB0
```
*(Windows users using VS Code: Compile via `Ctrl+Alt+B` and upload using `Ctrl+Alt+U`.)*

---

## 🔍 Verification & Diagnostics

1. Launch your terminal log tracker to view the initial board banner sequence.
2. Connect a secondary USB cable from the **Native USB-OTG** port to the target device.
3. Verify the Tab5 indicator LED turns **solid Blue** (confirms active I2C bus synchronization).
4. Open any text application and begin typing.

### 🛡️ Testing the Self-Healing Watchdog
Disconnect an I2C line (SCL or SDA) while running to verify recovery routines:

* The keyboard status LED will instantly cut out.
* The serial monitor will output `[WATCHDOG ALERT] Bus failure detected.`
* Reconnect the jumper wire; the bus will automatically reset and resume typing without a hard restart.

---

## 📄 License
This project is open-source software licensed under the **MIT License**.
