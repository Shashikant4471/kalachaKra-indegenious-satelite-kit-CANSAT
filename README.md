# Kalachakra Indigenous Satellite Kit -(CANSAT)

Welcome to the **Kalachakra Indigenous Satellite Kit** project! This repository contains the firmware, software, and documentation for a custom CanSat (Satellite in a Can) built for educational and terrain scanning purposes.

## Project Overview

This project aims to build a low-cost, functional CanSat capable of:
- **Telemetry Transmission**: Sending real-time data to a ground station which visualizes the data in 3D.
- **Terrain Scanning**: Using an array of ultrasonic sensors to map the ground below.
- **Environmental Monitoring**: Collecting temperature, humidity, and pressure data.
- **GPS Tracking**: Logging coordinates for flight path analysis.

## Hardware Specifications

### Flight Computer
- **Microcontroller**: Arduino Mega 2560 (Primary) / ESP8266 (Wi-Fi Telemetry)
- **IMU**: MPU6050 (Accelerometer + Gyroscope)
- **Magnetometer**: HW-246 (QMC5883L)
- **GPS**: NEO-6M
- **Sensors**: 
  - 6x HC-SR04 Ultrasonic Sensors (Terrain Scanning)
  - DHT11/22 (Temperature & Humidity)
  - BMP180/280 (Barometric Pressure/Altitude - *Planned*)

## Landing & Terrain Scanning Mechanism

The CanSat features an intelligent terrain scanning system to identify safe landing zones.

### How it Works
1. **Sensor Array**: 5 ultrasonic sensors are arranged in a cross pattern (Center + 4 Corners).
2. **Scanning**: During descent, the system continuously pulses all sensors to detect the distance to the ground.
3. **Analysis**:
   - The onboard processor calculates the **variance** between the 5 sensor readings.
   - **Flat/Safe**: If all sensors read similar distances (low variance), the ground is flat.
   - **Uneven/Hazard**: If sensors have largely different readings (high variance), it indicates a slope, rocks, or obstacles.
4. **Decision**: The system outputs a status (`FLAT-SAFE`, `UNEVEN`, `HAZARD!`) which is transmitted to the ground station.

### Landing Gear
- **Parachute**: Deployed via a servo mechanism at apogee or predetermined altitude.
- **Legs**: (Optional) Static landing legs to protect the sensor array upon impact.

## Power & Actuation
- **Power Source**: Li-ion 18650 Battery
- **Servo**: SG90 Micro Servo (Parachute release mechanism)

## Software Stack
- **Firmware**: Arduino / C++
- **Ground Station**: Python (Terrain Viewer, Telemetry Dashboard)
- **Design & Docs**: Markdown, Fritzing/Breadboard diagrams

## Getting Started

1. **Firmware Upload**: Follow the guides in `docs/firmware_upload_guide.md` to flash the Arduino and ESP8266.
2. **Wiring**: Refer to `docs/wiring_diagram.md` for proper connections.
3. **Running the Ground Station**:
   ```bash
   # Activate virtual environment
   .\.venv\Scripts\activate
   # Run the terrain viewer
   python cansat_arduino/terrain_viewer.py
   ```

## Repository Structure
- `/cansat_arduino`: Arduino sketches and Python visualization tools.
- `/cansat_firmware`: Esp8266/Telemetery specific firmware.
- `/docs`: Documentation, wiring diagrams, and parts lists.
