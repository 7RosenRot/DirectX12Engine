#include <Include/Graphics/DirectX12Graphics.hpp>
#include <Include/Window/Window.hpp>
#include <stdexcept>
#include <iostream>

int main(void) {
  D3D12Engine::DirectX12Graphics WinodowParameters(720, 1280, L"DirectX12 Application");
  HINSTANCE hInstance = nullptr;
  int m_CmdShow = SW_SHOW;

  D3D12Engine::Window MyApplication;

  try {
    MyApplication.Run(&WinodowParameters, hInstance, m_CmdShow);
  } catch (const std::runtime_error& error) {
    std::cerr << "ERROR: " << error.what() << std::endl;

    MessageBoxA(NULL, error.what(), "DirectX12Engine Error", MB_ICONERROR);

    return -1;
  }

  return 0;
}
