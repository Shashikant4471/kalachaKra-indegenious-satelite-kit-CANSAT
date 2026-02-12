# CanSat Raspberry Pi - Setup Guide

## 📋 Prerequisites

1. Raspberry Pi 3 Model B+ with Raspbian/Raspberry Pi OS
2. SD card (8GB+) with OS installed
3. Monitor + keyboard (for initial setup) OR SSH access
4. Internet connection (for installing packages)

---

## STEP 1: Enable I2C and UART

### Open Raspberry Pi Configuration:
```bash
sudo raspi-config
```

1. Go to **Interface Options**
2. Enable **I2C** → Yes
3. Enable **Serial Port**:
   - "Login shell over serial?" → **No**
   - "Serial port hardware enabled?" → **Yes**
4. **Finish** → **Reboot**

### Verify I2C is working:
```bash
sudo apt install -y i2c-tools
i2cdetect -y 1
```

You should see your sensors at addresses **0x68** (MPU6050) and **0x0D** (QMC5883L).

---

## STEP 2: Install Python Dependencies

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install system packages
sudo apt install -y python3-pip python3-dev libgpiod2

# Install Python libraries
pip3 install smbus2 flask adafruit-circuitpython-dht pynmea2 pyserial
```

---

## STEP 3: Copy the Firmware

Copy `cansat_rpi.py` to your Raspberry Pi:

### Option A: USB drive
Copy the file to a USB stick → plug into Pi → copy to home folder

### Option B: SCP (from your PC)
```bash
scp cansat_rpi.py pi@<PI_IP_ADDRESS>:~/cansat_rpi.py
```

### Option C: Direct edit on Pi
```bash
nano ~/cansat_rpi.py
# Paste the code → Ctrl+X → Y → Enter
```

---

## STEP 4: Set Up WiFi Hotspot (Optional)

To make the Pi create its own WiFi network (like the ESP8266 did):

```bash
# Install hostapd and dnsmasq
sudo apt install -y hostapd dnsmasq

# Configure hostapd
sudo nano /etc/hostapd/hostapd.conf
```

Add this content:
```
interface=wlan0
driver=nl80211
ssid=CanSat-Telemetry
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=cansat2024
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
```

```bash
# Configure dnsmasq
sudo nano /etc/dnsmasq.conf
```

Add:
```
interface=wlan0
dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h
```

```bash
# Set static IP for wlan0
sudo nano /etc/dhcpcd.conf
```

Add at the bottom:
```
interface wlan0
static ip_address=192.168.4.1/24
nohook wpa_supplicant
```

```bash
# Enable and start
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl start hostapd
sudo systemctl restart dnsmasq
```

Now the Pi creates WiFi: **CanSat-Telemetry** (password: cansat2024)

---

## STEP 5: Run the Firmware

```bash
cd ~
sudo python3 cansat_rpi.py
```

> Use `sudo` because GPIO access requires root permissions.

You should see:
```
================================
   CanSat v1.0 (Raspberry Pi)
================================

[OK] smbus2 loaded
[OK] DHT library loaded
[OK] Serial library loaded
[OK] I2C bus initialized
[OK] MPU6050 initialized
[OK] QMC5883L initialized
[OK] DHT11 initialized on GPIO4
[OK] GPS initialized on /dev/serial0
[OK] LED initialized on GPIO17

================================
       MISSION STARTED!
================================

[OK] Web dashboard: http://192.168.4.1:8080
```

---

## STEP 6: View the Dashboard

1. Connect phone/laptop to **CanSat-Telemetry** WiFi
2. Open browser → **http://192.168.4.1:8080**
3. See live telemetry! 🎉

---

## 🔧 Auto-Start on Boot (Optional)

To run automatically when the Pi powers on:

```bash
sudo nano /etc/rc.local
```

Add before `exit 0`:
```bash
sudo python3 /home/pi/cansat_rpi.py &
```

---

## ❌ Troubleshooting

| Problem | Solution |
|---------|----------|
| "No module named smbus2" | Run: `pip3 install smbus2` |
| I2C sensors not found | Check wiring, run `i2cdetect -y 1` |
| GPS not reading | Enable UART in raspi-config |
| DHT errors | Normal - DHT11 occasionally fails |
| Can't access web page | Check Pi IP with `hostname -I` |
