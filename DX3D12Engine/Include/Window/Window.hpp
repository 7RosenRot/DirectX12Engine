#pragma once
#include <windows.h>
#include <stdexcept>
#include <iostream>
#include <Include/Graphics/InterfaceDirectX12.hpp>

namespace D3D12Engine {
  class Window {
  public:
    Window(InterfaceDirectX12* m_InterfaceDirectX12, HINSTANCE hInstance, int m_CmdShow);
    virtual ~Window();

    static HWND getHwnd() { return hwnd; }
    virtual void run_GameLoop();
  protected:
    //Game loop parameter (as long as m_isRunning is "true" -> game loop still running)
    bool isRunning{true};

    //Minimal boreders
    static const int min_WndWidth{500};
    static const int min_WndHeight{250};
    
    //Window descriptors & procedure
    void* handle{nullptr};
    static inline HWND hwnd{nullptr};
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  };
}