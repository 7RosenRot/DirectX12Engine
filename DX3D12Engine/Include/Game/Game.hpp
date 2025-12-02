#pragma once
#include <Include/Window/Window.hpp>

namespace D3D12Engine {
    class Game {
    public:
        Game();
        virtual ~Game();

        virtual void run_GameLoop() final;
    private:
        std::unique_ptr<Window> ptr_Window{};
    };
}