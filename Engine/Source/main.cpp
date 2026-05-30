#include <stdexcept>
#include <iostream>

#include <Application/Kernel/Kernel.hpp>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  Kernel Engine;
  
  //                 Screen Resolution 1280x720 ↓     Window Name ↓
  Engine.AppInitialize(hInstance, SW_SHOW, 1280, 720, L"DirectX12 Application");
  Engine.AppRun();

  return 0;
}
