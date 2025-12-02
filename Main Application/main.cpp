#include <Include/Graphics/InterfaceDirectX12.hpp>
#include <Include/Window/Window.hpp>
#include <Include/Game/Game.hpp>

int main(void) {
    D3D12Engine::Game game;
    game.run_GameLoop();

    return 0;
}

/*
    D3D12Engine::InterfaceDirectX12 MyApplication(720, 1280, L"MyApplication");
    HINSTANCE hInstance = nullptr;
    int CmdShow = SW_SHOW;

    D3D12Engine::Window::Window(&MyApplication, hInstance, CmdShow);
*/