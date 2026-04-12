#define UNICODE
#define _UNICODE

#include <Assets/resource/resource.h>
#include <Include/Window/Window.hpp>
#include <Include/Graphics/Core/DirectX12Graphics.hpp>

LRESULT CALLBACK D3D12Engine::Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  DirectX12Graphics* renderWindow = reinterpret_cast<DirectX12Graphics*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

  switch (msg) {
  case WM_CREATE: {
      LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
  case WM_GETMINMAXINFO: {
      LPMINMAXINFO setBorders = (LPMINMAXINFO)lParam;

      setBorders->ptMinTrackSize.x = 500;
      setBorders->ptMinTrackSize.y = 250;
    }
    return 0;
  case WM_SIZE: {
      UINT WindowWidth = LOWORD(lParam);
      UINT WindowHeight = HIWORD(lParam);

      renderWindow->OnResize(WindowWidth, WindowHeight);

      renderWindow->OnUpdate();
      renderWindow->OnRender();
    }
    return 0;
  case WM_PAINT: {
      if (renderWindow) {
        renderWindow->OnUpdate();
        renderWindow->OnRender();
      }
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

D3D12Engine::Window::Window() {}

void D3D12Engine::Window::Run(InterfaceDirectX12* InterfaceDirectX12, HINSTANCE hInstance, int CmdShow) {
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

  static const ATOM wndClassID = RegisterClassEx(&wndClass);

  RECT WndRect{0, 0, static_cast<LONG>(InterfaceDirectX12->GetWindowWidth()), static_cast<LONG>(InterfaceDirectX12->GetWindowHeight())};
  AdjustWindowRect(&WndRect, WS_OVERLAPPEDWINDOW, false);

  m_hWnd = CreateWindowExW(
    NULL,
    MAKEINTATOM(wndClassID),
    InterfaceDirectX12->GetWindowName(),
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    WndRect.right - WndRect.left,
    WndRect.bottom - WndRect.top,
    NULL,
    NULL,
    hInstance,
    InterfaceDirectX12
  );

  InterfaceDirectX12->OnInitialize();
  ShowWindow(static_cast<HWND>(m_hWnd), CmdShow);
  
  MSG msg{};
  ZeroMemory(&msg, sizeof(msg));

  while (msg.message != WM_QUIT) {
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  InterfaceDirectX12->OnDestroy();
}

D3D12Engine::Window::~Window() {
  DestroyWindow(static_cast<HWND>(m_hWnd));
}