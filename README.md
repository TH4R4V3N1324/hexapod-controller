# Hexapod Controller

Embedded firmware and hardware-support code for a hexapod robot controller. The repository contains firmware for the handheld/controller interface, an ESP32 wireless receiver, ATtiny1616 left and right boards, and a newer ESP32-S3/LVGL controller implementation.

The system is primarily written in C/C++ and uses PlatformIO, Arduino, and ESP-IDF. Communication is built around ESP-NOW for wireless control packets and I²C for exchanging controller and robot-state data.

## Features

- ESP-NOW wireless control communication
- I²C communication between the receiver and a controller/master device
- Packed control and robot-state packet formats
- Legacy ESP32-S2 Arduino controller firmware
- ESP32 receiver firmware
- ATtiny1616 left and right board firmware
- ESP32-S3 controller V2 architecture
- LVGL-based display/UI support
- LCD, touchscreen, IMU, RTC, battery, buzzer, SD-card, and wireless subsystems in V2
- Hardware schematics and UI design assets

## Repository layout

```text
controller/
  controller/
    include/              Legacy controller headers
      ControllerLogic.h   High-level controller behavior
      DataPacket.h        Packet definitions
      Display.h           Display abstraction
      I2CManager.h        I²C communication
      Input.h             Joystick and button input
      Pages.h             UI page definitions
      sensors/            Sensor interfaces
    src/                  Legacy controller implementation
      main.cpp            Application entry point
      ControllerLogic.cpp Controller state and behavior
      DataPacket.cpp      Packet handling
      Display.cpp         Display implementation
      I2CManager.cpp      I²C implementation
      Input.cpp           Input handling
      Pages.cpp           Page rendering
      sensors/            Sensor implementations
  schematic/              Hardware schematics
  photoshopPages/         UI design assets
  screenImages/           Display graphics

controllerV2/
  Left_Board/             ATtiny1616 left-board firmware
  Right_Board/            ATtiny1616 right-board firmware
  Animations/             Controller animation assets
  controllerV2/           ESP-IDF ESP32-S3 controller project
    main/                 Application drivers and subsystems
      BAT_Driver/         Battery monitoring
      Buzzer/             Buzzer control
      Controller_Logic/   Controller application logic
      Data_Packet/        Packet handling
      Display/             Display subsystem
      EXIO/               Extended I/O
      I2C_Driver/         I²C driver
      LCD_Driver/         LCD hardware driver
      LVGL_Driver/        LVGL integration
      LVGL_UI/            User interface
      PCF85063/           RTC driver
      QMI8658/            IMU driver
      SD_Card/            SD-card support
      Touch_Driver/       Touchscreen input
      UI_Animation/       UI animation logic
      Wireless/           Wireless communication
    components/lvgl/      LVGL component

receiver/
  include/                Receiver headers and packet definitions
    CommEspNow.h          ESP-NOW communication
    CommI2C.h             I²C callbacks
    ControlPacket.h       Wireless control packet format
    HexPacket.h           Robot-state packet format
  src/main.cpp            Receiver entry point
  platformio.ini          ESP32 DevKit configuration
```

## System architecture

The controller sends a packed `ControlPacket` containing joystick values, robot height, a command, and command arguments. The receiver validates the packet size, updates its control state, and sends the latest `HexPacket` back to the configured controller using ESP-NOW.

The receiver also exposes the current `ControlPacket` over I²C and accepts updated `HexPacket` data from the I²C master. I²C is configured as slave address `0x08`.

```text
Controller UI
     │
     │ ESP-NOW: ControlPacket
     ▼
ESP32 receiver
     │
     ├── ESP-NOW: HexPacket response
     │
     └── I²C slave 0x08
             │
             ▼
       Controller/master
```

## Packet formats

Both packets use packed structures so their byte layouts remain consistent across devices.

### ControlPacket

```cpp
struct ControlPacket {
    int16_t joystick1X;
    int16_t joystick1Y;
    int16_t joystick2X;
    int16_t joystick2Y;
    int16_t currentHeight;
    Command command;
    int16_t commandArgs[3];
};
```

Available commands:

```text
CMD_NONE
CMD_SET_GAIT
CMD_SET_MODE
CMD_SET_CONFIG
CMD_HOME_STANCE
CMD_REQUEST_CONFIG
```

### HexPacket

```cpp
struct HexPacket {
    int16_t legConfigs[3];
    int16_t currentHeight;
    int16_t currentPhase;
    int16_t currentGait;
    int16_t currentMode;
};
```

The receiver rejects packets whose size does not exactly match the expected structure. Keep field order and types synchronized across all firmware projects.

## Requirements

- Git
- PlatformIO CLI or PlatformIO for VS Code
- ESP-IDF for `controllerV2/controllerV2`
- A compatible programmer and USB/serial connection for the target board
- Hardware matching the selected firmware target

## Building and uploading

### Legacy ESP32-S2 controller

```bash
cd controller/controller
pio run
pio run --target upload
pio device monitor --baud 115200
```

Target configuration:

- Board: `esp32-s2-saola-1`
- Framework: Arduino
- Display library: `olikraus/U8g2`
- Serial monitor: `115200` baud

The PlatformIO configuration currently uses `COM4` as the upload port. Change `upload_port` in `controller/controller/platformio.ini` for your machine.

### ESP32 receiver

```bash
cd receiver
pio run
pio run --target upload
pio device monitor --baud 115200
```

The receiver targets an `esp32dev` board with the Arduino framework.

Before using the receiver, verify the peer MAC address in `receiver/include/CommEspNow.h`:

```cpp
uint8_t controllerMAC[] = {
    0xFC, 0x01, 0x2C, 0xD9, 0x41, 0xE4
};
```

The current implementation uses unencrypted ESP-NOW and a hard-coded peer address.

### ATtiny1616 left board

```bash
cd controllerV2/Left_Board
pio run
pio run --target upload
```

The project is configured for:

- Board: `ATtiny1616`
- Framework: Arduino
- CPU frequency: `20 MHz`
- Upload protocol: SerialUPDI
- Default upload port: `COM3`

Update `upload_port` in `platformio.ini` when necessary.

### ATtiny1616 right board

```bash
cd controllerV2/Right_Board
pio run
pio run --target upload
```

The right board uses the same ATtiny1616, Arduino, 20 MHz, and SerialUPDI configuration as the left board.

### ESP32-S3 controller V2

```bash
cd controllerV2/controllerV2
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash
idf.py -p PORT monitor
```

Or run the complete workflow with:

```bash
idf.py -p PORT build flash monitor
```

The V2 project is organized as an ESP-IDF application with LVGL, RGB LCD, touch input, wireless communication, battery monitoring, RTC, IMU, SD-card, buzzer, and I²C subsystems.

## Development notes

- Do not change packet field order or types without updating every sender and receiver.
- `ControlPacket` and `HexPacket` use `#pragma pack(push, 1)` to prevent compiler padding.
- ESP-NOW packets are validated by exact byte length.
- The receiver uses I²C address `0x08`.
- Serial communication is configured for `115200` baud.
- The PlatformIO projects contain Windows-specific ports such as `COM3` and `COM4`; replace them on other platforms.
- The ESP-NOW peer is currently configured without encryption.
- The V2 controller includes an inherited ESP-IDF RGB LCD example README; the V2 source tree contains the project-specific drivers and UI components described above.

## Troubleshooting

### ESP-NOW packets are rejected

Check that the sender and receiver use the same packed structure definition, field order, and integer widths. The receiver logs the expected and received packet sizes over serial.

### The receiver cannot send a response

Verify that:

1. The controller MAC address in `receiver/include/CommEspNow.h` is correct.
2. ESP-NOW initializes successfully.
3. The controller is configured on the same Wi-Fi channel.
4. The peer has been added successfully.

### I²C data is not exchanged

Verify the wiring, ground connection, master/slave roles, and that the master addresses the receiver at `0x08`. Also confirm that the transmitted data size equals `sizeof(ControlPacket)` or `sizeof(HexPacket)` as appropriate.

### Upload fails

Check the selected serial port, programmer wiring, board power, and upload protocol. The ATtiny1616 projects use SerialUPDI, while the ESP32 projects use their PlatformIO or ESP-IDF upload workflows.

## License

No license file is currently included in the repository. Unless a license is added, the code should be treated as all rights reserved.

## Contributing

1. Create a feature branch.
2. Make changes within the relevant firmware project.
3. Build the affected PlatformIO or ESP-IDF target.
4. Test packet compatibility and hardware communication.
5. Open a pull request with the target board and test setup described.
