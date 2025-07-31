#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

// TODO: This is a global for now.
global_variable bool Running;
global_variable BITMAPINFO BitMapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitMapDeviceContext;

internal void Win32ResizeDIBSection(int Width, int Height) {
  // TODO: Bulletproof this, maybe don't free first, free after, then free first
  // if that fails;

  // TODO: Free our DIBSection
  //

  if (BitmapHandle) {
    DeleteObject(BitmapHandle);
  }
  if (!BitMapDeviceContext) {

    // TODO: should we recrate these udner cretain special circumstances
    BitMapDeviceContext = CreateCompatibleDC(0);
  }

  BITMAPINFO BitMapInfo;
  BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
  BitMapInfo.bmiHeader.biWidth = Width;
  BitMapInfo.bmiHeader.biHeight = Height;
  BitMapInfo.bmiHeader.biPlanes = 1;
  BitMapInfo.bmiHeader.biBitCount = 32;
  BitMapInfo.bmiHeader.biCompression = BI_RGB;

  BitmapHandle = CreateDIBSection(BitMapDeviceContext, &BitMapInfo,
                                  DIB_RGB_COLORS, &BitmapMemory, 0, 0);
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width,
                                int Height) {
  StretchDIBits(DeviceContext, X, Y, Width, Height, X, Y, Width, Height,
                BitmapMemory, &BitMapInfo, DIB_RGB_COLORS, SRCCOPY);
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
    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);
    int X = Paint.rcPaint.left;
    int Y = Paint.rcPaint.top;
    int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
    int Width = Paint.rcPaint.right - Paint.rcPaint.left;
    Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
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
