#pragma once
#include <Include/Window/Window.hpp>

namespace D3D12Engine {
    class Game {
    public:
        Game();
        ~Game();

        virtual void gameLoop() final;

    private:
        bool m_IsRunnig{true};
        std::unique_ptr<Window> m_Display{nullptr};
    };
}