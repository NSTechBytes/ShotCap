# ShotCap

ShotCap is a powerful command‑line screenshot capture tool for Windows. Written in C++ using the Windows API and GDI+, ShotCap supports capturing screenshots from various sources such as the full desktop, specific windows, regions (both predefined and interactively selected), monitors, and the active window. The tool also offers advanced features like DPI awareness, mouse pointer capture, timestamp annotation, JPEG quality control, clipboard support, repeat capture, and more.

## Features

- **Full Desktop Capture:** Capture the entire screen.
- **Region Capture:** Capture a rectangular region:
  - Predefined via `-r x,y,w,h`
  - Interactive selection via `-select` (with a full‑screen overlay and a crosshair cursor).
- **Window Capture:** Capture a specific window by its title using `-w "Window Title"`.
- **Active Window Capture:** Capture the foreground window using `-active`.
- **Monitor Capture:** Capture a specific monitor (0-based index) with `-m <index>`.
- **Delay Capture:** Add a delay (in seconds) before capturing using `-d`.
- **Mouse Pointer Capture:** Include the mouse pointer in the screenshot with `-p`.
- **Timestamp Annotation:** Overlay the current date/time on the screenshot using `-timestamp`.
- **JPEG Quality Control:** Adjust JPEG quality (0–100) via `-quality <value>` (only applies when saving as JPEG).
- **Output Directory Selection:** Save screenshots to a specified directory with `-dir`.
- **Clipboard Support:** Copy the screenshot to the clipboard with `-clipboard`.
- **Auto-Open:** Automatically open the captured image after saving using `-show`.
- **Repeat Capture (Timelapse Mode):** Capture multiple screenshots at set intervals using `-repeat <interval> <count>`.
- **Monitor/Window Listing:** List available monitors (`-listmonitors`) or visible top-level windows (`-listwindows`).

## Getting Started

### Prerequisites

- **Operating System:** Windows 10 (or later recommended).
- **Compiler:** Visual Studio (2017 or later recommended).
- **Libraries:** GDI+ and Shcore (for DPI awareness). Ensure the Windows SDK is installed.

### Building ShotCap

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/<your-username>/ShotCap.git
   ```

2. **Open the Project in Visual Studio:**
   - Open the solution file (`ShotCap.sln`).

3. **Configure Project Settings:**
   - **Character Set:** Ensure the project uses **Unicode**.
   - **Additional Dependencies:** Under **Configuration Properties > Linker > Input**, add `gdiplus.lib` and `Shcore.lib`.
   - **Target Version:** Ensure `_WIN32_WINNT` is defined as at least `0x0A00` (Windows 10) in your project settings or source code.

4. **Build the Project:**
   - Build the solution (e.g., press **Ctrl+Shift+B**) to produce `ShotCap.exe`.

## Usage

ShotCap is used via the command line. Open a command prompt in the directory where `ShotCap.exe` is located and use one or more of the following options:

### Command-Line Options

```
Usage: ShotCap.exe [options]
Options:
  -f <filename>         Output file name (default: screenshot.png)
  -dir <directory>      Output directory (default: current directory)
  -d <delay>            Delay in seconds before capturing (default: 0)
  -r <x,y,w,h>          Capture region (default: full screen)
  -select               Interactively select a region with the mouse (overrides -r)
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

- **Capture the Full Desktop (Default PNG):**

  ```bash
  ShotCap.exe
  ```

- **Capture a Specific Region (Predefined):**

  ```bash
  ShotCap.exe -r 100,100,500,400
  ```

- **Interactive Region Selection:**

  ```bash
  ShotCap.exe -select
  ```

  *A full‑screen overlay with a crosshair cursor will appear. Click and drag to select a region.*

- **Capture a Specific Window by Title:**

  ```bash
  ShotCap.exe -w "Untitled - Notepad"
  ```

- **Capture the Active Window:**

  ```bash
  ShotCap.exe -active
  ```

- **Capture Monitor 1:**

  ```bash
  ShotCap.exe -m 1
  ```

- **Capture with a Delay of 3 Seconds:**

  ```bash
  ShotCap.exe -d 3
  ```

- **Include Mouse Pointer and Timestamp:**

  ```bash
  ShotCap.exe -p -timestamp
  ```

- **Save as JPEG with Quality 80:**

  ```bash
  ShotCap.exe -f shot.jpg -format jpg -quality 80
  ```

- **Copy to Clipboard and Auto-Open:**

  ```bash
  ShotCap.exe -clipboard -show
  ```

- **Repeat Capture (Every 5 Seconds, 3 Times):**

  ```bash
  ShotCap.exe -repeat 5 3
  ```

- **List Available Monitors:**

  ```bash
  ShotCap.exe -listmonitors
  ```

- **List Visible Windows:**

  ```bash
  ShotCap.exe -listwindows
  ```

## Code Structure

### DPI Awareness

- **DPI Handling:**  
  The code calls `SetProcessDpiAwarenessContext` (with a fallback to `SetProcessDPIAware`) to ensure screen coordinates are correct on high-DPI displays.

### Interactive Region Selection

- **Overlay Window:**  
  When the `-select` flag is provided, a full‑screen layered window is created. The window uses a crosshair cursor (set via `wc.hCursor = LoadCursor(NULL, IDC_CROSS)`), and the user can click and drag to select a region. The selected coordinates are stored in the global variable `g_selRect`.

### Capture Methods

- **Full Desktop, Region, Monitor, Window, and Active Window Capture:**  
  Depending on the command‑line arguments, ShotCap uses either `BitBlt` or `PrintWindow` (with flags `PW_RENDERFULLCONTENT` and `PW_CLIENTONLY`) to capture the screen. If window capture fails, the code falls back to capturing the full desktop and cropping.

### Image Processing and Output

- **Image Conversion:**  
  The captured HBITMAP is converted into a GDI+ `Bitmap`.
- **Timestamp Annotation:**  
  If enabled, the current date/time is drawn onto the image.
- **JPEG Quality:**  
  JPEG output quality is set via `EncoderParameters`.
- **Clipboard:**  
  The screenshot can be copied to the clipboard after conversion to a device-independent bitmap (DIB).
- **File Output and Auto-Open:**  
  The image is saved in the specified format to disk, and can be opened automatically if requested.

### Repeat Capture

- **Timelapse Mode:**  
  With `-repeat <interval> <count>`, the tool can capture multiple screenshots at fixed intervals.

## Contributing

Contributions are welcome! Please fork the repository and create a pull request with improvements, bug fixes, or new features. For major changes, open an issue first to discuss your ideas.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Microsoft Windows API Documentation](https://docs.microsoft.com/en-us/windows/win32/api/)
- Inspiration and ideas from the open-source community.
