#include <Include/Game/Game.hpp>

D3D12Engine::Game::Game() {
    m_Display = std::make_unique<Window>();
}

void D3D12Engine::Game::gameLoop() {
    while (m_IsRunnig) {
        MSG m_Message{};

        while (PeekMessage(&m_Message, NULL, 0, 0, PM_REMOVE)) {
            if (m_Message.message == WM_QUIT) {
                m_IsRunnig = false;
                break;
            }
        }
        TranslateMessage(&m_Message);
        DispatchMessage(&m_Message);
    }
    Sleep(1);
}

D3D12Engine::Game::~Game() {}