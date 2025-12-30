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
    virtual void run_GameLoop();
  protected:
    //Game loop parameter (as long as m_isRunning is "true" -> game loop still running)
    bool m_isRunning{true};

    //Minimal boreders
    static const int min_WndWidth{500};
    static const int min_WndHeight{250};
    
    //Window descriptors & procedure
    static inline HWND m_hWnd{nullptr};
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  };
}