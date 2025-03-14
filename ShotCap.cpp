#include <windows.h>
#include <gdiplus.h>
#include <shellscalingapi.h>  // For DPI awareness
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <iomanip>
#include <ctime>
#include <algorithm>

#pragma comment (lib, "gdiplus.lib")
#pragma comment (lib, "Shcore.lib")  // For DPI functions

using namespace Gdiplus;

//------------------------------------------------------------------------------
// Helper: Convert an HBITMAP to a DIB in global memory for clipboard use.
// Returns an HGLOBAL containing a BITMAPINFOHEADER and pixel data.
HGLOBAL CreateDIBFromHBITMAP(HBITMAP hBitmap)
{
    BITMAP bm;
    if (!GetObject(hBitmap, sizeof(bm), &bm))
        return NULL;

    BITMAPINFOHEADER bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight; // bottom-up DIB
    bi.biPlanes = 1;
    bi.biBitCount = bm.bmBitsPixel;
    bi.biCompression = BI_RGB;

    int lineBytes = ((bm.bmWidth * bm.bmBitsPixel + 31) & ~31) / 8;
    DWORD dwSize = lineBytes * bm.bmHeight;
    DWORD dwMemSize = sizeof(BITMAPINFOHEADER) + dwSize;

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, dwMemSize);
    if (!hMem)
        return NULL;

    LPVOID pMem = GlobalLock(hMem);
    if (!pMem) {
        GlobalFree(hMem);
        return NULL;
    }

    memcpy(pMem, &bi, sizeof(BITMAPINFOHEADER));
    LPVOID pBits = (LPBYTE)pMem + sizeof(BITMAPINFOHEADER);

    HDC hDC = GetDC(NULL);
    if (!hDC) {
        GlobalUnlock(hMem);
        GlobalFree(hMem);
        return NULL;
    }

    BITMAPINFO* pbi = (BITMAPINFO*)pMem;
    if (!GetDIBits(hDC, hBitmap, 0, bm.bmHeight, pBits, pbi, DIB_RGB_COLORS)) {
        ReleaseDC(NULL, hDC);
        GlobalUnlock(hMem);
        GlobalFree(hMem);
        return NULL;
    }
    ReleaseDC(NULL, hDC);
    GlobalUnlock(hMem);
    return hMem;
}

//------------------------------------------------------------------------------
// Helper: Retrieve the CLSID of an image encoder (e.g., PNG, JPEG, BMP).
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0, size = 0;
    if (GetImageEncodersSize(&num, &size) != Ok || size == 0)
        return -1;

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)malloc(size);
    if (!pImageCodecInfo)
        return -1;

    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    free(pImageCodecInfo);
    return -1;
}

//------------------------------------------------------------------------------
// Print usage instructions.
void printUsage()
{
    std::cout << "Usage: ShotCap.exe [options]\n"
        << "Options:\n"
        << "  -f <filename>         Output file name (default: screenshot.png)\n"
        << "  -dir <directory>      Output directory (default: current directory)\n"
        << "  -d <delay>            Delay in seconds before capturing (default: 0)\n"
        << "  -r <x,y,w,h>          Capture region (default: full screen)\n"
        << "  -format <format>      Image format: png, jpg, bmp (default: png)\n"
        << "  -quality <0-100>      JPEG quality (only applies when -format jpg, default: 90)\n"
        << "  -w <window_title>     Capture a specific window by its title\n"
        << "  -active               Capture the active (foreground) window\n"
        << "  -m <monitor_index>    Capture a specific monitor (0-based index)\n"
        << "  -clipboard            Copy captured image to clipboard\n"
        << "  -show                 Open the captured image after saving\n"
        << "  -p                    Include the mouse pointer in the screenshot\n"
        << "  -timestamp            Annotate screenshot with current date/time\n"
        << "  -repeat <i> <n>       Repeat capture every i seconds for n times\n"
        << "  -listmonitors         List available monitors and exit\n"
        << "  -listwindows          List visible top-level windows and exit\n"
        << "  -v                    Enable verbose logging\n"
        << "  -h, --help            Display this help message\n";
}

//------------------------------------------------------------------------------
// Utility: Split a string by a delimiter.
std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter))
        tokens.push_back(token);
    return tokens;
}

//------------------------------------------------------------------------------
// Structure to store monitor information.
struct MonitorInfo {
    RECT rect;
};

// Callback for enumerating monitors.
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT lprcMonitor, LPARAM dwData)
{
    std::vector<MonitorInfo>* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
    MonitorInfo mi;
    mi.rect = *lprcMonitor;
    monitors->push_back(mi);
    return TRUE;
}

//------------------------------------------------------------------------------
// Callback for enumerating top-level windows.
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    if (IsWindowVisible(hWnd))
    {
        const int len = 256;
        wchar_t title[len] = { 0 };
        GetWindowTextW(hWnd, title, len);
        if (wcslen(title) > 0)
        {
            std::wstringstream* ss = reinterpret_cast<std::wstringstream*>(lParam);
            *ss << L"Handle: " << hWnd << L" | Title: " << title << std::endl;
        }
    }
    return TRUE;
}

//------------------------------------------------------------------------------
// Annotate the captured image with a timestamp (or custom text) at the bottom-right.
void AnnotateImage(Bitmap* bmp, const std::wstring& text, bool verbose)
{
    Graphics graphics(bmp);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

    Font font(L"Arial", 20);
    SolidBrush brush(Color(255, 255, 255, 255)); // White text
    SolidBrush shadowBrush(Color(128, 0, 0, 0));   // Black shadow

    RectF layoutRect;
    StringFormat format;
    format.SetAlignment(StringAlignmentFar);
    format.SetLineAlignment(StringAlignmentFar);
    RectF bound;
    graphics.MeasureString(text.c_str(), -1, &font, RectF(0, 0, bmp->GetWidth(), bmp->GetHeight()), &format, &bound);

    float margin = 10.0f;
    PointF position(bmp->GetWidth() - bound.Width - margin, bmp->GetHeight() - bound.Height - margin);

    graphics.DrawString(text.c_str(), -1, &font, PointF(position.X + 2, position.Y + 2), &format, &shadowBrush);
    graphics.DrawString(text.c_str(), -1, &font, position, &format, &brush);

    if (verbose)
        std::wcout << L"[INFO] Timestamp annotation applied: " << text << std::endl;
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // --- DPI Awareness Setup ---
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
    {
        SetProcessDPIAware();
    }
    // --------------------------------

    // Default parameters.
    std::wstring outputFile = L"screenshot.png";
    std::wstring outputDir = L""; // if empty, use current directory.
    int delaySeconds = 0;
    bool regionSpecified = false;
    int regionX = 0, regionY = 0, regionW = 0, regionH = 0;
    std::wstring imageFormat = L"png";
    std::wstring windowTitle = L"";
    int monitorIndex = -1;
    bool copyToClipboard = false;
    bool showAfterCapture = false;
    bool capturePointer = false;
    bool annotateTimestamp = false;
    bool captureActiveWindow = false;
    bool repeatEnabled = false;
    int repeatInterval = 0;
    int repeatCount = 0;
    int jpegQuality = 90; // Default JPEG quality.
    bool verbose = false;
    bool listMonitors = false;
    bool listWindows = false;

    // Parse command-line arguments.
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            printUsage();
            return 0;
        }
        else if (arg == "-f" && i + 1 < argc)
        {
            int len = MultiByteToWideChar(CP_UTF8, 0, argv[i + 1], -1, NULL, 0);
            wchar_t* buffer = new wchar_t[len];
            MultiByteToWideChar(CP_UTF8, 0, argv[i + 1], -1, buffer, len);
            outputFile = buffer;
            delete[] buffer;
            i++;
        }
        else if (arg == "-dir" && i + 1 < argc)
        {
            int len = MultiByteToWideChar(CP_UTF8, 0, argv[i + 1], -1, NULL, 0);
            wchar_t* buffer = new wchar_t[len];
            MultiByteToWideChar(CP_UTF8, 0, argv[i + 1], -1, buffer, len);
            outputDir = buffer;
            delete[] buffer;
            i++;
        }
        else if (arg == "-d" && i + 1 < argc)
        {
            delaySeconds = std::atoi(argv[i + 1]);
            i++;
        }
        else if (arg == "-r" && i + 1 < argc)
        {
            std::vector<std::string> parts = split(argv[i + 1], ',');
            if (parts.size() == 4)
            {
                regionX = std::stoi(parts[0]);
                regionY = std::stoi(parts[1]);
                regionW = std::stoi(parts[2]);
                regionH = std::stoi(parts[3]);
                regionSpecified = true;
            }
            else
            {
                std::cerr << "Invalid region specification. Expected format: x,y,w,h\n";
                return -1;
            }
            i++;
        }
        else if (arg == "-format" && i + 1 < argc)
        {
            std::string fmt = argv[i + 1];
            std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);
            if (fmt == "png" || fmt == "jpg" || fmt == "bmp")
            {
                int len = MultiByteToWideChar(CP_UTF8, 0, fmt.c_str(), -1, NULL, 0);
                wchar_t* buffer = new wchar_t[len];
                MultiByteToWideChar(CP_UTF8, 0, fmt.c_str(), -1, buffer, len);
                imageFormat = buffer;
                delete[] buffer;
            }
            else
            {
                std::cerr << "Unsupported image format. Supported formats: png, jpg, bmp\n";
                return -1;
            }
            i++;
        }
        else if (arg == "-quality" && i + 1 < argc)
        {
            jpegQuality = std::atoi(argv[i + 1]);
            if (jpegQuality < 0 || jpegQuality > 100)
            {
                std::cerr << "Quality must be between 0 and 100.\n";
                return -1;
            }
            i++;
        }
        else if (arg == "-w" && i + 1 < argc)
        {
            int len = MultiByteToWideChar(CP_UTF8, 0, argv[i + 1], -1, NULL, 0);
            wchar_t* buffer = new wchar_t[len];
            MultiByteToWideChar(CP_UTF8, 0, argv[i + 1], -1, buffer, len);
            windowTitle = buffer;
            delete[] buffer;
            i++;
        }
        else if (arg == "-active")
        {
            captureActiveWindow = true;
        }
        else if (arg == "-m" && i + 1 < argc)
        {
            monitorIndex = std::atoi(argv[i + 1]);
            i++;
        }
        else if (arg == "-clipboard")
        {
            copyToClipboard = true;
        }
        else if (arg == "-show")
        {
            showAfterCapture = true;
        }
        else if (arg == "-p")
        {
            capturePointer = true;
        }
        else if (arg == "-timestamp")
        {
            annotateTimestamp = true;
        }
        else if (arg == "-repeat" && i + 2 < argc)
        {
            repeatInterval = std::atoi(argv[i + 1]);
            repeatCount = std::atoi(argv[i + 2]);
            repeatEnabled = true;
            i += 2;
        }
        else if (arg == "-listmonitors")
        {
            listMonitors = true;
        }
        else if (arg == "-listwindows")
        {
            listWindows = true;
        }
        else if (arg == "-v")
        {
            verbose = true;
        }
        else
        {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage();
            return -1;
        }
    }

    // If listing monitors is requested.
    if (listMonitors)
    {
        std::vector<MonitorInfo> monitors;
        if (EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors))
        {
            std::wcout << L"Monitors available:\n";
            for (size_t i = 0; i < monitors.size(); i++)
            {
                RECT r = monitors[i].rect;
                std::wcout << L"  [" << i << L"] (" << r.left << L"," << r.top << L") - ("
                    << r.right << L"," << r.bottom << L")\n";
            }
        }
        else
        {
            std::wcerr << L"Failed to enumerate monitors.\n";
        }
        return 0;
    }

    // If listing windows is requested.
    if (listWindows)
    {
        std::wstringstream ss;
        EnumWindows(EnumWindowsProc, (LPARAM)&ss);
        std::wcout << L"Visible windows:\n" << ss.str();
        return 0;
    }

    if (verbose)
        std::wcout << L"[INFO] Starting ShotCap...\n";

    if (delaySeconds > 0)
    {
        if (verbose)
            std::wcout << L"[INFO] Waiting for " << delaySeconds << L" seconds before capturing...\n";
        Sleep(delaySeconds * 1000);
    }

    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Ok)
    {
        std::cerr << "Failed to initialize GDI+." << std::endl;
        return -1;
    }

    // Lambda: Capture and save a screenshot using current settings.
    auto captureAndSave = [&](const std::wstring& fileName) -> bool
        {
            HDC hSourceDC = nullptr;
            RECT captureRect = { 0, 0, 0, 0 };

            // Determine capture method.
            if (captureActiveWindow)
            {
                HWND hWnd = GetForegroundWindow();
                if (!hWnd)
                {
                    std::wcerr << L"Failed to get active window." << std::endl;
                    return false;
                }
                if (verbose)
                    std::wcout << L"[INFO] Capturing active window.\n";
                if (!GetWindowRect(hWnd, &captureRect))
                {
                    std::cerr << "Failed to get active window rect." << std::endl;
                    return false;
                }
                hSourceDC = GetDC(hWnd);
                if (!hSourceDC)
                {
                    std::cerr << "Failed to get active window DC." << std::endl;
                    return false;
                }
            }
            else if (!windowTitle.empty())
            {
                HWND hWnd = FindWindowW(NULL, windowTitle.c_str());
                if (!hWnd)
                {
                    std::wcerr << L"Window not found: " << windowTitle << std::endl;
                    return false;
                }
                if (verbose)
                    std::wcout << L"[INFO] Capturing window: " << windowTitle << std::endl;
                if (!GetWindowRect(hWnd, &captureRect))
                {
                    std::cerr << "Failed to get window rect." << std::endl;
                    return false;
                }
                hSourceDC = GetDC(hWnd);
                if (!hSourceDC)
                {
                    std::cerr << "Failed to get window DC." << std::endl;
                    return false;
                }
            }
            else if (monitorIndex != -1)
            {
                if (verbose)
                    std::wcout << L"[INFO] Capturing monitor index: " << monitorIndex << std::endl;
                std::vector<MonitorInfo> monitors;
                if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors))
                {
                    std::cerr << "Failed to enumerate monitors." << std::endl;
                    return false;
                }
                if (monitorIndex < 0 || monitorIndex >= static_cast<int>(monitors.size()))
                {
                    std::cerr << "Invalid monitor index." << std::endl;
                    return false;
                }
                captureRect = monitors[monitorIndex].rect;
                HWND hDesktopWnd = GetDesktopWindow();
                hSourceDC = GetDC(hDesktopWnd);
                if (!hSourceDC)
                {
                    std::cerr << "Failed to get desktop DC." << std::endl;
                    return false;
                }
            }
            else if (regionSpecified)
            {
                if (verbose)
                    std::wcout << L"[INFO] Capturing region: (" << regionX << L"," << regionY
                    << L"," << regionW << L"," << regionH << L")\n";
                captureRect.left = regionX;
                captureRect.top = regionY;
                captureRect.right = regionX + regionW;
                captureRect.bottom = regionY + regionH;
                HWND hDesktopWnd = GetDesktopWindow();
                hSourceDC = GetDC(hDesktopWnd);
                if (!hSourceDC)
                {
                    std::cerr << "Failed to get desktop DC." << std::endl;
                    return false;
                }
            }
            else
            {
                if (verbose)
                    std::wcout << L"[INFO] Capturing full desktop.\n";
                HWND hDesktopWnd = GetDesktopWindow();
                hSourceDC = GetDC(hDesktopWnd);
                if (!hSourceDC)
                {
                    std::cerr << "Failed to get desktop DC." << std::endl;
                    return false;
                }
                captureRect.left = 0;
                captureRect.top = 0;
                captureRect.right = GetSystemMetrics(SM_CXSCREEN);
                captureRect.bottom = GetSystemMetrics(SM_CYSCREEN);
            }

            int capW = captureRect.right - captureRect.left;
            int capH = captureRect.bottom - captureRect.top;
            if (verbose)
                std::wcout << L"[INFO] Capture dimensions: " << capW << L"x" << capH << std::endl;

            HDC hCaptureDC = CreateCompatibleDC(hSourceDC);
            if (!hCaptureDC)
            {
                std::cerr << "Failed to create compatible DC." << std::endl;
                if (!captureActiveWindow && (!windowTitle.empty() || monitorIndex != -1 || regionSpecified))
                    ReleaseDC(NULL, hSourceDC);
                return false;
            }
            HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hSourceDC, capW, capH);
            if (!hCaptureBitmap)
            {
                std::cerr << "Failed to create compatible bitmap." << std::endl;
                DeleteDC(hCaptureDC);
                if (!captureActiveWindow && (!windowTitle.empty() || monitorIndex != -1 || regionSpecified))
                    ReleaseDC(NULL, hSourceDC);
                return false;
            }
            HGDIOBJ hOld = SelectObject(hCaptureDC, hCaptureBitmap);
            if (!hOld)
            {
                std::cerr << "Failed to select bitmap into DC." << std::endl;
                DeleteObject(hCaptureBitmap);
                DeleteDC(hCaptureDC);
                if (!captureActiveWindow && (!windowTitle.empty() || monitorIndex != -1 || regionSpecified))
                    ReleaseDC(NULL, hSourceDC);
                return false;
            }

            if (!BitBlt(hCaptureDC, 0, 0, capW, capH,
                hSourceDC, captureRect.left, captureRect.top, SRCCOPY | CAPTUREBLT))
            {
                std::cerr << "BitBlt failed." << std::endl;
                SelectObject(hCaptureDC, hOld);
                DeleteObject(hCaptureBitmap);
                DeleteDC(hCaptureDC);
                if (!captureActiveWindow && (!windowTitle.empty() || monitorIndex != -1 || regionSpecified))
                    ReleaseDC(NULL, hSourceDC);
                return false;
            }

            if (capturePointer)
            {
                CURSORINFO ci = { 0 };
                ci.cbSize = sizeof(ci);
                if (GetCursorInfo(&ci) && (ci.flags == CURSOR_SHOWING))
                {
                    int iconX = ci.ptScreenPos.x - captureRect.left;
                    int iconY = ci.ptScreenPos.y - captureRect.top;
                    DrawIconEx(hCaptureDC, iconX, iconY, ci.hCursor, 0, 0, 0, NULL, DI_NORMAL);
                    if (verbose)
                        std::wcout << L"[INFO] Mouse pointer drawn.\n";
                }
            }

            if (copyToClipboard)
            {
                if (verbose)
                    std::wcout << L"[INFO] Copying image to clipboard...\n";
                HGLOBAL hDib = CreateDIBFromHBITMAP(hCaptureBitmap);
                if (!hDib)
                {
                    std::cerr << "Failed to create DIB for clipboard." << std::endl;
                }
                else
                {
                    if (OpenClipboard(NULL))
                    {
                        EmptyClipboard();
                        SetClipboardData(CF_DIB, hDib);
                        CloseClipboard();
                        if (verbose)
                            std::wcout << L"[INFO] Image copied to clipboard successfully.\n";
                    }
                    else
                    {
                        std::cerr << "Failed to open clipboard." << std::endl;
                        GlobalFree(hDib);
                    }
                }
            }

            // Create a GDI+ Bitmap from the captured HBITMAP.
            Bitmap* bmp = new Bitmap(hCaptureBitmap, NULL);
            if (!bmp)
            {
                std::cerr << "Failed to create GDI+ Bitmap." << std::endl;
                SelectObject(hCaptureDC, hOld);
                DeleteObject(hCaptureBitmap);
                DeleteDC(hCaptureDC);
                if (!captureActiveWindow && (!windowTitle.empty() || monitorIndex != -1 || regionSpecified))
                    ReleaseDC(NULL, hSourceDC);
                return false;
            }

            // If timestamp annotation is enabled, overlay current time.
            if (annotateTimestamp)
            {
                std::time_t t = std::time(nullptr);
                struct tm tmTime;
                localtime_s(&tmTime, &t);
                std::wstringstream ts;
                ts << std::put_time(&tmTime, L"%Y-%m-%d %H:%M:%S");
                AnnotateImage(bmp, ts.str(), verbose);
            }

            const WCHAR* mimeType = L"image/png";
            if (imageFormat == L"jpg")
                mimeType = L"image/jpeg";
            else if (imageFormat == L"bmp")
                mimeType = L"image/bmp";

            CLSID encoderClsid;
            if (GetEncoderClsid(mimeType, &encoderClsid) < 0)
            {
                std::cerr << "Image encoder not found for specified format." << std::endl;
                delete bmp;
                SelectObject(hCaptureDC, hOld);
                DeleteObject(hCaptureBitmap);
                DeleteDC(hCaptureDC);
                if (!captureActiveWindow && (!windowTitle.empty() || monitorIndex != -1 || regionSpecified))
                    ReleaseDC(NULL, hSourceDC);
                return false;
            }

            EncoderParameters encoderParams;
            ULONG qualityParam = jpegQuality;
            if (imageFormat == L"jpg")
            {
                encoderParams.Count = 1;
                encoderParams.Parameter[0].Guid = EncoderQuality;
                encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
                encoderParams.Parameter[0].NumberOfValues = 1;
                encoderParams.Parameter[0].Value = &qualityParam;
            }
            else
            {
                encoderParams.Count = 0;
            }

            Status stat;
            if (imageFormat == L"jpg")
                stat = bmp->Save(fileName.c_str(), &encoderClsid, &encoderParams);
            else
                stat = bmp->Save(fileName.c_str(), &encoderClsid, NULL);

            if (stat != Ok)
                std::wcerr << L"Failed to save screenshot (" << fileName << L"). Status code: " << stat << std::endl;
            else
                std::wcout << L"Screenshot saved as " << fileName << std::endl;

            if (showAfterCapture)
            {
                if (verbose)
                    std::wcout << L"[INFO] Opening image...\n";
                ShellExecuteW(NULL, L"open", fileName.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }

            delete bmp;
            SelectObject(hCaptureDC, hOld);
            DeleteObject(hCaptureBitmap);
            DeleteDC(hCaptureDC);
            if (!captureActiveWindow && (!windowTitle.empty() || monitorIndex != -1 || regionSpecified))
                ReleaseDC(NULL, hSourceDC);

            return true;
        };

    // If repeat mode is enabled, capture multiple times.
    if (repeatEnabled && repeatCount > 0)
    {
        std::wstring baseName = outputFile;
        std::wstring extension = L"";
        size_t pos = outputFile.find_last_of(L'.');
        if (pos != std::wstring::npos)
        {
            baseName = outputFile.substr(0, pos);
            extension = outputFile.substr(pos);
        }
        else
        {
            if (imageFormat == L"jpg")
                extension = L".jpg";
            else if (imageFormat == L"bmp")
                extension = L".bmp";
            else
                extension = L".png";
        }
        for (int i = 0; i < repeatCount; i++)
        {
            std::wstringstream ss;
            ss << baseName << L"_" << std::setfill(L'0') << std::setw(3) << i + 1 << extension;
            std::wstring fileName = ss.str();
            if (!outputDir.empty())
            {
                fileName = outputDir + L"\\" + fileName;
            }
            if (!captureAndSave(fileName))
            {
                std::wcerr << L"[ERROR] Capture iteration " << i + 1 << L" failed.\n";
            }
            if (i < repeatCount - 1)
            {
                if (verbose)
                    std::wcout << L"[INFO] Waiting " << repeatInterval << L" seconds before next capture...\n";
                Sleep(repeatInterval * 1000);
            }
        }
    }
    else
    {
        std::wstring fileName = outputFile;
        if (!outputDir.empty())
        {
            fileName = outputDir + L"\\" + outputFile;
        }
        captureAndSave(fileName);
    }

    GdiplusShutdown(gdiplusToken);
    if (verbose)
        std::wcout << L"[INFO] Done.\n";
    return 0;
}
