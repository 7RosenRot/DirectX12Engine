#pragma once
#include <Include/Graphics/InterfaceDirectX12.hpp>

#include <windows.h>

namespace D3D12Engine {
  class Window {
  public:
    Window();
    virtual ~Window();

    static HWND GetHwnd() { return m_hWnd; }
    static void Run(InterfaceDirectX12* InterfaceDirectX12, HINSTANCE hInstance, int CmdShow);
  protected:
    bool m_isRunning{true};
    
    static inline HWND m_hWnd{nullptr};
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  };
}