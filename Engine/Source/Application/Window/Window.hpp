#pragma once

#include <windows.h>
#include <windowsx.h>
#include <string>
#include <functional>
#include <imgui_impl_win32.h>

class Window {
 public:
  Window() = default;
  virtual ~Window() = default;
  
  static HWND GetHwnd() { return m_hwnd; }
  static bool IsWindowMinimized() { return IsIconic(m_hwnd) != 0; }
  bool SetWindow(HINSTANCE hInstance, int nCmdShow);
 protected:
  bool m_isRunning = true;
  
  static inline HWND m_hwnd = nullptr;
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};