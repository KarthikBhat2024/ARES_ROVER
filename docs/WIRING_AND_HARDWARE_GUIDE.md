# ARES — Smart Six-Wheel Rescue Rover
## Complete Hardware, Electrical, and Wiring Engineering Guide

---

## 1. System Architecture & Critical Hardware Verification

The **ARES (Autonomous Response & Exploration System)** rover employs an ESP32 microcontroller coordinating three **L298N dual H-bridge motor drivers** to control six geared DC motors in a 6-wheel differential (skid-steer) configuration.

```
                  +--------------------------------------------------+
                  |         ARES ROVER HARDWARE ARCHITECTURE         |
                  +--------------------------------------------------+

                                 +--------------------+
                                 |  ESP32 Micro-      |
                                 |  controller        |
                                 +----+----+----+-----+
                                      |    |    |
           +--------------------------+    |    +--------------------------+
           | Logic Signals                 | Logic Signals                 | Logic Signals
           v                               v                               v
    +--------------+                +--------------+                +--------------+
    | L298N #1     |                | L298N #2     |                | L298N #3     |
    | (Front Axle) |                | (Middle Axle)|                | (Rear Axle)  |
    +---+------+---+                +---+------+---+                +---+------+---+
        |      |                        |      |                        |      |
        v      v                        v      v                        v      v
     [ M1 ]  [ M2 ]                  [ M3 ]  [ M4 ]                  [ M5 ]  [ M6 ]
     Front   Front                   Middle  Middle                  Rear    Rear
     Left    Right                   Left    Right                   Left    Right
```

---

## 2. Deep ESP32 Pinout Safety & Boot Strapping Analysis

The ESP32 microcontroller has specialized internal bootloader circuitry connected to specific pins called **Strapping Pins**. Using these pins carelessly will cause boot failures, unexpected motor jerking, or flash corruption.

### Analysis of Proposed Pin Table:

| Motor | Role | IN1 | IN2 | Enable/PWM | Pin Safety Assessment |
|---|---|---:|---:|---:|---|
| **M1** | Front Left | GPIO 13 | GPIO 14 | GPIO 25 | **SAFE.** General IO / DAC. |
| **M2** | Front Right | GPIO 26 | GPIO 27 | GPIO 33 | **SAFE.** General IO / ADC2. |
| **M3** | Middle Left | GPIO 16 | GPIO 17 | GPIO 32 | **SAFE.** General IO (UART2 pins). |
| **M4** | Middle Right| GPIO 18 | GPIO 19 | GPIO 23 | **SAFE.** General IO / VSPI pins. |
| **M5** | Rear Left | GPIO 21 | GPIO 22 | GPIO 4 | **SAFE.** General IO / I2C pins. |
| **M6** | Rear Right | **GPIO 5** | **GPIO 15** | **GPIO 2** | **CRITICAL WARNING:** All 3 pins are ESP32 strapping pins! |

### The Exact Dangers of Motor 6 Pins (GPIO 2, 5, 15):

1. **GPIO 2 (Strapping Pin & On-board LED):**
   - **Boot Function:** Controls download boot mode. Must be floating or pulled LOW during reset to flash/boot reliably.
   - **Hardware Conflict:** On many ESP32 development boards (NodeMCU-32S, DevKit V1), GPIO 2 is hardwired to an on-board blue surface-mount LED with a series resistor to GND.
   - **Risk on L298N ENB:** The L298N input stage has internal pull-up / pull-down circuitry. If connected to GPIO 2, it can bias the pin voltage during reset, causing the ESP32 to refuse to boot after programming, or become stuck in the ROM bootloader.
2. **GPIO 5 (Strapping Pin / SDIO Timing):**
   - **Boot Function:** Outputs a high-frequency PWM clock signal during the first few milliseconds of power-on while the ESP32 ROM bootloader executes.
   - **Risk on M6 IN1:** Because GPIO 5 pulses at boot, **Motor 6 will momentarily twitch, jerk, or kick** every time you power on or reset the rover, even before your code runs!
3. **GPIO 15 (Strapping Pin / Boot Debug TX):**
   - **Boot Function:** Outputs ROM boot debug messages at 115,200 baud upon power up.
   - **Risk on M6 IN2:** Rapid serial switching between 0V and 3.3V on IN2 combined with the clock pulse on IN1 means **Motor 6 will violently chatter during boot**!

---

## 3. Two Control Architectures: 3-Wire vs. 2-Wire PWM

We have engineered the firmware to support both modes via `#define MOTOR_CONTROL_MODE` in `config.h`.

### Architecture A: 3-Wire Control (Your Proposed Setup)
- Requires 3 pins per motor: `IN1` (Direction), `IN2` (Direction), `ENA/ENB` (PWM Speed).
- Total GPIOs required: **18 pins**.
- **MANDATORY HARDWARE RULE:** The black plastic jumpers on `ENA` and `ENB` of the L298N **MUST BE REMOVED**.
  > **DANGER:** If the ENA/ENB jumper remains installed, the pin is connected directly to the L298N's on-board 5V regulator. Connecting an ESP32 3.3V GPIO pin to a 5V-connected header can **permanently destroy the ESP32 GPIO pin** due to 5V over-voltage!
- **Limitation:** Leaves very few GPIOs for distance sensors, servos, cameras, headlights, and I2C peripherals.

### Architecture B: 2-Wire PWM Control (Highly Recommended Engineering Approach)
- Requires only 2 pins per motor: `IN1` and `IN2`.
- Total GPIOs required: **12 pins** (saving 6 GPIOs!).
- **How it works:**
  - Leave the black plastic jumpers **ON** `ENA` and `ENB` (keeps them tied to 5V permanently).
  - Apply PWM directly to `IN1` and `IN2`:
    - **Forward:** `PWM` on `IN1`, `LOW` on `IN2`.
    - **Reverse:** `LOW` on `IN1`, `PWM` on `IN2`.
    - **Stop:** `LOW` on `IN1`, `LOW` on `IN2`.
- **Advantages:**
  - Eliminates the need to use strapping pins GPIO 2, 5, and 15 entirely!
  - Eliminates all motor twitching at boot.
  - Frees up GPIO 21 & 22 for I2C (OLED screen, IMU / Gyroscope, PCA9685 servo expander).
  - Frees up GPIO 4 for ultrasonic distance sensors (HC-SR04).
  - Frees up ADC pins (GPIO 34, 35) for battery voltage monitoring.

---

## 4. Complete Wiring Tables

### Driver 1: Front Axle (M1 Front-Left & M2 Front-Right)

| L298N Board #1 Pin | Connects To | Wire Description |
|---|---|---|
| **ENA** | ESP32 **GPIO 25** | M1 Speed PWM *(Remove jumper if 3-wire)* |
| **IN1** | ESP32 **GPIO 13** | M1 Direction Input 1 |
| **IN2** | ESP32 **GPIO 14** | M1 Direction Input 2 |
| **IN3** | ESP32 **GPIO 26** | M2 Direction Input 1 |
| **IN4** | ESP32 **GPIO 27** | M2 Direction Input 2 |
| **ENB** | ESP32 **GPIO 33** | M2 Speed PWM *(Remove jumper if 3-wire)* |
| **OUT1 & OUT2** | Motor 1 (Front Left) | 2-pin DC motor leads |
| **OUT3 & OUT4** | Motor 2 (Front Right)| 2-pin DC motor leads |
| **+12V (Screw Terminal)** | Battery Positive (+) | External Motor Battery (7.4V - 12V) |
| **GND (Screw Terminal)** | Battery Negative (-) AND ESP32 GND | **COMMON GROUND** (Mandatory!) |
| **+5V (Screw Terminal)** | *Unconnected / Logic Output* | Output 5V if 5V jumper installed |

---

### Driver 2: Middle Axle (M3 Middle-Left & M4 Middle-Right)

| L298N Board #2 Pin | Connects To | Wire Description |
|---|---|---|
| **ENA** | ESP32 **GPIO 32** | M3 Speed PWM *(Remove jumper if 3-wire)* |
| **IN1** | ESP32 **GPIO 16** | M3 Direction Input 1 |
| **IN2** | ESP32 **GPIO 17** | M3 Direction Input 2 |
| **IN3** | ESP32 **GPIO 18** | M4 Direction Input 1 |
| **IN4** | ESP32 **GPIO 19** | M4 Direction Input 2 |
| **ENB** | ESP32 **GPIO 23** | M4 Speed PWM *(Remove jumper if 3-wire)* |
| **OUT1 & OUT2** | Motor 3 (Middle Left)| 2-pin DC motor leads |
| **OUT3 & OUT4** | Motor 4 (Middle Right)| 2-pin DC motor leads |
| **+12V (Screw Terminal)** | Battery Positive (+) | Shared battery bus (+) |
| **GND (Screw Terminal)** | Battery Negative (-) AND ESP32 GND | Shared common GND bus (-) |

---

### Driver 3: Rear Axle (M5 Rear-Left & M6 Rear-Right)

| L298N Board #3 Pin | Connects To | Wire Description |
|---|---|---|
| **ENA** | ESP32 **GPIO 4** | M5 Speed PWM *(Remove jumper if 3-wire)* |
| **IN1** | ESP32 **GPIO 21** | M5 Direction Input 1 |
| **IN2** | ESP32 **GPIO 22** | M5 Direction Input 2 |
| **IN3** | ESP32 **GPIO 5** | M6 Direction Input 1 *(Strapping pin)* |
| **IN4** | ESP32 **GPIO 15** | M6 Direction Input 2 *(Strapping pin)* |
| **ENB** | ESP32 **GPIO 2** | M6 Speed PWM *(Strapping pin)* |
| **OUT1 & OUT2** | Motor 5 (Rear Left) | 2-pin DC motor leads |
| **OUT3 & OUT4** | Motor 6 (Rear Right)| 2-pin DC motor leads |
| **+12V (Screw Terminal)** | Battery Positive (+) | Shared battery bus (+) |
| **GND (Screw Terminal)** | Battery Negative (-) AND ESP32 GND | Shared common GND bus (-) |

---

## 5. The Critical L298N Jumpers Explained

Each L298N module has three jumpers that beginners frequently misunderstand:

```
                  +-----------------------------------+
                  |        L298N JUMPER DIAGRAM       |
                  |                                   |
                  |   [ENA Jumper]     [ENB Jumper]   |
                  |   [   ::     ]     [   ::     ]   |
                  |                                   |
                  |         [78M05 5V Jumper]         |
                  |         [      ::       ]         |
                  |                                   |
                  |   +12V       GND        +5V       |
                  |  [ O ]      [ O ]      [ O ]      |
                  +-----------------------------------+
```

### 1. The 5V Regulator Jumper (Behind the blue screw terminals):
- **When Battery Voltage is 7V to 12V (e.g., 2S Li-ion 7.4V or 3S LiPo 11.1V):**
  - **LEAVE THIS JUMPER INSTALLED.**
  - The onboard 78M05 linear regulator drops the motor battery voltage down to 5V to power the L298N internal logic IC.
  - The `+5V` screw terminal can be used to output a small 5V logic supply (up to ~200mA).
- **When Battery Voltage is > 12V (e.g., 4S LiPo 14.8V or 16V):**
  - **REMOVE THIS JUMPER.** Voltages above 12V will overheat and destroy the 78M05 linear regulator. You must then supply an external clean 5V to the `+5V` terminal.

### 2. The ENA and ENB Jumpers:
- **For 3-Wire PWM Control:** You **MUST REMOVE** both jumpers. Plug ESP32 GPIOs directly into the top pin of the ENA/ENB header.
- **For 2-Wire PWM Control:** You **KEEP BOTH JUMPERS INSTALLED**.

---

## 6. Power Supply, Stall Currents, and Thermal Warnings

### The Common Ground Rule (MANDATORY):
You **MUST** connect the ESP32 `GND` pin to all three L298N `GND` terminals and the battery negative terminal. 
> Without a common ground reference, the 3.3V logic signals from the ESP32 will float relative to the driver, leading to random motor twitching, uncontrolled spinning, or total failure to respond.

### Never Power Motors from the ESP32:
- The ESP32's onboard 3.3V voltage regulator (usually an AMS1117-3.3 or ME6211) can deliver at most **500mA to 800mA**, which is barely enough for the ESP32 itself during Wi-Fi transmission bursts (up to 300mA peak!).
- Connecting even a single small DC motor to the ESP32 3.3V or VIN pin will trigger brownouts, continuous bootloops, or permanent board destruction.

### Motor Stall Currents & Battery Sizing:
- **Standard Yellow TT Gear Motors:**
  - Running current (free spin): ~150mA – 250mA each.
  - Stall current (under heavy load / stuck wheel): **0.8A – 1.2A each**.
  - **Total Rover Stall Demand:** 6 motors × 1.0A = **6.0 Amperes peak!**
- **Recommended Battery:**
  - **Do NOT use standard 9V alkaline rectangular batteries** (PP3). Their internal resistance is so high that drawing 500mA causes their voltage to drop below 3V immediately.
  - **Recommended:** 
    - 2S or 3S 18650 Li-ion battery pack (7.4V to 11.1V, rated for at least 10A continuous discharge).
    - Or a 3S LiPo battery (11.1V, 2200mAh 20C+).
- **Powering the ESP32 Board:**
  - Power the ESP32 either through its Micro-USB port via a standard 5V USB power bank, OR through an efficient **DC-DC Step-Down Buck Converter (e.g., LM2596 or MP1584)** stepping down the motor battery voltage to a steady 5.0V fed into the ESP32 `VIN` pin.

### Thermal & Efficiency Limitations of L298N:
- **Voltage Drop:** The L298N is built using bipolar junction transistors (BJTs), NOT modern MOSFETs. It has an unavoidable internal voltage drop of **1.5V to 2.2V** across the internal transistors!
  - If using a 7.4V battery, your 6V motors will only see `7.4V - 2.0V = 5.4V`.
  - If using a 12V battery, your motors will see `12V - 2.0V = 10.0V`.
- **Heat Dissipation:**
  - `P_heat = Voltage_Drop × Current = 2.0V × 1.5A ≈ 3.0 Watts per motor pair`.
  - Under continuous heavy load, the L298N aluminum heatsinks will become hot to the touch (50°C - 70°C). Ensure your rover chassis has adequate airflow.
- **Heavy Metal-Gear Motor Warning:**
  - If you upgrade to large metal-geared motors (e.g., 25GA or 37GB motors with stall currents > 3A), the L298N modules will trigger thermal shutdown or burn out. For high-torque motors, upgrade to MOSFET drivers like the **TB6612FNG**, **DRV8833**, or **BTS7960 43A H-Bridge**.

---

## 7. Step-by-Step Benchtop Testing Procedure

Before placing the rover on the ground, follow this verification checklist:

1. **Elevation:** Prop the rover up on a stand or block so that **all 6 wheels can spin freely in mid-air**.
2. **Flash Stage 1 Firmware:** Connect the ESP32 via USB and upload `ares_rover.ino`.
3. **Open Serial Monitor:** Set baud rate to **115,200 baud**.
4. **Automated Self-Test:** Type `t` in the Serial Monitor and press Enter.
   - The ESP32 will test Motor 1 through 6 sequentially at a safe 43% speed.
   - Verify that each wheel spins smoothly.
5. **Polarity Verification (Individual Tests):**
   - Type `m1+` to test M1 forward. Does Front-Left wheel rotate forward? If it rotates backwards, change `#define INVERT_M1 true` in `config.h`!
   - Repeat for `m2+` through `m6+`.
6. **Watchdog Verification:**
   - Type `w` (Drive Forward). Observe that the rover drives forward, and then automatically halts after 1.5 seconds when commands cease. This proves your safety watchdog is active and will prevent runaway rovers!
