#pragma once
#include <windows.h>
#include <stdexcept>
#include <iostream>
#include <Include/Graphics/InterfaceDirectX12.hpp>

namespace D3D12Engine {
  class Window {
  public:
    Window(InterfaceDirectX12* InterfaceDirectX12, HINSTANCE hInstance, int CmdShow);
    virtual ~Window();

    static HWND getHwnd() { return m_hWnd; }
    virtual void RunGameLoop();
  protected:
    bool m_isRunning{true};

    static const int m_minWndWidth{500};
    static const int m_minWndHeight{250};
    
    static inline HWND m_hWnd{nullptr};
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  };
}