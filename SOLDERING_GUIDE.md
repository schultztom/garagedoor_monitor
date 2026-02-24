# MPU-6050 to Wemos D1 (ESP8266) Soldering Guide

## ⚠️ CRITICAL WARNINGS

1. **VOLTAGE:** MPU-6050's VCC pin requires **3.3V** - Connect to Wemos D1's **"3V3" pin** (NOT the "5V" pin!)
2. **TERMINOLOGY:** MPU-6050 pin is labeled "VCC" but needs 3.3V (NOT 5V despite VCC traditionally meaning 5V)
3. **DOUBLE CHECK:** Verify all connections before powering on
4. **POLARITY:** Reversing VCC/GND will destroy components
5. **STATIC:** Use anti-static precautions when handling boards

---

## Visual Wiring Diagram

```
    MPU-6050 Module                    Wemos D1 R1 (ESP8266)
    ┌──────────────┐                  ┌──────────────────┐
    │              │                  │   [USB Port]     │
    │    MPU-6050  │                  │                  │
    │              │                  │                  │
    └──────────────┘                  │                  │
         │││││                        │                  │
         │││││                        │                  │
    ┌────┴┴┴┴┴────┐                  │                  │
    │ VCC          │ ──RED WIRE────► │ 3V3 ◄── USE THIS │
    │ GND          │ ──BLACK WIRE──► │ GND              │
    │ SCL          │ ──WHITE WIRE──► │ D1 (GPIO 5)      │
    │ SDA          │ ──GREEN WIRE──► │ D2 (GPIO 4)      │
    │ XDA          │   (not used)     │                  │
    │ XCL          │   (not used)     │ 5V ◄── AVOID!    │
    │ AD0          │   (not used)     │                  │
    │ INT          │   (not used)     │ D5 (GPIO 14)     │
    └──────────────┘                  │   ↓              │
                                      │   Door Trigger   │
                                      └──────────────────┘
```

## Pin Connection Table

### MPU-6050 → Wemos D1 R1 Connections

| Cable Color | MPU-6050 Pin | Actual Voltage Need | → | Wemos D1 Pin | Voltage Output | Function |
|-------------|--------------|---------------------|---|--------------|----------------|----------|
| **RED**     | **VCC**      | 3.3V (2.375-3.46V)  | → | **3V3**      | 3.3V           | Power    |
| **BLACK**   | **GND**      | Ground              | → | **GND**      | Ground         | Ground   |
| **WHITE**   | **SCL**      | 3.3V logic          | → | **D1**       | GPIO 5         | I2C Clock|
| **GREEN**   | **SDA**      | 3.3V logic          | → | **D2**       | GPIO 4         | I2C Data |

**⚠️ CRITICAL:** MPU-6050's "VCC" pin is labeled VCC but requires 3.3V, NOT 5V!

---

## Step-by-Step Soldering Instructions

### What You Need:
- Soldering iron (300-350°C / 572-662°F)
- Solder (60/40 or 63/37 tin/lead, or lead-free)
- 4 wires: RED, BLACK, WHITE, GREEN (20-26 AWG recommended)
- Wire strippers
- Multimeter for testing
- Helping hands or vice to hold boards

### Cable Preparation:

1. **Cut 4 wires to appropriate length** (10-20cm / 4-8 inches depending on your setup)
   - RED wire
   - BLACK wire
   - WHITE wire
   - GREEN wire

2. **Strip both ends** of each wire (expose 3-5mm of bare wire)

3. **Pre-tin all wire ends and board pads:**
   - Heat wire end with soldering iron
   - Apply small amount of solder to bare wire
   - Repeat for all 8 wire ends (4 wires × 2 ends)
   - Pre-tin the pads on both MPU-6050 and Wemos D1

---

### CABLE 1: RED WIRE (Power)

**MPU-6050 Side:**
1. Locate the pin labeled **"VCC"** on MPU-6050 (usually the first or second pin)
2. Place RED wire's tinned end on the VCC pad/hole
3. Heat pad and wire together with soldering iron (2-3 seconds)
4. Solder should flow and create shiny joint
5. Remove iron, let cool for 5 seconds
6. Gently tug wire to verify solid connection

**Wemos D1 Side:**
1. Locate the pin labeled **"3V3"** or **"3.3V"** (near USB port, in power pins section)
2. **VERIFY it says "3V3", NOT "5V"** - this is critical!
3. Place RED wire's other tinned end on the 3V3 pad/hole
4. Heat pad and wire together (2-3 seconds)
5. Solder should flow and create shiny joint
6. Remove iron, let cool
7. Verify solid connection

---

### CABLE 2: BLACK WIRE (Ground)

**MPU-6050 Side:**
1. Locate the pin labeled **"GND"** on MPU-6050 (usually next to VCC)
2. Place BLACK wire's tinned end on the GND pad/hole
3. Heat and solder (2-3 seconds)
4. Let cool and verify connection

**Wemos D1 Side:**
1. Locate any pin labeled **"GND"** or **"G"** (there are usually multiple GND pins)
2. Place BLACK wire's other end on a GND pad/hole
3. Heat and solder (2-3 seconds)
4. Let cool and verify connection

---

### CABLE 3: WHITE WIRE (I2C Clock - SCL)

**MPU-6050 Side:**
1. Locate the pin labeled **"SCL"** on MPU-6050 (Serial Clock)
2. Place WHITE wire's tinned end on the SCL pad/hole
3. Heat and solder (2-3 seconds)
4. Let cool and verify connection

**Wemos D1 Side:**
1. Locate the pin labeled **"D1"** (this is GPIO 5)
2. **NOT** "D0", specifically **"D1"**
3. Place WHITE wire's other end on the D1 pad/hole
4. Heat and solder (2-3 seconds)
5. Let cool and verify connection

---

### CABLE 4: GREEN WIRE (I2C Data - SDA)

**MPU-6050 Side:**
1. Locate the pin labeled **"SDA"** on MPU-6050 (Serial Data)
2. Place GREEN wire's tinned end on the SDA pad/hole
3. Heat and solder (2-3 seconds)
4. Let cool and verify connection

**Wemos D1 Side:**
1. Locate the pin labeled **"D2"** (this is GPIO 4)
2. **NOT** "D1", specifically **"D2"**
3. Place GREEN wire's other end on the D2 pad/hole
4. Heat and solder (2-3 seconds)
5. Let cool and verify connection

---

## Summary: Where Each Cable Goes

| Cable Color | One End (MPU-6050) | Other End (Wemos D1) |
|-------------|--------------------|----------------------|
| **RED**     | VCC pin            | 3V3 pin ⚠️ (NOT 5V!) |
| **BLACK**   | GND pin            | GND pin              |
| **WHITE**   | SCL pin            | D1 pin (GPIO 5)      |
| **GREEN**   | SDA pin            | D2 pin (GPIO 4)      |

### Door Trigger Connection

| Function | → | Wemos D1 Pin | GPIO # | Notes |
|----------|---|--------------|--------|-------|
| **Door Trigger Output** | → | **D5** | GPIO 14 | Connect to garage door opener relay/control |

---

## Detailed Physical Connections

### 1. Power Connection (VCC)
- **MPU-6050 VCC pin** → **Wemos D1 "3V3" pin**
- Use **3.3V ONLY** - MPU-6050 is NOT 5V tolerant
- Solder a solid connection, use red wire for clarity
- The 3V3 pin is usually near the USB connector on Wemos D1

### 2. Ground Connection (GND)
- **MPU-6050 GND pin** → **Wemos D1 "GND" pin**
- Any GND pin on Wemos D1 works (there are multiple)
- Use black wire for clarity
- Ensure solid connection - this is your common reference

### 3. I2C Clock (SCL)
- **MPU-6050 SCL pin** → **Wemos D1 "D1" pin**
- D1 = GPIO 5
- This is the I2C clock line
- Use yellow wire (standard I2C color convention)

### 4. I2C Data (SDA)
- **MPU-6050 SDA pin** → **Wemos D1 "D2" pin**
- D2 = GPIO 4
- This is the I2C data line
- Use green wire (standard I2C color convention)

### 5. Door Trigger Output
- **Wemos D1 "D5" pin** → **Your garage door opener control input**
- D5 = GPIO 14
- This provides a 500ms HIGH pulse (3.3V) when triggered
- You may need a relay or optocoupler depending on your garage door opener

---

## Wemos D1 R1 Pin Layout Reference

```
                 [USB Port]
                     ||
        RST  [ ]   [ ]  TX
        A0   [ ]   [ ]  RX
        D0   [ ]   [ ]  D1  ← SCL (GPIO 5) - Connect MPU-6050 SCL here
        D5   [ ]   [ ]  D2  ← SDA (GPIO 4) - Connect MPU-6050 SDA here
        D6   [ ]   [ ]  D3
        D7   [ ]   [ ]  D4
        D8   [ ]   [ ]  GND ← Connect MPU-6050 GND here
        3V3  [ ]   [ ]  5V
             ↑
             └─ Connect MPU-6050 VCC here (3.3V)
```

**Note:** Pin positions may vary slightly by manufacturer. Always verify with your specific board's silkscreen labels.

---

## MPU-6050 Module Pin Layout

Most MPU-6050 breakout boards have this standard layout:

```
[ VCC ]  ← 3.3V from Wemos D1
[ GND ]  ← GND from Wemos D1
[ SCL ]  ← D1 (GPIO 5) from Wemos D1
[ SDA ]  ← D2 (GPIO 4) from Wemos D1
[ XDA ]  (not used)
[ XCL ]  (not used)
[ AD0 ]  (leave floating or connect to GND)
[ INT ]  (not used)
```

---

## Soldering Instructions

### Tools Required:
- Soldering iron (temperature: 350°C / 662°F)
- Rosin core solder (63/37 or 60/40 tin/lead or lead-free)
- Wire strippers
- Small gauge wire (22-26 AWG)
- Heat shrink tubing or electrical tape
- Helping hands or PCB holder
- Multimeter (for continuity checking)

### Soldering Steps:

#### Step 1: Prepare Wires
1. Cut 4 wires approximately 10-15 cm (4-6 inches) long
2. Strip 3-5mm of insulation from each end
3. Tin the stripped ends (coat with solder)
4. Use color-coded wires: RED (VCC), BLACK (GND), YELLOW (SCL), GREEN (SDA)

#### Step 2: Solder to MPU-6050
1. **Secure the MPU-6050** in helping hands or PCB holder
2. **VCC (Red wire):**
   - Place tinned wire end in VCC through-hole
   - Heat pad and wire simultaneously for 2-3 seconds
   - Apply solder to joint (not iron)
   - Remove iron and hold wire steady until solder solidifies
3. **Repeat for:** GND (black), SCL (yellow), SDA (green)
4. **Inspect joints:** Should be shiny, smooth, and cone-shaped

#### Step 3: Solder to Wemos D1
1. **Secure the Wemos D1** in helping hands
2. **Identify pins:** 3V3, GND, D1, D2
3. **Solder wires** to corresponding pins:
   - Red wire → 3V3 pin
   - Black wire → GND pin
   - Yellow wire → D1 pin
   - Green wire → D2 pin
4. **Use same technique:** Heat pad and wire, apply solder, remove heat

#### Step 4: Verify Connections
**BEFORE POWERING ON:**
1. **Visual inspection:** Check for cold solder joints, bridges, or loose connections
2. **Continuity test** with multimeter:
   - MPU-6050 VCC should connect to Wemos D1 3V3
   - MPU-6050 GND should connect to Wemos D1 GND
   - MPU-6050 SCL should connect to Wemos D1 D1
   - MPU-6050 SDA should connect to Wemos D1 D2
3. **Short check:** Verify NO continuity between VCC and GND
4. **Insulate exposed connections** with heat shrink or tape

---

## Mounting the MPU-6050

### Important Placement Guidelines:

1. **Mount on garage door itself** (not the wall or track)
2. **Orientation for closed door:**
   - Y-axis should be vertical (parallel to gravity)
   - Z-axis should be horizontal (perpendicular to door surface)
3. **Secure mounting:**
   - Use hot glue, epoxy, or small screws
   - Ensure sensor doesn't vibrate independently of door
   - Keep wires strain-relieved and secure

### Ideal Mounting Position:
```
                    Garage Door (Closed Position)
                    
                         ↑ Y-axis
                         |
         ================|================= ← Top of door
         |               |               |
         |         [MPU-6050]           |
         |               |               |
         |               → Z-axis        |
         |                               |
         |                               |
         |                               |
         ================================= ← Bottom of door
```

**When door is closed:**
- Y-axis acceleration ≈ 9.8 m/s² (vertical, feeling gravity)
- Z-axis acceleration ≈ 0 m/s² (horizontal)

**When door is fully open (horizontal):**
- Y-axis acceleration ≈ 0 m/s²
- Z-axis acceleration ≈ 9.8 m/s² (now vertical, feeling gravity)

---

## Door Trigger Wiring

### Option 1: Direct Connection (If your door opener accepts 3.3V logic)
```
Wemos D1 D5 (GPIO 14) → Door Opener Control Input
Wemos D1 GND         → Door Opener Ground
```

### Option 2: Using a Relay Module (Recommended)
```
Wemos D1 D5 (GPIO 14) → Relay Module IN
Wemos D1 GND         → Relay Module GND
Wemos D1 3V3         → Relay Module VCC
Relay NO/COM         → Garage Door Opener Terminals (parallel with wall button)
```

**Relay Module Advantages:**
- Isolates ESP8266 from door opener voltage
- Compatible with any garage door opener
- Simulates physical button press

---

## Testing Procedure

### 1. Visual Inspection
- [ ] All solder joints are shiny and solid
- [ ] No solder bridges between pins
- [ ] Wires are correct colors on correct pins
- [ ] No exposed wire strands that could short

### 2. Electrical Testing (Multimeter)
- [ ] Measure voltage at MPU-6050 VCC: Should be 3.3V when powered
- [ ] Measure continuity: VCC to 3V3 pin
- [ ] Measure continuity: GND to GND pin
- [ ] Measure continuity: SCL to D1 pin
- [ ] Measure continuity: SDA to D2 pin
- [ ] Verify NO continuity between VCC and GND

### 3. Initial Power-Up
- [ ] Connect Wemos D1 to USB (no garage door connection yet)
- [ ] Check serial monitor at 115200 baud
- [ ] Should see: "Initializing MPU6050..."
- [ ] Should see: "MPU6050 Found!"
- [ ] Should see: "WiFi connected!"
- [ ] Should see: IP address printed

### 4. Web Interface Test
- [ ] Open web browser to ESP8266 IP address
- [ ] Verify status page loads
- [ ] Check Y and Z acceleration readings update
- [ ] Y should be ~9.8 when sensor is vertical
- [ ] Z should be ~9.8 when sensor is horizontal
- [ ] Sensor Health should show "OK"

### 5. Movement Test
- [ ] Tilt/move the sensor
- [ ] Verify acceleration values change
- [ ] Check that status updates (OPENING/CLOSING detected)

---

## Troubleshooting

### "Failed to find MPU6050 chip"
- **Check wiring:** Verify SDA and SCL connections
- **Check power:** Ensure 3.3V at MPU-6050 VCC pin
- **Check I2C address:** MPU-6050 default is 0x68 (AD0 to GND) or 0x69 (AD0 to VCC)
- **Try swapping:** SDA and SCL (common mistake)

### No WiFi Connection
- **Check credentials:** Verify WIFI_SSID and WIFI_PASSWORD in platformio.ini match your network
- **Signal strength:** Ensure ESP8266 is within WiFi range
- **2.4GHz only:** ESP8266 does not support 5GHz networks

### Web Interface Not Loading
- **Find IP address:** Check serial monitor for IP
- **Ping test:** Try pinging the IP address
- **Firewall:** Check if firewall is blocking port 80

### Incorrect Readings
- **Orientation:** Ensure sensor is mounted in correct orientation
- **Calibration:** May need to adjust tolerance values
- **Vibration:** Ensure sensor is firmly mounted to door

---

## Final Safety Checks

### Before Closing the Enclosure:
- [ ] All connections are secure and insulated
- [ ] No loose wire strands
- [ ] Wires are strain-relieved
- [ ] MPU-6050 is firmly mounted
- [ ] ESP8266 is accessible for USB programming
- [ ] Ventilation for heat dissipation

### Before Connecting to Garage Door:
- [ ] Test trigger output with multimeter (should pulse 3.3V)
- [ ] Verify 500ms pulse duration
- [ ] Understand your garage door opener wiring
- [ ] Use proper isolation (relay recommended)
- [ ] Test manually before automation

---

## Support Information

### Default Configuration Values:
- **Closed Position:** Y = 9.8 m/s², Z = 0 m/s²
- **Open Position:** Y = 0 m/s², Z = 9.8 m/s²
- **Position Tolerance:** ± 0.5 m/s²
- **Movement Threshold:** 0.5 m/s²
- **Stop Timeout:** 2 seconds
- **Max Open/Close Time:** 30 seconds
- **Stall Timeout:** 5 seconds

### WiFi Credentials (in platformio.ini):
```ini
'-DWIFI_SSID="YourNetworkName"'
'-DWIFI_PASSWORD="YourNetworkPassword"'
```

---

**Document Version:** 1.0  
**Last Updated:** December 20, 2025  
**Project:** Garage Door Monitor with MPU-6050
