#define UNICODE
#define _UNICODE

#include <../Assets/Resource/resource.h>

#include <Application/Window/Window.hpp>
#include <Application/Kernel/Kernel.hpp>
#include <Application/Input/Input.hpp>

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  IRenderer* renderWindow = Kernel::GetRendererInstance();

  switch (msg) {
  case WM_GETMINMAXINFO: {
      LPMINMAXINFO setBorders = (LPMINMAXINFO)lParam;

      setBorders->ptMinTrackSize.x = 500;
      setBorders->ptMinTrackSize.y = 250;
    }
    return 0;
  
  case WM_SIZE: {
      UINT WindowWidth  = LOWORD(lParam);
      UINT WindowHeight = HIWORD(lParam);

      if (renderWindow != nullptr) {
        renderWindow->OnResize(WindowWidth, WindowHeight);
      }
    }
    return 0;

  case WM_KEYDOWN: {
      if (wParam == VK_ESCAPE) {
        Input::SetMouseLock(false);
      }

      Input::SetStatusKey(static_cast<UINT8>(wParam), true);
    }
    return 0;

  case WM_KEYUP: {
      Input::SetStatusKey(static_cast<UINT8>(wParam), false);
    }
    return 0;

  case WM_LBUTTONDOWN: {
      Input::SetMouseLock(true);
    }
    return 0;

  case WM_MOUSEMOVE: {
      if (Input::IsMouseLocked()) {
        Input::OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      }
    }
    return 0;

  case WM_PAINT: {
      if (renderWindow != nullptr) {
        renderWindow->OnUpdate();
        
        renderWindow->OnRender();
      }

      ValidateRect(hwnd, NULL);
    }
    return 0;

  case WM_DESTROY: {
      PostQuitMessage(0);
    }
    return 0;
    
  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }

  return 0;
}

bool Window::SetWindow(HINSTANCE hInstance, int nCmdShow) {
  WNDCLASSEX wndClass{0};

  wndClass.cbSize = sizeof(WNDCLASSEX);
  wndClass.lpszClassName = L"WindowName";
  wndClass.lpszMenuName = L"";

  wndClass.style = CS_HREDRAW | CS_VREDRAW;
  wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
  wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
  wndClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
  wndClass.hInstance = hInstance;

  wndClass.lpfnWndProc = &WndProc;

  RegisterClassEx(&wndClass);

  RECT WndRect{ 0, 0, static_cast<long>(Kernel::GetWindowWidth()), static_cast<long>(Kernel::GetWindowHeight()) };
  AdjustWindowRect(&WndRect, WS_OVERLAPPEDWINDOW, false);

  m_hwnd = CreateWindowExW(
    NULL, L"WindowName", Kernel::GetWindowName(),
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
    WndRect.right - WndRect.left,
    WndRect.bottom - WndRect.top,
    NULL, NULL, hInstance, NULL
  );

  if (!m_hwnd) {
    return false;
  }

  ShowWindow(static_cast<HWND>(m_hwnd), nCmdShow);
  UpdateWindow(m_hwnd);

  return true;
}