#include <Include/Game/Game.hpp>

int main(void) {
    D3D12Engine::Game game{};
    game.gameLoop();

    return 0;
}