#pragma once
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdexcept>
#include <iostream>
#include <Include/Graphics/InterfaceDirectX12.hpp>

namespace D3D12Engine {
    class Window {
    public:
        Window(InterfaceDirectX12* m_InterfaceDirectX12, HINSTANCE hInstance, int nCmdShow);
        ~Window();

        static HWND GetHwnd() { return m_hwnd; };
    protected:
        void* m_handle{nullptr};
        bool fullscreen{false};

        static HWND m_hwnd;
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}