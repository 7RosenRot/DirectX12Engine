#include <Include/Graphics/DirectX12Graphics.hpp>
#include <Include/Window/Window.hpp>

int main(void) {
  D3D12Engine::DirectX12Graphics MyApplication(720, 1280, L"DirectX12 Application");
  HINSTANCE hInstance = nullptr;
  int m_CmdShow = SW_SHOW;

  return D3D12Engine::Window::RunApplication(&MyApplication, hInstance, m_CmdShow);
}