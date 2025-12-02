#include <Include/Game/Game.hpp>
#include <iostream>

D3D12Engine::Game::Game() {
    D3D12Engine::InterfaceDirectX12 MyApplication(720, 1280, L"MyApplication");
    HINSTANCE hInstance = nullptr;
    int CmdShow = SW_SHOW;

    ptr_Window = std::make_unique<Window>(&MyApplication, hInstance, CmdShow);
}

void D3D12Engine::Game::run_GameLoop() {
    MSG msg{};

    bool m_isRunnig{true};

    while (m_isRunnig) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_isRunnig = false;
                
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(1);
    }
}

D3D12Engine::Game::~Game() {}