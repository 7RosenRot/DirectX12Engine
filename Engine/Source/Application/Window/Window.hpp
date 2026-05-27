#pragma once

#include <windows.h>
#include <windowsx.h>
#include <string>

class Window {
 public:
  Window() = default;
  virtual ~Window() = default;
  
  static HWND GetHwnd() { return m_hwnd; }
  bool SetWindow(HINSTANCE hInstance, int nCmdShow);
 protected:
  bool m_isRunning = true;
  
  static inline HWND m_hwnd = nullptr;
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};