# hexapod-controller

Firmware for the hexapod's handheld controllers, and for the ESP32 receiver that links them to the
Servo2040 stack. A controller sends joysticks and commands over **ESP-NOW**. It talks to the
[ESP32 stack](https://github.com/TH4R4V3N1324/hexapod-esp32) directly. The
[Servo2040 stack](https://github.com/TH4R4V3N1324/hexapod-servo2040) goes through the receiver,
which passes them on over I²C.

The robot, its build, and how the stacks fit together are documented in
[hexapod-docs](https://github.com/TH4R4V3N1324/hexapod-docs). The controllers' PCBs are in
[hexapod-hardware](https://github.com/TH4R4V3N1324/hexapod-hardware/tree/main/controllers). This
README only covers building and working on the firmware.

> **Status:** on hold. The OLED controller was built for an earlier prototype. It now speaks the
> current packet layout, but it hasn't been tested against either robot stack yet.

## Projects

| Folder | What it is | Board | Toolchain |
| --- | --- | --- | --- |
| [`oled-controller/`](oled-controller/) | **OLED controller**: two sticks, buttons, encoder, menus on a 128×64 OLED | [OLED controller](https://github.com/TH4R4V3N1324/hexapod-hardware/tree/main/controllers/oled-controller) PCB, rev v3 (ESP32-S2) | PlatformIO, Arduino |
| [`receiver/`](receiver/) | ESP-NOW in, I²C out, for the Servo2040 stack | Any ESP32 dev board (`esp32dev`) | PlatformIO, Arduino |
| [`touch-controller/display/`](touch-controller/display/) | **Touch controller** (experimental): LVGL touchscreen UI, IMU, RTC, SD card | ESP32-S3 2.8" RGB LCD board | ESP-IDF 5.3, LVGL 8.4 |
| [`touch-controller/left-board/`, `right-board/`](touch-controller/) | The touch controller's side boards: a stick each, plus a D-pad (left) or encoder (right), polled by the display over UART | ATtiny1616 | PlatformIO, Arduino (megaTinyCore) |

The OLED controller and the receiver are the working pair. The touch controller is an unfinished
redesign and has no published PCB.

## Quick start

You need [PlatformIO](https://platformio.org/) (the VS Code extension or the CLI). Both projects pin
the `espressif32` platform at 6.11.0 (Arduino-ESP32 2.0.17), the same as hexapod-esp32.

```sh
git clone --recursive https://github.com/TH4R4V3N1324/hexapod-controller.git
cd hexapod-controller/oled-controller     # or receiver
pio run -t upload
pio device monitor
```

`--recursive` fetches LVGL (the `external/lvgl` submodule), which only the touch controller needs.
In an existing clone, run `git submodule update --init`.

Then pair the two ends (see [Pairing](#pairing)): each side only talks to the MAC address it's
built with.

The touch controller's display board needs [ESP-IDF](https://docs.espressif.com/projects/esp-idf/) 5.3:

```sh
cd touch-controller/display
idf.py set-target esp32s3     # first time only
idf.py -p PORT build flash monitor
```

The side boards are programmed through a SerialUPDI adapter: `pio run -t upload` in `left-board/`
or `right-board/`.

## Hardware

| Firmware | PCB revision | Hardware files |
| --- | --- | --- |
| `oled-controller/` | OLED controller, rev **v3** (ESP32-S2) | [`controllers/oled-controller`](https://github.com/TH4R4V3N1324/hexapod-hardware/tree/main/controllers/oled-controller) |
| `receiver/` | None (stock ESP32 DevKit) | — |
| `touch-controller/` | ESP32-S3 display board plus two ATtiny1616 side boards | Not published |

When a board revision changes, update this table and the matching one in the hardware repo.

**Receiver wiring** (Servo2040 stack): ESP32 GPIO 21 (SDA) and 22 (SCL) to the Servo 2040's GPIO 20
and 21, and a shared ground. The Servo 2040 is the I²C master at 100 kHz; the receiver is the slave at
`0x08`.

## Repository layout

```text
common/
  HexapodPackets.h     ControlPacket, HexPacket, Command, Gait, Mode: shared with the robot firmware
external/
  lvgl/                Submodule, LVGL v8.4.0 (touch controller only)
oled-controller/       OLED controller (ESP32-S2, PCB rev v3)
  include/, src/
    main.cpp           Entry point and loop
    ControllerLogic    Page navigation; turns button presses into commands
    Input              Sticks (centred at boot, ±128, deadzone), buttons and encoder
    DataPacket         ESP-NOW: sends a ControlPacket every 10 ms, receives HexPackets
    Display, Pages     Menus on the SSD1309 128×64 OLED (U8g2)
    bitmaps.h          Screen icons, compiled in
    sensors/           BMI330 IMU and INA260 battery current sensor
  assets/
    screens/           Icon PNGs, the sources of bitmaps.h
    psd/               Page mock-ups and fonts
receiver/              Servo2040-stack receiver (ESP32)
  include/
    CommEspNow.h       ESP-NOW, paired-controller check, link-loss failsafe
    CommI2C.h          I²C slave at 0x08
  src/main.cpp
touch-controller/      Touch controller (experimental)
  display/             ESP-IDF project (ESP32-S3); main/ has one folder per driver or subsystem
  left-board/, right-board/  Side boards (ATtiny1616)
  animations/          Idle animation frames (PNG sources and .bin for the SD card)
```

## How it works

```text
ESP32 stack:      controller ──ESP-NOW──► robot's ESP32
                             ◄─HexPacket─

Servo2040 stack:  controller ──ESP-NOW──► receiver ──I²C──► Servo 2040
                             ◄─HexPacket─          ◄─HexPacket─
```

The controller sends a `ControlPacket` every 10 ms, and the robot answers each one with a
`HexPacket`.

The receiver keeps the latest `ControlPacket` for the Servo 2040 to read, and the latest
`HexPacket` the Servo 2040 wrote, which it sends back with its next reply to the controller.

**Failsafe:** if no packet comes from the controller for **250 ms**, the receiver centres both
sticks and clears the command, so the robot stops walking instead of repeating the last input.
Height is kept, so the robot doesn't drop. The failsafe is on from boot until the first packet.
The ESP32 stack has its own 200 ms timeout.

## Robot interface

The packets are in [`common/HexapodPackets.h`](common/HexapodPackets.h), the only copy in this
repo. All three projects that use them include it. It must stay byte-compatible with the robot
firmware's `DataPacket.h` in
[hexapod-esp32](https://github.com/TH4R4V3N1324/hexapod-esp32/blob/main/include/DataPacket.h) and
[hexapod-servo2040](https://github.com/TH4R4V3N1324/hexapod-servo2040/blob/main/includes/DataPacket.h).
The robot firmware is the reference. `static_assert`s on the sizes catch accidental changes.

```cpp
struct ControlPacket {           // controller → robot, 17 bytes
    int16_t joystick1X, joystick1Y, joystick2X, joystick2Y;  // ±128, X right, Y forward
    int16_t currentHeight;       // body height in mm, 0 = unchanged
    Command command;
    int16_t commandArgs[3];
};

struct HexPacket {               // robot → controller, 11 bytes, in reply to each packet
    int16_t legConfigs[3];       // calibration offsets requested with CMD_REQUEST_CONFIG
    int16_t currentHeight;       // mm
    int8_t currentPhase;         // always 0
    Gait currentGait;
    Mode currentMode;
};
```

| Command | Arguments | Sent by the OLED controller |
| --- | --- | --- |
| `CMD_SET_GAIT` | `[0]` gait | ✅ Button 2 on the home page, or the gait page |
| `CMD_SET_MODE` | `[0]` mode. `MODE_CONFIG` moves every joint to its centre plus its offset | ✅ Button 3 on the home page, or the mode page. Opening the config page sends `MODE_CONFIG` |
| `CMD_SET_CONFIG` | `[0]` leg 0–5, `[1]` joint 0–2, `[2]` offset in degrees (robot clamps to ±60) | ✅ Joint page |
| `CMD_HOME_STANCE` | Stand up | ✅ Going back from the config page to the menu |
| `CMD_REQUEST_CONFIG` | `[0]` leg; its offsets come back in `legConfigs` | — |
| `CMD_SIT` | `[0]` 1 sit, 0 stand | — |
| `CMD_POWER_DOWN` | Sit down, then turn servo power off | — |

The encoder on the home page raises and lowers `currentHeight`. The robot applies a command once,
when it or its arguments change. New commands must be appended
so existing values don't change.

## Pairing

ESP-NOW peers are set by MAC address, built into each firmware:

| Firmware | Setting | Set it to |
| --- | --- | --- |
| `oled-controller/` | `receiverMAC` in `include/DataPacket.h` | The robot's ESP32 (ESP32 stack) or the receiver (Servo2040 stack) |
| `touch-controller/display/` | `receiverMAC` in `main/Data_Packet/Data_Packet.cpp` | The same |
| `receiver/` | `controllerMAC` in `include/CommEspNow.h` | The controller |
| hexapod-esp32 | `CONTROLLER_MAC` in `include/RobotConfig.h` | The controller |

The receiver prints its own MAC at boot. Like hexapod-esp32, it ignores packets from any other
sender and prints their address (`Ignored a packet from ...`), so you can copy the controller's
address from the monitor.

## Development notes

- **Packet changes start in the robot firmware.** Change `DataPacket.h` there first, then mirror
  it in `common/HexapodPackets.h` and update the size checks. `HexapodPackets.h` has to compile as
  C too, because the touch controller's display includes it from `main.c`.
- **Receiver concurrency:** the ESP-NOW callback runs in the Wi-Fi task and the I²C callbacks in
  the I²C driver's, so both packets are guarded by `packetMux`. Don't print from the callbacks:
  they set flags that `loop()` reports.
- **LVGL** is the `external/lvgl` submodule, pinned to v8.4.0 and found through
  `EXTRA_COMPONENT_DIRS` in `touch-controller/display/CMakeLists.txt`. To update it, check out a
  newer tag in `external/lvgl`, rebuild, and commit the submodule. v9 renames much of the API, so
  moving to it is a port, not an update. The SD card's drive letter is
  `CONFIG_LV_FS_STDIO_LETTER=83` (`S`) in `sdkconfig`; LVGL 8.2 got this wrong, which is why it
  used to be a patched copy.
- **Upload ports** aren't set in `platformio.ini`; PlatformIO detects them. Pass `--upload-port`
  if it picks the wrong one.

### Troubleshooting

- **The robot ignores the controller:** check that the controller's `receiverMAC` is the robot's
  (or the receiver's) address. On hexapod-esp32 or the receiver, look for `Ignored a packet from
  ...` on the monitor.
- **`Ignored a 14-byte packet` or a similar size mismatch:** one end has an old packet layout.
  Rebuild both against the current `HexapodPackets.h` and the robot's `DataPacket.h`.
- **The robot stops every few seconds:** the receiver's failsafe is tripping (`Failsafe: ...` on
  its monitor). Move the controller closer, or check it's sending every 10 ms.
- **`Failed to resolve component 'lvgl'`** or missing `lvgl.h`: the submodule isn't checked out.
  Run `git submodule update --init`.
- **`idf.py` reports a `"path" field ... does not point to a directory`:** an old
  `dependencies.lock` with a machine-specific path. Delete it and build again.

## License

Apache-2.0. See [LICENSE](LICENSE) and [NOTICE](NOTICE). LVGL, the `external/lvgl` submodule, is
MIT.
