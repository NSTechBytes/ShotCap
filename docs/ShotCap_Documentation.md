# ShotCap Documentation

ShotCap is a powerful command‑line screenshot capture tool for Windows. Written in C++ using the Windows API and GDI+, ShotCap offers many capture options—from full desktop and window capture to interactive region selection with a crosshair cursor. This documentation explains the app's features, installation, usage, and development guidelines.

---

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Installation and Setup](#installation-and-setup)
- [Usage](#usage)
  - [Command‑Line Options](#command-line-options)
  - [Examples](#examples)
- [Troubleshooting](#troubleshooting)
- [Development Guide](#development-guide)
- [Contributing](#contributing)
- [License](#license)

---

## Introduction

ShotCap is designed to simplify the process of capturing screenshots on Windows systems. It supports capturing:
- The full desktop
- Specific regions (via command‑line coordinates or interactive mouse selection)
- Specific windows (by title or active window)
- Specific monitors (in multi‑monitor setups)

In addition, ShotCap offers options such as:
- Including the mouse pointer in screenshots
- Annotating screenshots with a timestamp
- Repeating captures at specified intervals (for timelapse sequences)
- Copying the screenshot to the clipboard
- Automatically opening the screenshot after capture

---

## Features

- **Full Desktop Capture:** Capture your entire screen with a single command.
- **Region Capture:** Specify coordinates via `-r x,y,w,h` or interactively select an area with `-select`.
- **Window Capture:** Capture a specific window by its title using `-w "Window Title"` or the active window with `-active`.
- **Monitor Capture:** Capture a specific monitor in multi‑monitor configurations with `-m <index>`.
- **Mouse Pointer:** Optionally include the mouse pointer using `-p`.
- **Timestamp Annotation:** Overlay the current date/time on your screenshot with `-timestamp`.
- **Repeat Capture:** Capture multiple screenshots at set intervals with `-repeat <interval> <count>`.
- **Clipboard Support:** Copy the screenshot directly to the clipboard using `-clipboard`.
- **Auto-Open:** Automatically open the saved screenshot with `-show`.
- **Verbose Logging:** Get detailed output during execution with the `-v` flag.

---

## Installation and Setup

### Prerequisites

- **Operating System:** Windows 10 or later (recommended).
- **Development Environment:** Visual Studio 2019 or later is recommended.
- **Dependencies:**  
  - GDI+  
  - Shcore (for DPI awareness)

### Build Instructions

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/NSTechBytes/ShotCap.git
   cd ShotCap
   ```

2. **Open the Solution:**

   Open the provided `ShotCap.sln` file in Visual Studio.

3. **Configure Project Settings:**

   - **Character Set:** Set to "Use Unicode Character Set" (Project Properties → General).
   - **Additional Dependencies:**  
     Add `gdiplus.lib` and `Shcore.lib` under Project Properties → Linker → Input.
   - **Target Windows Version:**  
     Ensure `_WIN32_WINNT` is defined to at least `0x0A00` (Windows 10) in the project settings or in your source files.

4. **Build the Project:**

   Build the solution (e.g., press **Ctrl+Shift+B**).

---

## Usage

ShotCap is executed from the command line. After building, navigate to the folder containing `ShotCap.exe` and run it with the desired options.

### Command‑Line Options

```
Usage: ShotCap.exe [options]

Options:
  -f <filename>         Output file name (default: screenshot.png)
  -dir <directory>      Output directory (default: current directory)
  -d <delay>            Delay in seconds before capturing (default: 0)
  -r <x,y,w,h>          Capture region (default: full screen)
  -select               Interactively select a region with the mouse
  -format <format>      Image format: png, jpg, bmp (default: png)
  -quality <0-100>      JPEG quality (only for -format jpg, default: 90)
  -w <window_title>     Capture a specific window by its title
  -active               Capture the active (foreground) window
  -m <monitor_index>    Capture a specific monitor (0-based index)
  -clipboard            Copy captured image to clipboard
  -show                 Open the captured image after saving
  -p                    Include the mouse pointer in the screenshot
  -timestamp            Annotate screenshot with current date/time
  -repeat <i> <n>       Repeat capture every i seconds for n times
  -listmonitors         List available monitors and exit
  -listwindows          List visible top-level windows and exit
  -v                    Enable verbose logging
  -h, --help            Display this help message
```

### Examples

- **Capture Full Desktop:**

  ```bash
  ShotCap.exe
  ```

- **Capture with a 3-Second Delay:**

  ```bash
  ShotCap.exe -d 3
  ```

- **Capture a Specific Region by Coordinates:**

  ```bash
  ShotCap.exe -r 100,100,500,400
  ```

- **Interactively Select a Region:**

  ```bash
  ShotCap.exe -select
  ```

- **Capture a Specific Window by Title:**

  ```bash
  ShotCap.exe -w "Untitled - Notepad"
  ```

- **Capture the Active Window:**

  ```bash
  ShotCap.exe -active
  ```

- **Capture a Specific Monitor (Index 1):**

  ```bash
  ShotCap.exe -m 1
  ```

- **Include Mouse Pointer and Timestamp:**

  ```bash
  ShotCap.exe -p -timestamp
  ```

- **Copy to Clipboard and Auto-Open:**

  ```bash
  ShotCap.exe -clipboard -show
  ```

- **Repeat Capture (e.g., every 5 seconds, 3 times):**

  ```bash
  ShotCap.exe -repeat 5 3
  ```

- **Verbose Logging:**

  ```bash
  ShotCap.exe -v -select -timestamp
  ```

---

## Troubleshooting

- **Black Screenshots:**  
  If capturing a specific window (by title or active window) results in a black image, it might be due to hardware acceleration or the window’s rendering method. Try using the full desktop capture (`-r` or `-select`) and then cropping the window's region.

- **Interactive Selection Issues:**  
  If the interactive selection mode is not working, ensure that no other application is capturing mouse events and that the overlay is visible. Run with verbose logging (`-v`) to see detailed messages.

- **Clipboard Problems:**  
  If copying to clipboard fails, make sure no other application is locking the clipboard. Running as an administrator may help in some cases.

---

## Development Guide

For detailed development instructions, please refer to our [CONTRIBUTING.md](https://github.com/NSTechBytes/ShotCap/blob/main/CONTRIBUTING.md) file.

---

## Contributing

Contributions are welcome! If you want to contribute to ShotCap:
- Fork the repository.
- Create a new branch for your feature or bug fix.
- Make your changes and commit them with clear, descriptive messages.
- Submit a pull request, referencing any related issues.

For detailed guidelines, please see our [CONTRIBUTING.md](https://github.com/NSTechBytes/ShotCap/blob/main/CONTRIBUTING.md).

---

## License

ShotCap is released under the MIT License. See the [LICENSE](https://github.com/NSTechBytes/ShotCap/blob/main/LICENSE) file for details.

---

## Acknowledgments

- Thanks to the Windows API and GDI+ documentation for providing guidance.
- Inspiration from the open-source community for creating a comprehensive screenshot tool.
- Special thanks to all contributors who help improve ShotCap!
