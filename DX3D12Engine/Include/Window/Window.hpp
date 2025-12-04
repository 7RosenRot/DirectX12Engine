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

        virtual void run_GameLoop();
    protected:
        //Game loop parameter (as long as m_isRunning is "true" -> game loop still running)
        bool m_isRunning{true};

        //Minimal boreders
        static const int min_WndWidth{500};
        static const int min_WndHeight{250};
        
        //Window descriptors & procedure
        void* m_handle{nullptr};
        static HWND m_hwnd;
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}