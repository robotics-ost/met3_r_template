# Troubleshooting

Something doesn't work? Find your problem below. The headings are the messages or symptoms you see.

> [!TIP]
> **Search this page for your error message** with `Ctrl+F`. When there are many error messages, always start with the **first** one.

## Build

### `include/wifi_secrets.h is missing`

The file with your WiFi credentials doesn't exist yet. Create it as described in [Quick Start, Step 5](../README.md#step-5-enter-the-wifi-credentials). You need to do this on every computer you work on, because the file is not on GitHub.

### The build fails with strange errors right from the start

Check the path of your project folder:

1. It must not contain **spaces** or **special characters** (ä, ö, ü, ...).
2. It must not be inside **OneDrive** or another synchronized folder.

Move the folder (e.g. to `C:\robotics\`), open it again in VS Code and rebuild.

### The first build takes very long

The first time, PlatformIO downloads the ESP32 toolchain, which can take 10&ndash;20 minutes. Make sure your computer is connected to the internet and wait until it's done. Later builds are much faster.

### `fatal error: motor_model.h: No such file or directory`

The compiler can't find a header file:

1. Check that the `.h` file is in `lib/control_system/include/`.
2. Check the spelling in the `#include` line, including upper and lower case.

### `undefined reference to 'motor_model_init'`

The function is declared, but its implementation is missing:

1. Check that the `.c` file is in `lib/control_system/src/`.
2. Check that the function name in the `.c` file is spelled exactly like in the `.h` file.

### `implicit declaration of function '...'`

You call a function the compiler doesn't know at this point. Either the `#include` of its header is missing or the name is misspelled.

### VS Code shows red squiggly lines, but the build works

The error display in the editor (IntelliSense) is out of date. Open the **Command Palette** (`Ctrl+Shift+P`) and run **PlatformIO: Rebuild IntelliSense Index**. The build result is what counts.

## Upload

### The robot is not found / `Could not open port` / no COM port

1. **Use a USB cable that can transfer data.** Many cables can only charge. Try another cable.
2. Check that the USB driver is installed ([Getting Started, Step 5](GETTING_STARTED.md#step-5-usb-driver-cp210x)).
3. Close all other programs that could use the port, e.g. a second Serial Monitor or another VS Code window.
4. Linux: check that your user has access to the serial port ([Getting Started, Step 5](GETTING_STARTED.md#step-5-usb-driver-cp210x)).

### The upload hangs at `Connecting........_____`

1. Unplug the USB cable, plug it in again and retry.
2. If it still hangs: keep the **BOOT** button on the board pressed while `Connecting...` is shown, and release it when the upload starts.
3. If that doesn't help either: keep **BOOT** pressed, briefly press the **EN** button (reset), then release **BOOT**.

## Serial Monitor and crashes

### The Serial Monitor shows unreadable characters

The baud rate doesn't match. Open the Serial Monitor with the PlatformIO button in the status bar. It uses the speed from `platformio.ini` (115200).

### The robot restarts again and again

Unplug the USB cable and scroll up in the Serial Monitor to the message **before** the restart:

| Message | Cause | What to do |
|---|---|---|
| `ESP_ERROR_CHECK failed` | A function returned an error, e.g. an init function with an invalid parameter | The message shows the file, line and failed call. Check the parameters you pass, e.g. in `include/config.h` |
| `Guru Meditation Error` with `LoadProhibited` or `StoreProhibited` | Access to an invalid address: a `NULL` pointer, a pointer to an array that no longer exists, or an index out of range | See [Pointers and addresses](C_FOR_PYTHON_PROGRAMMERS.md#pointers-and-addresses) and [static](C_FOR_PYTHON_PROGRAMMERS.md#static) |
| `Task watchdog got triggered` | A task doesn't finish its cycle, e.g. an endless loop or a delay inside the control loop | Never wait or loop endlessly inside the control loop, see [Working with the Code](WORKING_WITH_THE_CODE.md) |
| `Stack overflow` / `stack canary` | A task uses more memory for local variables than it has, e.g. because of large local arrays | Make large arrays `static` or increase the stack size in `xTaskCreatePinnedToCore()` in `src/main.c` |

## Motors

### The motors don't move

1. **Check the safety level.** The motors only run in the safety level *Active*. Switch the levels with the buttons in the web GUI.
2. **Check the battery.** It must be charged and switched on. USB alone doesn't power the motors. If the battery voltage is too low, the Serial Monitor shows `Bus voltage too low`.
3. Check that your code actually calls `motor_driver_set_voltage()` in the control loop.

### A wheel turns in the wrong direction

1. **Check your implementation of the motor model first**, e.g. the signs in `motor_model_calc_q()` and `motor_model_calc_voltage()`. If both wheels turn the wrong way, the error is most likely there.
2. If your implementation is correct and only one motor turns the wrong way because of how it is mounted, set the parameter `invert_direction` of this motor (Exercise 02). Don't add extra signs in your equations for this.

## WiFi and web GUI

### The robot doesn't connect to the WiFi

1. Check the SSID and password in `include/wifi_secrets.h`. Upper and lower case matter.
2. **Upload again** after every change of `wifi_secrets.h`. The credentials are compiled into the program.
3. **Hotspot:** it must use the **2.4 GHz** band. The ESP32 can't connect to 5 GHz networks.

When the connection works, the Serial Monitor shows `Got IP: ...`.

### The web GUI doesn't start: `Could not resolve host`

The GUI can't find the robot by its hostname (e.g. `robot_1.local`):

1. **Is your PC in the same network as the robot?** The hostname can only be found within the same network (not needed if your laptop is the hotspot).
2. **Is the robot connected to the WiFi?** The Serial Monitor shows `Got IP: ...` when it is.
3. **Is the hostname correct?** It must match `UDP_INTERFACE_HOSTNAME` in `include/config.h`, followed by `.local`. Check `host_interface/esp32_config.json`.
4. **Name resolution with `.local` doesn't work in every network and on every computer.** Use the robot's IP address instead: it is shown in the Serial Monitor as `Got IP: ...`. Enter it in `host_interface/esp32_config.json` instead of the hostname.

### The web GUI starts, but shows no data

This typically happens when you use the IP address: the GUI starts even if the robot can't be reached.

1. **Is your PC in the same network as the robot?** Your PC and the robot must be connected to the same WiFi network (not needed if your laptop is the hotspot).
2. **Is a firewall blocking the connection?** When you start the GUI for the first time, Windows asks whether Python may access the network. Allow it. If you clicked *Cancel*, allow Python in the Windows Defender Firewall settings.
3. **Is another program connected to the robot?** The robot only sends data to one program at a time. Close other GUI windows or scripts.
4. **Is the hostname or IP address correct?** Check `host_interface/esp32_config.json`.

### The robot doesn't respond anymore after *Do System Off*

This is the expected behaviour. In the safety level *Off*, the ESP32 goes into deep sleep: WiFi and the Serial Monitor stop. Press the **EN** button (reset) on the board or switch the robot off and on again.

### The plots stopped working after I changed the telemetry

The data format on the robot and on the PC no longer match. Follow all steps in section *A. Changing what's in a control sample* of the [host interface guide](../host_interface/README.md). Open the browser console with `F12` and look for a `TypeError`.

### Windows: `running scripts is disabled on this system`

This message appears when the virtual environment is activated, either by you (`.venv\Scripts\activate`) or automatically by VS Code right after you created it with `python -m venv .venv`. PowerShell blocks the activation script by default. Run this command once in PowerShell, then activate the virtual environment again:

```powershell
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```

### `python` or `pip` is not found

Python is not installed or not in the `PATH`. Reinstall it and tick **Add python.exe to PATH** ([Getting Started, Step 6](GETTING_STARTED.md#step-6-python-3)). On macOS and Linux, use `python3` instead of `python`.

## Git and GitHub

See the section *Common problems* in the [Git Guide](GIT_GUIDE.md#common-problems).

## Still stuck?

Ask your lecturer. Bring the **complete first error message** (copy the text, or take a screenshot of the Serial Monitor or the build output) and describe what you did just before the problem occurred.
