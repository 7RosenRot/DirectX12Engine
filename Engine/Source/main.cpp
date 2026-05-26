#include <stdexcept>
#include <iostream>

#include <Include/Graphics/Core/DirectX12Graphics.hpp>
#include <Include/Window/Window.hpp>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int CmdShow) {
  //                       Screen Resolution 1280x720 ↓                        ↓ Window Name
  D3D12Engine::DirectX12Graphics WindowParameters(1280, 720, 16, 9, L"DirectX12 Application");
  //                                                           ↑ Aspect Ratio 16:9

  D3D12Engine::Window MyApplication;

  try {
    MyApplication.Run(&WindowParameters, hInstance, CmdShow);
  } catch (const std::runtime_error& error) {
    std::cerr << "ERROR: " << error.what() << std::endl;

    MessageBoxA(NULL, error.what(), "DirectX12Engine Error", MB_ICONERROR);

    return -1;
  }

  return 0;
}
