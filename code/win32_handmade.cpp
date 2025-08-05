#include <stdint.h>
#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// TODO: This is a global for now.
global_variable bool Running;
global_variable BITMAPINFO BitMapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;

internal void Win32ResizeDIBSection(int Width, int Height) {
  // TODO: Bulletproof this, maybe don't free first, free after, then free first
  // if that fails;

  if (BitmapMemory) {
    VirtualFree(BitmapMemory, 0, MEM_RELEASE);
  }

  BitmapWidth = Width;
  BitmapHeight = Height;

  BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
  BitMapInfo.bmiHeader.biWidth = BitmapWidth;
  BitMapInfo.bmiHeader.biHeight = -BitmapHeight;
  BitMapInfo.bmiHeader.biPlanes = 1;
  BitMapInfo.bmiHeader.biBitCount = 32;
  BitMapInfo.bmiHeader.biCompression = BI_RGB;

  int BytesPerPixel = 4;
  int BitmapMemorySize = BytesPerPixel * (BitmapWidth * BitmapHeight);
  BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  int Pitch = Width * BytesPerPixel;
  uint8 *Row = (uint8 *)BitmapMemory;
  for (int Y = 0; Y < BitmapHeight; ++Y) {
    uint8 *Pixel = (uint8 *)Row;
    for (int X = 0; X < BitmapWidth; ++X) {
      /*
       Pixel in memory: BB GG RR xx
       */
      *Pixel = 0;
      ++Pixel;

      *Pixel = 0;
      ++Pixel;

      *Pixel = 255;
      ++Pixel;

      *Pixel = 0;
      ++Pixel;
    }
    Row += Pitch;
  }
}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X,
                                int Y, int Width, int Height) {

  int WindowWidth = WindowRect->right - WindowRect->left;
  int WindowHeight = WindowRect->bottom - WindowRect->top;
  StretchDIBits(DeviceContext,
                /*
                X, Y, Width, Height, X, Y, Width, Height,*/
                0, 0, BitmapWidth, BitmapHeight, 0, 0, WindowWidth,
                WindowHeight, BitmapMemory, &BitMapInfo, DIB_RGB_COLORS,
                SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallBack(HWND Window, UINT Message,
                                         WPARAM wParam, LPARAM lParam) {
  LRESULT Result = 0;
  switch (Message) {
  case WM_SIZE: {
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    int Height = ClientRect.bottom - ClientRect.top;
    int Width = ClientRect.right - ClientRect.left;
    Win32ResizeDIBSection(Width, Height);
    OutputDebugStringA("WM_SIZE\n");
  } break;
  case WM_DESTROY: {
    // TODO: Handle this as an error - recreate window?
    Running = false;
    OutputDebugStringA("WM_DESTROY\n");
  } break;
  case WM_CLOSE: {
    // TODO: Handle this with a message to the user?
    Running = false;
    OutputDebugStringA("WM_CLOSE\n");
  } break;
  case WM_ACTIVATEAPP: {
    OutputDebugStringA("WM_ACTIVEAPP\n");
  } break;
  case WM_PAINT: {
    OutputDebugStringA("WM_PAINT\n");
    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);
    int X = Paint.rcPaint.left;
    int Y = Paint.rcPaint.top;
    int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
    int Width = Paint.rcPaint.right - Paint.rcPaint.left;
    RECT ClientRect;
    Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
    EndPaint(Window, &Paint);
  } break;
  default: {
    // OutputDebugStringA("default\n");
    Result = DefWindowProc(Window, Message, wParam, lParam);
  } break;
  }
  return (Result);
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance,
                     PSTR CommandLine, int ShowCode) {
  WNDCLASS WindowClass = {};

  // TODO: Check if this matters
  WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = Win32MainWindowCallBack;
  WindowClass.hInstance = Instance;
  // WindowClass.hIcon = hInstance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";
  if (RegisterClass(&WindowClass)) {
    HWND WindowHandle = CreateWindowEx(
        0, WindowClass.lpszClassName, "Handmade Hero",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
    if (WindowHandle) {
      Running = true;
      MSG Message;
      {
        while (Running) {
          BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
          if (MessageResult > 0) {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
          } else {
            break;
          }
        }
      }
    } else {
      // TODO: Logging
    }
  } else {
    // TODO: Logging
  }

  return 0;
}
