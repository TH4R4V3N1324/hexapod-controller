# hexapod-controller

Firmware for the hexapod's handheld controllers, and for the ESP32 receiver that lets any robot be
driven by a handheld controller or an off-the-shelf Bluetooth gamepad (PlayStation, Xbox, Switch Pro,
8BitDo and others). A handheld controller sends joysticks and commands over **ESP-NOW**, directly to
the [ESP32 stack](https://github.com/TH4R4V3N1324/hexapod-esp32) or through the receiver. The
receiver passes them to the [Servo2040 stack](https://github.com/TH4R4V3N1324/hexapod-servo2040)
over I²C, or relays them to the ESP32 stack over ESP-NOW.

The robot, its build, and how the stacks fit together are documented in
[hexapod-docs](https://github.com/TH4R4V3N1324/hexapod-docs). The controllers' PCBs are in
[hexapod-hardware](https://github.com/TH4R4V3N1324/hexapod-hardware/tree/main/controllers). This
README only covers building and working on the firmware.

> **Status:** on hold. The OLED controller was built for an earlier prototype. It now speaks the
> current packet layout, but it hasn't been tested against either robot stack yet. Gamepad support
> in the receiver builds but hasn't been tested on hardware.

## Projects

| Folder | What it is | Board | Toolchain |
| --- | --- | --- | --- |
| [`oled-controller/`](oled-controller/) | **OLED controller**: two sticks, buttons, encoder, menus on a 128×64 OLED | [OLED controller](https://github.com/TH4R4V3N1324/hexapod-hardware/tree/main/controllers/oled-controller) PCB, rev v3 (ESP32-S2) | PlatformIO, Arduino |
| [`receiver/`](receiver/) | **Receiver**: handheld controller (ESP-NOW) or Bluetooth gamepad in; Servo 2040 (I²C) or the robot's ESP32 (ESP-NOW) out | Any classic ESP32 dev board (needs Bluetooth Classic and BLE) | ESP-IDF 5.3, Bluepad32 4.2 |
| [`touch-controller/display/`](touch-controller/display/) | **Touch controller** (experimental): LVGL touchscreen UI, IMU, RTC, SD card | ESP32-S3 2.8" RGB LCD board | ESP-IDF 5.3, LVGL 8.4 |
| [`touch-controller/left-board/`, `right-board/`](touch-controller/) | The touch controller's side boards: a stick each, plus a D-pad (left) or encoder (right), polled by the display over UART | ATtiny1616 | PlatformIO, Arduino (megaTinyCore) |

The OLED controller and the receiver are the working pair. The touch controller is an unfinished
redesign and has no published PCB.

## Quick start

```sh
git clone --recursive https://github.com/TH4R4V3N1324/hexapod-controller.git
```

`--recursive` fetches the submodules: LVGL for the touch controller, and Bluepad32 with the BTstack
inside it for the receiver. In an existing clone, run `git submodule update --init --recursive`.

The OLED controller uses [PlatformIO](https://platformio.org/) (the VS Code extension or the CLI),
pinned to the `espressif32` platform 6.11.0 (Arduino-ESP32 2.0.17), the same as hexapod-esp32:

```sh
cd oled-controller
pio run -t upload
pio device monitor
```

The receiver and the touch controller's display need
[ESP-IDF](https://docs.espressif.com/projects/esp-idf/) **5.3**:

```sh
cd receiver                   # or touch-controller/display (target esp32s3)
idf.py set-target esp32       # first time only
idf.py -p PORT build flash monitor
```

Then pair the ends (see [Pairing](#pairing)): ESP-NOW devices only talk to the MAC addresses
they're built with, and the receiver only accepts gamepads it has paired with.

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
`0x08`. On the ESP32 stack the receiver needs only power: it talks to the robot over ESP-NOW. The
BOOT button pairs gamepads and the blue LED (GPIO 2) shows the state. All pins and addresses are in
`receiver/main/Config.h`.

The receiver needs a **classic ESP32** (such as an ESP32 DevKit). The S2 has no Bluetooth, and the
S3 and C3 have only BLE, which leaves out PlayStation and Switch Pro controllers.

## Repository layout

```text
common/
  HexapodPackets.h     ControlPacket, HexPacket, Command, Gait, Mode: shared with the robot firmware
external/
  lvgl/                Submodule, LVGL v8.4.0 (touch controller)
  bluepad32/           Submodule, Bluepad32 4.2.0, with BTstack inside it (receiver)
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
receiver/              Receiver (ESP-IDF, classic ESP32)
  main/
    Config.h           Addresses, pins, timings, gamepad mapping settings
    main.cpp           Start-up, the 100 Hz loop, the pairing button and LED
    Link               Which input drives, the failsafe, and the packets the outputs share
    EspNowLink         Handheld controller in, relay to the robot's ESP32 out
    I2CLink            I²C slave for the Servo 2040
    Gamepad            Bluepad32 platform: pairing, and gamepad → ControlPacket
  CMakeLists.txt       Also turns the BTstack submodule into an ESP-IDF component, in build/
touch-controller/      Touch controller (experimental)
  display/             ESP-IDF project (ESP32-S3); main/ has one folder per driver or subsystem
  left-board/, right-board/  Side boards (ATtiny1616)
  animations/          Idle animation frames (PNG sources and .bin for the SD card)
```

## How it works

```text
ESP32 stack, handheld:   controller ──ESP-NOW──────────────────────────► robot's ESP32
ESP32 stack, gamepad:    gamepad ──Bluetooth──► receiver ──ESP-NOW relay──► robot's ESP32
Servo2040 stack:         controller ──ESP-NOW──┐
                         gamepad ──Bluetooth───┴► receiver ──I²C──► Servo 2040
```

A handheld controller sends a `ControlPacket` every 10 ms, and the robot answers each one with a
`HexPacket`. The receiver builds a `ControlPacket` from a gamepad every 10 ms the same way, so the
robots can't tell the difference and need no changes.

**Receiver outputs.** The I²C slave is always on. When `ROBOT_MAC` in `Config.h` is set, the
receiver also sends every `ControlPacket` to that robot over ESP-NOW. The robot then treats the
receiver as its controller, so set the robot's `CONTROLLER_MAC` to the receiver's MAC. Either way,
the robot's latest `HexPacket` goes back to the handheld controller with its next reply, and is used
for the gamepad's gait, mode and height buttons.

**One input at a time.** Whichever of the handheld controller and the gamepad is driving keeps
control while it's alive. The other takes over only when it's lost. One gamepad drives at a time;
a second one is refused.

**Failsafe.** The handheld controller counts as lost after **250 ms** without a packet, and a gamepad
when it disconnects or goes **500 ms** without a report while a stick is pushed. (Some gamepads only
report changes, so a resting one is quiet.) With nothing driving, the receiver centres the sticks
and clears the command, so the robot stops instead of repeating the last input. Height is kept, so
the robot doesn't drop. The robots also have their own 200 ms timeouts.

## Gamepads

Bluepad32 supports, among others, DualShock 4, DualSense, Xbox Wireless (controllers with
Bluetooth, on firmware 5.x), Switch Pro, Joy-Con, 8BitDo, Stadia and generic BLE gamepads. See its
[supported controllers](https://bluepad32.readthedocs.io/en/latest/supported_gamepads/).

| Input | Does | Packet |
| --- | --- | --- |
| Left stick | Walk: forward/back, and strafe or turn (by mode). Tilt mode: shift the body | `joystick1X/Y` |
| Right stick | Turn. Tilt mode: roll and pitch the body | `joystick2X/Y` |
| A (Cross, or B on Switch) | Stand up | `CMD_SIT 0` |
| B (Circle, or A on Switch) | Sit down | `CMD_SIT 1` |
| R1 / RB | Next gait (tripod, ripple, wave) | `CMD_SET_GAIT` |
| L1 / LB | Next mode (normal, strafe, tilt) | `CMD_SET_MODE` |
| D-pad up / down | Body height ±10 mm from the robot's current height | `currentHeight` |
| Hold Start / Options / Menu for 2 s | Power down: sit, then servo power off | `CMD_POWER_DOWN` |

Buttons are by position, Xbox names: A is always the bottom face button. Each command is held for
150 ms and then cleared, so pressing the same button again counts. Config mode and calibration are
left to the handheld controller. The step sizes, timings and dead zone are in `Config.h`; the button
choices are in `Gamepad::update()`.

When a gamepad connects it rumbles briefly, and gamepads with lights show player 1 and a blue light
bar.

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

### Gamepads

1. Press the receiver's **BOOT** button. The blue LED blinks quickly for 60 seconds: the pairing
   window. (A receiver that has never paired opens it by itself at power-up.)
2. Put the gamepad in pairing mode:
   - **DualShock 4 / DualSense:** hold Share (Create) and PS until the light bar flashes.
   - **Xbox:** hold the pair button on top until the Xbox button flashes quickly.
   - **Switch Pro:** hold the sync button on top.
   - **8BitDo:** see its manual for the mode (usually Switch or X-input) and pairing button.
3. It connects, rumbles, and is remembered; the window closes. From then on it reconnects by itself
   when you turn it on.

**Hold BOOT for 5 seconds** to forget every paired gamepad (the window then opens again). Outside a
pairing window only remembered gamepads can connect, so nobody nearby can take over the robot.

### ESP-NOW

ESP-NOW peers are set by MAC address, built into each firmware:

| Firmware | Setting | Set it to |
| --- | --- | --- |
| `oled-controller/` | `receiverMAC` in `include/DataPacket.h` | The robot's ESP32 (ESP32 stack) or the receiver |
| `touch-controller/display/` | `receiverMAC` in `main/Data_Packet/Data_Packet.cpp` | The same |
| `receiver/` | `CONTROLLER_MAC` in `main/Config.h` | The handheld controller, or all zeros for none |
| `receiver/` | `ROBOT_MAC` in `main/Config.h` | ESP32 stack: the robot's ESP32. Servo2040 stack: all zeros |
| hexapod-esp32 | `CONTROLLER_MAC` in `include/RobotConfig.h` | The handheld controller, or the receiver when relaying |

The receiver prints its own MAC at boot. Like hexapod-esp32, it ignores packets from any other
sender and prints their address (`Ignored a packet from ...`), so you can copy addresses from the
monitor.

## Development notes

- **Packet changes start in the robot firmware.** Change `DataPacket.h` there first, then mirror
  it in `common/HexapodPackets.h` and update the size checks. `HexapodPackets.h` has to compile as
  C too, because the touch controller's display includes it from `main.c`.
- **Receiver tasks:** Bluepad32 runs on BTstack's run loop in the main task; the 100 Hz loop, the
  I²C slave and the Wi-Fi (ESP-NOW) callbacks each run in their own. `Link` holds everything they
  share behind one lock. Bluepad32 isn't thread-safe, so the loop posts pairing changes to the
  BTstack task (`btstack_run_loop_execute_on_main_thread`). Don't print from the Wi-Fi callback:
  it sets flags that the loop reports.
- **Receiver I²C:** a classic ESP32 can't stretch the clock as a slave, so a read gets whatever is
  already in the TX FIFO. The receiver keeps one `ControlPacket` loaded and replaces it with the
  newest after every read (as Arduino's `Wire` does), so the Servo 2040 gets packets one tick old.
- **Bluepad32 and BTstack:** `external/bluepad32` is pinned to 4.2.0, and its own BTstack submodule
  comes with it. Bluepad32 normally copies BTstack into an ESP-IDF component inside the submodule
  (`integrate_btstack.py`); `receiver/CMakeLists.txt` does the same copy into `build/` instead, so
  the submodule stays clean. Bluepad32 also suggests a BTstack patch that relaxes the security
  check on incoming L2CAP connections; it isn't applied (Bluepad32's own template doesn't either).
  If a particular gamepad won't reconnect, that patch is the first thing to try.
- **Wi-Fi and Bluetooth share the radio.** The receiver scans for gamepads only while none is
  connected (BLE gamepads need a scan to reconnect), so ESP-NOW gets more airtime while driving.
  Wi-Fi's IRAM optimisations are off to make room for both stacks; check ESP-NOW latency with a
  gamepad connected when testing.
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
- **`Failed to resolve component 'lvgl'`**, missing `lvgl.h`, or `Bluepad32 or BTstack is
  missing`: the submodules aren't checked out. Run `git submodule update --init --recursive`.
- **A gamepad won't pair:** check the LED is blinking (press BOOT), and that the gamepad is in
  pairing mode, not just on. Xbox controllers need firmware 5.x or later (update with the Xbox
  Accessories app). If it was paired to something else, it may connect there first.
- **A paired gamepad won't reconnect:** hold BOOT for 5 seconds to forget everything, then pair
  again.
- **`idf.py` reports a `"path" field ... does not point to a directory`:** an old
  `dependencies.lock` with a machine-specific path. Delete it and build again.

## License

Apache-2.0. See [LICENSE](LICENSE) and [NOTICE](NOTICE). The submodules have their own licences:
LVGL is MIT, Bluepad32 is Apache-2.0, and BTstack is free for non-commercial use only (commercial
use needs a licence from BlueKitchen).
