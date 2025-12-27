#include <Include/Graphics/InterfaceDirectX12.hpp>
#include <Include/Window/Window.hpp>

int main(void) {
  D3D12Engine::InterfaceDirectX12 MyApplication(720, 1280, L"DirectX12 Application");
  HINSTANCE hInstance = nullptr;
  int m_CmdShow = SW_SHOW;

  D3D12Engine::Window wnd(&MyApplication, hInstance, m_CmdShow);
  wnd.run_GameLoop();

  return 0;
}