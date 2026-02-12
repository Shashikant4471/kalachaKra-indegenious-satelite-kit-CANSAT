# 🚀 Firmware Upload Guide - Step by Step (Beginner Friendly)

## What We're Doing

We need to:
1. Install **Arduino IDE** (the program to write & upload code)
2. Add **ESP8266 board support** (so Arduino knows about your chip)
3. Install **libraries** (extra code our sensors need)
4. **Upload** the firmware to your ESP8266

---

## STEP 1: Install Arduino IDE

### 1A: Download Arduino IDE

1. Open your browser
2. Go to: **https://www.arduino.cc/en/software**
3. Click **"Windows Win 10 and newer, 64 bits"**
4. Click **"Just Download"** (you don't need to donate)
5. Run the installer, click **Next → Next → Install → Finish**

### 1B: Open Arduino IDE

- Find **Arduino IDE** in your Start Menu and open it
- You should see a blank sketch window

---

## STEP 2: Add ESP8266 Board Support

Your ESP8266 isn't supported by default. We need to add it.

### 2A: Add the ESP8266 Board URL

1. In Arduino IDE, go to **File → Preferences**
2. Find the field: **"Additional Board Manager URLs"**
3. Paste this URL into that field:

```
http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

4. Click **OK**

```
┌─── Preferences ─────────────────────────────────────────┐
│                                                         │
│  Additional Board Manager URLs:                         │
│  ┌─────────────────────────────────────────────────────┐│
│  │ http://arduino.esp8266.com/stable/package_esp82...  ││
│  └─────────────────────────────────────────────────────┘│
│                                                         │
│                              [OK]  [Cancel]             │
└─────────────────────────────────────────────────────────┘
```

### 2B: Install ESP8266 Board Package

1. Go to **Tools → Board → Boards Manager** (or click the board icon on the left sidebar)
2. In the search box, type: **esp8266**
3. Find **"esp8266 by ESP8266 Community"**
4. Click **Install** (this may take a few minutes)
5. Wait for it to finish

```
┌─── Boards Manager ──────────────────────────────────┐
│  🔍 Search: esp8266                                 │
│                                                     │
│  esp8266 by ESP8266 Community        [INSTALL]      │
│  Boards included in this package:                   │
│  - NodeMCU 1.0                                      │
│  - Wemos D1 Mini                                    │
│  - Generic ESP8266 Module                           │
│  ...                                                │
└─────────────────────────────────────────────────────┘
```

### 2C: Select Your Board

1. Go to **Tools → Board → ESP8266 Boards**
2. Select **"NodeMCU 1.0 (ESP-12E Module)"**

> 💡 Even though yours is NodeMCU **V3**, the board selection is still "NodeMCU 1.0" — it works the same!

```
Tools → Board → ESP8266 Boards → NodeMCU 1.0 (ESP-12E Module)  ✅
```

---

## STEP 3: Install Required Libraries

Our code uses special libraries for the sensors. Install each one:

### How to Install a Library:

1. Go to **Sketch → Include Library → Manage Libraries**
2. Search for the library name
3. Click **Install**

### Libraries to Install:

| # | Search For | Library Name | What It's For |
|---|-----------|--------------|---------------|
| 1 | **DHT sensor** | "DHT sensor library" by Adafruit | Temperature sensor |
| 2 | **Adafruit Unified** | "Adafruit Unified Sensor" by Adafruit | Required by DHT |

> 💡 **Note:** The other libraries (ESP8266WiFi, Wire, Servo, SoftwareSerial) come pre-installed with the ESP8266 board package. You only need to install the 2 above!

### Step by Step:

```
1. Sketch → Include Library → Manage Libraries
2. Search: "DHT sensor library"
3. Find: "DHT sensor library" by Adafruit
4. Click [Install]
5. If asked to install dependencies → Click [Install All]

6. Search: "Adafruit Unified Sensor"
7. Find: "Adafruit Unified Sensor" by Adafruit
8. Click [Install]
```

---

## STEP 4: Open the Firmware File

1. In Arduino IDE, go to **File → Open**
2. Navigate to your project folder:

```
C:\Users\shash\Desktop\SATELITE\cansat_firmware\cansat_firmware.ino
```

3. Click **Open**
4. The code should appear in the editor

---

## STEP 5: Connect Your ESP8266

### 5A: Plug in the USB Cable

1. Connect the **micro-USB cable** to your ESP8266
2. Connect the other end to your **PC**
3. Wait a few seconds for Windows to detect it

### 5B: Install CH340 Driver (NodeMCU V3)

Your NodeMCU V3 uses a **CH340** USB-to-Serial chip. You need its driver:

1. Go to: **https://www.wch-ic.com/downloads/CH341SER_EXE.html**
2. Click **Download** to get `CH341SER.EXE`
3. Run the installer → Click **Install**
4. **Unplug and replug** the ESP8266

> 💡 After installing, Windows should show a notification like "USB-SERIAL CH340 (COM3)"

### 5C: Select the COM Port

1. Go to **Tools → Port**
2. You should see something like **"COM3"** or **"COM4"**
3. Select the one that appeared when you plugged in the ESP8266

```
Tools → Port → COM3 (or whichever number appears)  ✅
```

> 💡 **Not sure which port?** Unplug the ESP8266, check Tools → Port. Then plug it back in and check again. The NEW port is your ESP8266.

---

## STEP 6: Configure Upload Settings

Go to **Tools** and verify these settings:

| Setting | Value |
|---------|-------|
| **Board** | NodeMCU 1.0 (ESP-12E Module) |
| **Upload Speed** | 115200 |
| **CPU Frequency** | 80 MHz |
| **Flash Size** | 4MB (FS:2MB OTA:~1019KB) |
| **Port** | COM3 (your port number) |

```
Tools Menu Should Look Like:
─────────────────────────────────
Board:          NodeMCU 1.0 (ESP-12E Module)  ✅
Upload Speed:   115200                         ✅
CPU Frequency:  80 MHz                         ✅
Flash Size:     4MB (FS:2MB OTA:~1019KB)       ✅
Port:           COM3                           ✅
```

---

## STEP 7: Upload the Firmware! 🚀

### 7A: Click Upload

1. Click the **→ (right arrow) button** at the top of Arduino IDE
   - Or go to **Sketch → Upload**
   - Or press **Ctrl + U**

```
┌──────────────────────────────────────────────┐
│  ✓ Verify    → Upload    ...                 │
│              ↑                               │
│         CLICK THIS BUTTON                    │
└──────────────────────────────────────────────┘
```

### 7B: Wait for Compilation

The bottom panel will show messages:

```
Compiling sketch...
Compiling libraries...
Compiling core...
Linking everything together...
```

This takes **1-2 minutes** the first time. Be patient! ⏳

### 7C: Watch for Upload Progress

After compiling, you'll see:

```
Uploading...
Writing at 0x00000000... (3%)
Writing at 0x00004000... (6%)
...
Writing at 0x000fc000... (100%)

Done uploading.  ✅
```

> ⚠️ **If upload fails**, try pressing the **FLASH** button on the ESP8266 while uploading.

---

## STEP 8: Verify It's Working!

### 8A: Open Serial Monitor

1. Go to **Tools → Serial Monitor**
   - Or press **Ctrl + Shift + M**
2. Set baud rate to **115200** (bottom-right dropdown)

### 8B: What You Should See:

```
================================
   CanSat v1.1 - Initializing   
================================

[OK] MPU6050 initialized
[OK] QMC5883L initialized
[OK] DHT sensor ready
[OK] GPS initialized
[OK] WiFi AP created: CanSat-Telemetry
[OK] IP Address: 192.168.4.1

================================
       MISSION STARTED!         
================================
```

### 8C: Test the Web Interface

1. On your **phone**, go to WiFi settings
2. Connect to: **CanSat-Telemetry**
3. Password: **cansat2024**
4. Open browser → go to: **http://192.168.4.1**
5. You should see the live telemetry dashboard! 🎉

---

## ❌ Troubleshooting

### "Board not found" / No COM port
- Install the USB driver (Step 5B)
- Try a different USB cable (some are charge-only!)
- Try a different USB port

### "Compilation error"
- Make sure you installed BOTH libraries (Step 3)
- Make sure you selected the right board (Step 2C)

### "Upload failed"
- Hold the **FLASH** button on ESP8266 while clicking Upload
- Try changing Upload Speed to **9600** in Tools
- Make sure the correct COM port is selected

### "No sensor data showing"
- Check your wiring connections
- Check Serial Monitor for error messages
- Make sure I2C sensors are on D1 (SCL) and D2 (SDA)

---

## ✅ Summary Checklist

- [ ] Arduino IDE installed
- [ ] ESP8266 board URL added to Preferences
- [ ] ESP8266 board package installed
- [ ] Board set to "NodeMCU 1.0 (ESP-12E Module)"
- [ ] DHT sensor library installed
- [ ] Adafruit Unified Sensor library installed
- [ ] Firmware file opened
- [ ] ESP8266 connected via USB
- [ ] COM port selected
- [ ] Upload successful
- [ ] Serial Monitor shows sensor data
- [ ] Phone connected to CanSat-Telemetry WiFi
- [ ] Web dashboard loaded at 192.168.4.1

**Congratulations! Your CanSat is alive! 🛰️🎉**
