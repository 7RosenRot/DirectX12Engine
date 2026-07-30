#define UNICODE
#define _UNICODE

#include <imgui.h>
#include <imgui_internal.h>
#include <string.h>

#include <Application/Window/Resource/resource.h>
#include <Application/Window/Window.hpp>
#include <Application/Kernel/Kernel.hpp>
#include <Application/Input/Input.hpp>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
    return true;
  }
  
  D3D12Engine::DirectX12Graphics* pRenderInstance = Kernel::GetRendererInstance();
  Kernel* pKernelInstance = Kernel::GetKernelInstance();

  switch (msg) {
  case WM_GETMINMAXINFO: {
      LPMINMAXINFO setBorders = (LPMINMAXINFO)lParam;

      setBorders->ptMinTrackSize.x = 500;
      setBorders->ptMinTrackSize.y = 250;
    }
    return 0;
  
  case WM_SIZE: {
      if (wParam == SIZE_MINIMIZED) {
        return 0;
      }

      UINT WindowWidth  = LOWORD(lParam);
      UINT WindowHeight = HIWORD(lParam);

      if (pRenderInstance != nullptr) {
        pRenderInstance->OnResize(WindowWidth, WindowHeight);
      }

      if (pKernelInstance != nullptr) {
        pKernelInstance->RenderFrame();
      }
    }
    return 0;

  case WM_KEYDOWN: {
      if (wParam == VK_ESCAPE) {
        Input::SetMouseLock(false);
      }
      if (wParam == VK_SPACE) {
        if (ImGui::GetCurrentContext() == nullptr || !ImGui::GetIO().WantCaptureKeyboard) {
          Input::SetMouseLock(true);
        }
      }

      Input::SetStatusKey(static_cast<UINT8>(wParam), true);
    }
    return 0;

  case WM_KEYUP: {
      Input::SetStatusKey(static_cast<UINT8>(wParam), false);
    }
    return 0;

  case WM_MBUTTONDOWN: {
      ImGuiContext* g = ImGui::GetCurrentContext();
      bool overViewport = false;
      if (g != nullptr && g->HoveredWindow != nullptr) {
        if (strcmp(g->HoveredWindow->Name, "Scene Viewport") == 0) {
          overViewport = true;
        }
      }
      if (g == nullptr || overViewport) {
        Input::SetMouseLock(true);
        Input::SetMmbMode(true);
      }
    }
    return 0;

  case WM_MBUTTONUP: {
      if (Input::IsMmbMode()) {
        Input::SetMouseLock(false);
        Input::SetMmbMode(false);
      }
    }
    return 0;

  case WM_MOUSEWHEEL: {
      ImGuiContext* g = ImGui::GetCurrentContext();
      bool overViewport = false;
      if (g != nullptr && g->HoveredWindow != nullptr) {
        if (strcmp(g->HoveredWindow->Name, "Scene Viewport") == 0) {
          overViewport = true;
        }
      }
      if (Input::IsMouseLocked() || g == nullptr || overViewport) {
        float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
        Input::SetMouseWheelDelta(delta);
      }
    }
    return 0;

  case WM_MOUSEMOVE: {
      if (Input::IsMouseLocked()) {
        Input::OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      }
    }
    return 0;

  case WM_PAINT: {
      PAINTSTRUCT PaintStruct;
      HDC hdc = BeginPaint(hwnd, &PaintStruct);
      EndPaint(hwnd, &PaintStruct);
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