# Twitch Pinball Controller

## Overview
A fun, early-stage project to control a real pinball machine via Twitch chat!
Viewers can type commands like "left flip", "right flip", or "start" to trigger flippers and the start button remotely over the internet.

## Features
- **Twitch Chat Integration**: Uses Twitch EventSub WebSocket to catch chat commands.
- **ESP32 Control**: HTTP POST API pulses GPIOs to flip flippers via PC817 optocouplers.
- **WiFi**: Connects to your network for remote access.

## Status
Very early development—working prototype; rough around the edges.
Expect bugs and cleanup as it evolves.

## Setup
1. **Hardware**: ESP32, PC817 optocouplers, pinball machine with leaf switches.
2. **ESP32 Firmware**: Edit `src/wifi.c` adding your wifi credentials. Use PlatformIO to flash the firmware.
3. **Twitch Server**: Run `twitch-chat.js` (Node.js) with Twitch Client ID, Token (scope: `user:read:chat`), and ESP32 IP.

## Commands
- `left flip`: Triggers left flipper.
- `right flip`: Triggers right flipper.
- `start`: Triggers start button.

## Todo
- Code cleanup.
- Token refresh for Twitch.
- Cooldowns to prevent spam.
- Better error handling.
- Move wifi credentials to platformio.ini build flags.
