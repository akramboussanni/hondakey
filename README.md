# Wake-on-LAN Key

This is a simple ESP32 project that acts as a physical "Wake-on-LAN" key. It can trigger a WoL magic packet either to a Honda server or directly over the local network.

You can download Honda at https://github.com/akramboussanni/honda

## Getting Started

1.  **Hardware**: ESP32, optional OLED (SSD1306), and a button/touch sensor.
2.  **IDE**: Install **VS Code** and the **PlatformIO IDE** extension.
3.  **Setup**:
    -   Copy `include/config.h.example` to `include/config.h`.
    -   Open `include/config.h` and edit your WiFi credentials and Target MAC address.
    -   Set `USE_LOCAL_WOL` to `true` (recommended for simple home use).
    -   If you don't have an OLED screen, set `USE_OLED` to `false`.

## Usage

1.  Open the project in PlatformIO.
2.  Connect your ESP32.
3.  Click "Upload".
4.  Press the button to wake your PC!