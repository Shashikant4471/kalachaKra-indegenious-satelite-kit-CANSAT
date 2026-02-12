#!/usr/bin/env python3
"""
CanSat Firmware v1.0 - Raspberry Pi 3 Model B+
Satellite in a Coke Can

Components:
- Raspberry Pi 3 Model B+ (main controller + WiFi)
- MPU6050 Accelerometer/Gyro (I2C)
- HW-246 / QMC5883L Magnetometer (I2C)
- DHT11/22 Temperature & Humidity (GPIO)
- NEO-6M GPS Module (UART)
- LED Status Indicator (GPIO)

Dependencies:
    pip3 install smbus2 flask adafruit-circuitpython-dht pynmea2 pyserial
"""

import time
import math
import json
import threading
import struct
from flask import Flask, jsonify, render_template_string

# ============== SENSOR IMPORTS ==============
try:
    import smbus2 as smbus
    print("[OK] smbus2 loaded")
except ImportError:
    import smbus
    print("[OK] smbus loaded (fallback)")

try:
    import adafruit_dht
    import board
    print("[OK] DHT library loaded")
except ImportError:
    adafruit_dht = None
    print("[WARN] DHT library not available")

try:
    import serial
    print("[OK] Serial library loaded")
except ImportError:
    serial = None
    print("[WARN] Serial library not available")

try:
    import pynmea2
    print("[OK] pynmea2 GPS parser loaded")
except ImportError:
    pynmea2 = None
    print("[WARN] pynmea2 not available")

try:
    import RPi.GPIO as GPIO
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    print("[OK] GPIO loaded")
except ImportError:
    GPIO = None
    print("[WARN] GPIO not available (not on Pi?)")


# ============== PIN DEFINITIONS ==============
# Using BCM pin numbering
LED_PIN = 17        # GPIO17 (Pin 11)
DHT_PIN = 4         # GPIO4  (Pin 7) - DHT11 data

# I2C is on GPIO2 (SDA) and GPIO3 (SCL) - Bus 1
I2C_BUS = 1

# GPS is on UART: /dev/serial0
GPS_PORT = "/dev/serial0"
GPS_BAUD = 9600

# ============== I2C ADDRESSES ==============
MPU6050_ADDR = 0x68
QMC5883L_ADDR = 0x0D

# ============== MISSION SETTINGS ==============
TELEMETRY_INTERVAL = 0.1  # 100ms between readings
WIFI_SSID = "CanSat-Telemetry"
WIFI_PASS = "cansat2024"
WEB_PORT = 8080

# ============== SENSOR DATA ==============
telemetry = {
    # Accelerometer
    "accelX": 0.0, "accelY": 0.0, "accelZ": 0.0,
    "roll": 0.0, "pitch": 0.0,
    # Magnetometer
    "magX": 0.0, "magY": 0.0, "magZ": 0.0,
    "heading": 0.0,
    # Environment
    "temperature": 0.0, "humidity": 0.0,
    # GPS
    "latitude": 0.0, "longitude": 0.0,
    "gpsAltitude": 0.0, "gpsSpeed": 0.0,
    "gpsSatellites": 0, "gpsValid": False,
    # System
    "missionTime": 0,
    "parachuteDeployed": False,
    "status": "INITIALIZING"
}

mission_start_time = time.time()


# ============== I2C BUS ==============
try:
    bus = smbus.SMBus(I2C_BUS)
    print("[OK] I2C bus initialized")
except Exception as e:
    bus = None
    print(f"[ERR] I2C bus failed: {e}")


# ============== MPU6050 ACCELEROMETER ==============
def init_mpu6050():
    """Initialize MPU6050 - wake it up from sleep mode"""
    if bus is None:
        return False
    try:
        bus.write_byte_data(MPU6050_ADDR, 0x6B, 0x00)  # Wake up
        bus.write_byte_data(MPU6050_ADDR, 0x1C, 0x00)  # ±2g range
        print("[OK] MPU6050 initialized")
        return True
    except Exception as e:
        print(f"[ERR] MPU6050 init failed: {e}")
        return False


def read_mpu6050():
    """Read accelerometer data from MPU6050"""
    if bus is None:
        return
    try:
        # Read 6 bytes starting from register 0x3B
        data = bus.read_i2c_block_data(MPU6050_ADDR, 0x3B, 6)

        # Convert to signed 16-bit values
        raw_x = struct.unpack('>h', bytes(data[0:2]))[0]
        raw_y = struct.unpack('>h', bytes(data[2:4]))[0]
        raw_z = struct.unpack('>h', bytes(data[4:6]))[0]

        # Convert to g (±2g range, 16384 LSB/g)
        telemetry["accelX"] = round(raw_x / 16384.0, 3)
        telemetry["accelY"] = round(raw_y / 16384.0, 3)
        telemetry["accelZ"] = round(raw_z / 16384.0, 3)

        # Calculate roll and pitch
        ax = telemetry["accelX"]
        ay = telemetry["accelY"]
        az = telemetry["accelZ"]

        telemetry["roll"] = round(math.atan2(ay, az) * 180.0 / math.pi, 2)
        telemetry["pitch"] = round(
            math.atan2(-ax, math.sqrt(ay * ay + az * az)) * 180.0 / math.pi, 2
        )
    except Exception as e:
        print(f"[ERR] MPU6050 read: {e}")


# ============== QMC5883L MAGNETOMETER (HW-246) ==============
def init_qmc5883l():
    """Initialize QMC5883L magnetometer"""
    if bus is None:
        return False
    try:
        # Set/Reset period register
        bus.write_byte_data(QMC5883L_ADDR, 0x0B, 0x01)
        # Control register: continuous mode, 200Hz, 8G, OSR=512
        bus.write_byte_data(QMC5883L_ADDR, 0x09, 0x1D)
        print("[OK] QMC5883L initialized")
        return True
    except Exception as e:
        print(f"[ERR] QMC5883L init failed: {e}")
        return False


def read_qmc5883l():
    """Read magnetometer data from QMC5883L"""
    if bus is None:
        return
    try:
        # Read 6 bytes starting from register 0x00
        data = bus.read_i2c_block_data(QMC5883L_ADDR, 0x00, 6)

        # QMC5883L is little-endian (different from HMC5883L!)
        raw_x = struct.unpack('<h', bytes(data[0:2]))[0]
        raw_y = struct.unpack('<h', bytes(data[2:4]))[0]
        raw_z = struct.unpack('<h', bytes(data[4:6]))[0]

        telemetry["magX"] = raw_x
        telemetry["magY"] = raw_y
        telemetry["magZ"] = raw_z

        # Calculate heading
        heading = math.atan2(raw_y, raw_x) * 180.0 / math.pi
        if heading < 0:
            heading += 360
        telemetry["heading"] = round(heading, 2)
    except Exception as e:
        print(f"[ERR] QMC5883L read: {e}")


# ============== DHT11 TEMPERATURE & HUMIDITY ==============
dht_device = None

def init_dht():
    """Initialize DHT11 sensor"""
    global dht_device
    if adafruit_dht is None:
        print("[WARN] DHT library not installed")
        return False
    try:
        dht_device = adafruit_dht.DHT11(board.D4)
        print("[OK] DHT11 initialized on GPIO4")
        return True
    except Exception as e:
        print(f"[ERR] DHT init failed: {e}")
        return False


def read_dht():
    """Read temperature and humidity from DHT11"""
    if dht_device is None:
        return
    try:
        telemetry["temperature"] = round(dht_device.temperature, 2)
        telemetry["humidity"] = round(dht_device.humidity, 2)
    except RuntimeError:
        pass  # DHT11 occasionally fails, just skip
    except Exception as e:
        print(f"[ERR] DHT read: {e}")


# ============== GPS MODULE (NEO-6M) ==============
gps_serial = None

def init_gps():
    """Initialize GPS serial connection"""
    global gps_serial
    if serial is None:
        print("[WARN] Serial library not installed")
        return False
    try:
        gps_serial = serial.Serial(GPS_PORT, GPS_BAUD, timeout=1)
        print(f"[OK] GPS initialized on {GPS_PORT}")
        return True
    except Exception as e:
        print(f"[ERR] GPS init failed: {e}")
        return False


def read_gps():
    """Read and parse GPS data"""
    if gps_serial is None or pynmea2 is None:
        return
    try:
        while gps_serial.in_waiting > 0:
            line = gps_serial.readline().decode('ascii', errors='replace').strip()
            if line.startswith('$GPGGA') or line.startswith('$GPRMC'):
                try:
                    msg = pynmea2.parse(line)
                    if hasattr(msg, 'latitude') and msg.latitude:
                        telemetry["latitude"] = round(msg.latitude, 6)
                        telemetry["longitude"] = round(msg.longitude, 6)
                        telemetry["gpsValid"] = True
                    if hasattr(msg, 'altitude') and msg.altitude:
                        telemetry["gpsAltitude"] = round(float(msg.altitude), 1)
                    if hasattr(msg, 'spd_over_grnd') and msg.spd_over_grnd:
                        telemetry["gpsSpeed"] = round(float(msg.spd_over_grnd) * 1.852, 1)
                    if hasattr(msg, 'num_sats'):
                        telemetry["gpsSatellites"] = int(msg.num_sats)
                except pynmea2.ParseError:
                    pass
    except Exception as e:
        print(f"[ERR] GPS read: {e}")


# ============== LED STATUS ==============
def init_led():
    """Initialize status LED"""
    if GPIO is None:
        return False
    try:
        GPIO.setup(LED_PIN, GPIO.OUT)
        GPIO.output(LED_PIN, GPIO.LOW)
        print(f"[OK] LED initialized on GPIO{LED_PIN}")
        return True
    except Exception as e:
        print(f"[ERR] LED init failed: {e}")
        return False


def blink_led():
    """Toggle LED state"""
    if GPIO is None:
        return
    try:
        current = GPIO.input(LED_PIN)
        GPIO.output(LED_PIN, not current)
    except:
        pass


# ============== WEB SERVER (Flask) ==============
app = Flask(__name__)

# HTML Dashboard (same design as ESP8266 version)
DASHBOARD_HTML = """
<!DOCTYPE html>
<html>
<head>
    <title>CanSat Ground Station</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: 'Segoe UI', sans-serif;
            background: linear-gradient(135deg, #0c0c1e 0%, #1a1a3e 100%);
            color: #fff;
            min-height: 100vh;
            padding: 20px;
        }
        h1 {
            text-align: center;
            font-size: 1.5em;
            margin-bottom: 20px;
            background: linear-gradient(90deg, #00d2ff, #7b2ff7);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 15px;
            max-width: 900px;
            margin: 0 auto;
        }
        .card {
            background: rgba(255,255,255,0.05);
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 12px;
            padding: 15px;
            backdrop-filter: blur(10px);
        }
        .card h3 {
            font-size: 0.9em;
            color: #888;
            margin-bottom: 10px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .value {
            font-size: 2em;
            font-weight: bold;
            color: #00d2ff;
        }
        .value.warn { color: #ff6b6b; }
        .small { font-size: 0.9em; color: #aaa; margin-top: 5px; }
        .status {
            text-align: center;
            padding: 10px;
            border-radius: 8px;
            font-weight: bold;
            margin-bottom: 15px;
        }
        .status.ok { background: rgba(0,210,100,0.2); color: #00d264; }
        .status.warn { background: rgba(255,107,107,0.2); color: #ff6b6b; }
        .btn {
            display: block;
            width: 100%;
            padding: 12px;
            margin-top: 15px;
            border: none;
            border-radius: 8px;
            font-size: 1em;
            cursor: pointer;
            font-weight: bold;
        }
        .btn-deploy {
            background: linear-gradient(135deg, #ff416c, #ff4b2b);
            color: white;
        }
    </style>
</head>
<body>
    <h1>🛰️ CanSat Ground Station (RPi)</h1>
    <div id="status" class="status ok">CONNECTED - Reading sensors...</div>
    <div class="grid">
        <div class="card">
            <h3>🌡️ Temperature</h3>
            <div class="value"><span id="temp">--</span>°C</div>
            <div class="small">Humidity: <span id="hum">--</span>%</div>
        </div>
        <div class="card">
            <h3>🧭 Heading</h3>
            <div class="value"><span id="hdg">--</span>°</div>
            <div class="small">Roll: <span id="roll">--</span>° | Pitch: <span id="pitch">--</span>°</div>
        </div>
        <div class="card">
            <h3>📡 GPS</h3>
            <div class="value" style="font-size:1.2em"><span id="lat">--</span>, <span id="lon">--</span></div>
            <div class="small">Alt: <span id="gpsAlt">--</span>m | Sats: <span id="gpsSat">--</span></div>
        </div>
        <div class="card">
            <h3>⏱️ Mission Time</h3>
            <div class="value"><span id="time">0</span>s</div>
            <div class="small">Chute: <span id="chute">NOT DEPLOYED</span></div>
        </div>
        <div class="card">
            <h3>📊 Acceleration</h3>
            <div class="small" style="font-size:1.1em">
                X: <span id="ax">--</span>g<br>
                Y: <span id="ay">--</span>g<br>
                Z: <span id="az">--</span>g
            </div>
        </div>
    </div>
    <button class="btn btn-deploy" onclick="deploy()">🪂 DEPLOY PARACHUTE</button>

    <script>
        function update() {
            fetch('/data')
                .then(r => r.json())
                .then(d => {
                    document.getElementById('temp').textContent = d.temp?.toFixed(1) || '--';
                    document.getElementById('hum').textContent = d.hum?.toFixed(1) || '--';
                    document.getElementById('hdg').textContent = d.hdg?.toFixed(1) || '--';
                    document.getElementById('roll').textContent = d.roll?.toFixed(1) || '--';
                    document.getElementById('pitch').textContent = d.pitch?.toFixed(1) || '--';
                    document.getElementById('lat').textContent = d.lat?.toFixed(6) || '--';
                    document.getElementById('lon').textContent = d.lon?.toFixed(6) || '--';
                    document.getElementById('gpsAlt').textContent = d.gpsAlt?.toFixed(1) || '--';
                    document.getElementById('gpsSat').textContent = d.gpsSat || '--';
                    document.getElementById('time').textContent = (d.time / 1000).toFixed(1);
                    document.getElementById('chute').textContent = d.chute ? 'DEPLOYED' : 'NOT DEPLOYED';
                    document.getElementById('ax').textContent = d.ax?.toFixed(3) || '--';
                    document.getElementById('ay').textContent = d.ay?.toFixed(3) || '--';
                    document.getElementById('az').textContent = d.az?.toFixed(3) || '--';
                })
                .catch(e => {
                    document.getElementById('status').className = 'status warn';
                    document.getElementById('status').textContent = 'CONNECTION LOST';
                });
        }
        function deploy() {
            if (confirm('Deploy parachute?')) {
                fetch('/deploy').then(() => update());
            }
        }
        setInterval(update, 200);
        update();
    </script>
</body>
</html>
"""

@app.route('/')
def index():
    return render_template_string(DASHBOARD_HTML)

@app.route('/data')
def data():
    return jsonify({
        "temp": telemetry["temperature"],
        "hum": telemetry["humidity"],
        "hdg": telemetry["heading"],
        "roll": telemetry["roll"],
        "pitch": telemetry["pitch"],
        "ax": telemetry["accelX"],
        "ay": telemetry["accelY"],
        "az": telemetry["accelZ"],
        "time": int((time.time() - mission_start_time) * 1000),
        "chute": telemetry["parachuteDeployed"],
        "lat": telemetry["latitude"],
        "lon": telemetry["longitude"],
        "gpsAlt": telemetry["gpsAltitude"],
        "gpsSpd": telemetry["gpsSpeed"],
        "gpsSat": telemetry["gpsSatellites"],
        "gpsValid": telemetry["gpsValid"]
    })

@app.route('/deploy')
def deploy():
    telemetry["parachuteDeployed"] = True
    telemetry["status"] = "PARACHUTE DEPLOYED"
    print("[!] PARACHUTE DEPLOYED!")
    return "OK"

@app.route('/reset')
def reset():
    telemetry["parachuteDeployed"] = False
    telemetry["status"] = "READY"
    return "OK"


# ============== SENSOR READING LOOP ==============
def sensor_loop():
    """Main sensor reading loop - runs in separate thread"""
    global mission_start_time
    mission_start_time = time.time()
    telemetry["status"] = "RUNNING"

    print("\n================================")
    print("       MISSION STARTED!         ")
    print("================================\n")

    while True:
        try:
            # Read all sensors
            read_mpu6050()
            read_qmc5883l()
            read_dht()
            read_gps()

            # Update mission time
            telemetry["missionTime"] = int((time.time() - mission_start_time) * 1000)

            # Heartbeat LED
            blink_led()

            # Serial telemetry
            elapsed = telemetry["missionTime"] / 1000.0
            print(f"T:{elapsed:.1f}s | Temp:{telemetry['temperature']}C | "
                  f"Hum:{telemetry['humidity']}% | Hdg:{telemetry['heading']}°")

            time.sleep(TELEMETRY_INTERVAL)

        except KeyboardInterrupt:
            print("\n[!] Mission stopped by user")
            break
        except Exception as e:
            print(f"[ERR] Loop error: {e}")
            time.sleep(1)


# ============== MAIN ==============
def main():
    print("\n================================")
    print("   CanSat v1.0 (Raspberry Pi)  ")
    print("================================\n")

    # Initialize all sensors
    init_mpu6050()
    init_qmc5883l()
    init_dht()
    init_gps()
    init_led()

    # Blink LED to show ready
    if GPIO:
        for i in range(6):
            blink_led()
            time.sleep(0.2)

    # Start sensor loop in background thread
    sensor_thread = threading.Thread(target=sensor_loop, daemon=True)
    sensor_thread.start()

    # Start web server
    print(f"\n[OK] Web dashboard: http://<Pi-IP>:{WEB_PORT}")
    print(f"[OK] Or connect to WiFi hotspot: {WIFI_SSID}")
    print(f"[OK] Then open: http://192.168.4.1:{WEB_PORT}\n")

    app.run(host='0.0.0.0', port=WEB_PORT, debug=False)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n[!] Shutting down...")
    finally:
        if GPIO:
            GPIO.cleanup()
        print("[OK] Cleanup complete. Goodbye!")
