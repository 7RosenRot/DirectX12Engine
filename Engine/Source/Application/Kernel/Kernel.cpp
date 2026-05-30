#include <Application/Kernel/Kernel.hpp>

void Kernel::AppInitialize(
  HINSTANCE hInstance,
  int nCmdShow,
  UINT WindowWidth,
  UINT WindowHeight,
  const std::wstring& WindowName
) {
  m_WindowWidth = WindowWidth;
  m_WindowHeight = WindowHeight;
  m_WindowName = WindowName;

  m_pWindow = std::make_unique<Window>();
  m_pWindow->SetWindow(hInstance, nCmdShow);

  Input::Initialize(Window::GetHwnd());
  Input::SetMouseLock(true);

  m_pRenderer = std::make_unique<D3D12Engine::DirectX12Graphics>(
    Window::GetHwnd(), m_WindowWidth, m_WindowHeight
  );
  
  m_pRendererInstance = m_pRenderer.get();
  
  m_pRenderer->OnInitialize();
  m_pRenderer->OnResize(m_WindowWidth, m_WindowHeight);

  m_AppRunning = true;
}

void Kernel::AppRun() {
  MSG msg{};
  ZeroMemory(&msg, sizeof(msg));

  while (m_AppRunning) {
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        m_AppRunning = false;

        break;
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    if (m_pRenderer != nullptr) {
      m_pRenderer->OnUpdate();
      m_pRenderer->OnRender();
    }
  }

  AppDestroy();
}

void Kernel::AppDestroy() {
  if (m_pRenderer != nullptr) {
    m_pRenderer->OnDestroy();

    m_pRenderer.reset();
    m_pRendererInstance = nullptr;
  }

  m_pWindow.reset();
}