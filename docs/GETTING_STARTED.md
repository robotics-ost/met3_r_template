# Getting Started: Software Installation

This guide explains how to install everything you need for the MeT3_R practical exercises. **You only have to do this once per computer.** Plan about one hour (mostly waiting for downloads).

> [!IMPORTANT]
> Complete **all** steps and tick off the [checklist](#checklist) at the end.
> Afterwards, continue with the [Quick Start](../README.md#quick-start) in the main README.

> [!NOTE]
> Keyboard shortcuts in this documentation are given for Windows and Linux. On macOS, use `Cmd` instead of `Ctrl`.

## Overview

| # | What | Why |
|---|---|---|
| 1 | GitHub account | Stores your code online and shares it with your lecturers |
| 2 | Git | Version control: tracks the changes to your code |
| 3 | VS Code | Code editor |
| 4 | PlatformIO extension | Compiles your code and uploads it to the robot |
| 5 | USB driver (CP210x) | Lets your computer talk to the robot over USB |
| 6 | Python 3 | Runs the web GUI on your PC (plots, commands) |

## Step 1: GitHub account

If you don't have one yet, create a free account on <https://github.com/signup>.

## Step 2: Git

**Windows:** Download and run the installer from <https://git-scm.com/downloads/win>. Keep all default settings.

**macOS:** Open the *Terminal* app and run `xcode-select --install`. This installs Git together with other developer tools.

**Linux (Debian/Ubuntu):** `sudo apt install git`

**All systems:** Tell Git who you are. Open a terminal (Windows: *Git Bash* or *PowerShell*) and run the following two commands with **your** name and e-mail address:

```bash
git config --global user.name "Max Muster"
git config --global user.email "max.muster@ost.ch"
```

> [!NOTE]
> Without this step, Git refuses to create commits later on.

## Step 3: VS Code

Download and install VS Code from <https://code.visualstudio.com/>.

## Step 4: PlatformIO extension

1. In VS Code, open the **Extensions** view (`Ctrl+Shift+X`).
2. Search for **PlatformIO IDE** and click **Install**.
3. Wait until the installation has finished and **restart VS Code** when asked.

After the restart, you see an ant-head icon in the left side bar. That is PlatformIO.

> [!WARNING]
> Do **not** install the extension *pioarduino IDE* in addition to PlatformIO IDE. The two conflict with each other.

> [!NOTE]
> The ESP32 toolchain is downloaded automatically the first time you open the project ([Quick Start, Step 4](../README.md#step-4-open-the-project-in-vs-code)). This can take 10&ndash;20 minutes.

## Step 5: USB driver (CP210x)

The robot's ESP32 board uses a **CP2102** USB-to-serial chip. Most operating systems recognize it automatically. Check this first and only install the driver if needed.

> [!WARNING]
> **Use a USB cable that can transfer data.** Many cables (e.g. from power banks) can only charge. With such a cable the robot is not detected, even with the right driver.

### Windows

1. Connect the robot with a USB cable.
2. Open the **Device Manager** (right-click on the start button &rarr; *Device Manager*).
3. Expand **Ports (COM &amp; LPT)**. If you see **Silicon Labs CP210x USB to UART Bridge (COMx)**, you are done.
4. Otherwise download the **CP210x VCP driver for Windows** from <https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers>, unzip it, right-click on `silabser.inf` &rarr; **Install**, and check again.

### macOS

1. Connect the robot and run `ls /dev/cu.*` in the *Terminal*.
2. If you see an entry such as `/dev/cu.usbserial-XXXX` or `/dev/cu.SLAB_USBtoUART`, you are done.
3. Otherwise install the **CP210x VCP driver for macOS** from <https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers> and allow it under **System Settings** &rarr; **Privacy &amp; Security**.

### Linux

The driver is already part of the Linux kernel. You only have to allow your user to access the serial port:

```bash
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
sudo usermod -a -G dialout $USER    # Arch Linux: uucp instead of dialout
```

Then **log out and in again** and reconnect the robot. See the [PlatformIO udev documentation](https://docs.platformio.org/en/latest/core/installation/udev-rules.html) for details.

## Step 6: Python 3

You need Python 3 for the web GUI. If you already use Python, you can skip this step.

**Windows:** Download the installer from <https://www.python.org/downloads/>.

> [!IMPORTANT]
> On the first page of the Python installer, tick **Add python.exe to PATH**.

**macOS / Linux:** Python 3 is usually already installed. Check with `python3 --version`.

## Checklist

Check that:

- [ ] you can sign in to GitHub,
- [ ] `git --version` prints a version number in a terminal,
- [ ] `git config --global user.name` prints your name,
- [ ] VS Code shows the PlatformIO ant-head icon in the left side bar,
- [ ] your computer recognizes the robot when it is connected via USB (see [Step 5](#step-5-usb-driver-cp210x), requires the robot),
- [ ] `python --version` (Windows) or `python3 --version` (macOS/Linux) prints a version number.

All ticked? Continue with the [Quick Start](../README.md#quick-start).
