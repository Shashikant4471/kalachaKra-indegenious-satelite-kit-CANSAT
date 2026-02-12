# CanSat Parts List & Shopping Guide

## Components You Have ✓

| Component | Model | Quantity | Status |
|-----------|-------|----------|--------|
| Microcontroller | ESP8266 (NodeMCU/Wemos) | 1 | ✓ Have |
| Accelerometer | MPU6050 | 1 | ✓ Have |
| Magnetometer | HW-246 (QMC5883L) | 1 | ✓ Have |
| GPS Module | NEO-6M | 1 | ✓ Have |
| Touch Sensor | MPR121 | 1 | ✓ Have |
| Servo | SG90 Micro Servo | 1+ | ✓ Have |
| Battery | Li-ion (18650/LiPo) | 1 | ✓ Have |
| Temp/Humidity | DHT11/DHT22 | 1 | ✓ Have |
| Ultrasonic | HC-SR04 | 1 | ✓ Have |
| ATmega | Arduino Uno/Nano | 1 | ✓ Have (backup) |

---

## Recommended Additional Components 🛒

### Essential (Get These)

| Component | Purpose | Est. Price |
|-----------|---------|------------|
| **BMP180** | Barometric altitude (works to 9km) | $2-3 |
| **TP4056 Module** | Li-ion charging | $1-2 |
| **MT3608 Boost** | 3.7V → 5V for servo | $1-2 |
| **Perfboard** | Compact wiring | $1 |
| **Dupont Wires** | Connections | $2 |
| **330Ω Resistors** | LED current limiting | $1 |
| **10kΩ Resistors** | Pull-ups | $1 |

### Nice to Have

| Component | Purpose | Est. Price |
|-----------|---------|------------|
| **SD Card Module** | Data logging | $2-3 |
| **Buzzer** | Recovery beeper | $0.50 |
| **LoRa Module** | Long-range telemetry | $5-10 |

---

## Physical Build Materials

### Chassis (Coke Can Size: 66mm × 115mm)

| Option | Material | Difficulty |
|--------|----------|------------|
| 3D Print | PLA/PETG | Medium |
| PVC Pipe | 65mm diameter | Easy |
| Cardboard | Heavy cardboard tube | Easy |
| Acrylic | Cut pieces | Hard |

### Parachute Materials

| Item | Specification |
|------|---------------|
| Fabric | Nylon ripstop, 30cm diameter |
| Strings | Fishing line or thin cord, 4x 30cm |
| Attachment | Small swivel hook |

---

## Altitude Options

**You Have: NEO-6M GPS & Ultrasonic**
- GPS provides altitude at any height (needs clear sky)
- Ultrasonic accurate up to 4m (good for ground testing)

**Optional Future Upgrade: BMP180/BMP280**
- Barometric pressure → precise altitude
- Works indoors (no sky view needed)
- Range: 0-9000m

---

## Wiring Supply List

| Item | Quantity |
|------|----------|
| Male-to-Male dupont | 20 |
| Male-to-Female dupont | 20 |
| Heat shrink tubing | 1 pack |
| Electrical tape | 1 roll |
| Hot glue sticks | 5 |
| Zip ties (small) | 10 |

---

## Assembly Order

1. **Test components individually** on breadboard
2. **Plan layout** on paper (fit in 66mm cylinder)
3. **Solder** components to perfboard
4. **Wire** and hot glue secure
5. **Mount** in chassis
6. **Add** parachute mechanism
7. **Test** complete system
