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
  m_pInstance = this;

  m_pWindow = std::make_unique<Window>();
  m_pWindow->SetWindow(hInstance, nCmdShow);

  Input::Initialize(Window::GetHwnd());
  Input::SetMouseLock(false);

  m_pRenderer = std::make_unique<D3D12Engine::DirectX12Graphics>(
    Window::GetHwnd(), WindowWidth, WindowHeight
  );
  m_pRendererInstance = m_pRenderer.get();
  
  m_pRenderer->OnInitialize();
  m_pRenderer->OnResize(WindowWidth, WindowHeight);
  m_pRenderer->ResizeViewport(WindowWidth, WindowHeight);

  m_pAssetManager = std::make_unique<AssetManager>();
  m_pAssetManager->Initialize(
    m_pRenderer->GetDevice(),
    m_pRenderer->GetSrvAllocator(),
    m_pRenderer->GetContextPool(),
    m_pRenderer->GetCommandQueue()
  );

  m_pScene = std::make_unique<Scene>();
  m_pScene->Initialize(m_pAssetManager.get());
  m_pScene->GetActiveCamera().GetTransform().SetPosition(0.0F, 3.5F, -10.0F);

  m_pEngineUI = std::make_unique<EngineUI>(
    Window::GetHwnd(), m_pRenderer.get(), m_pScene.get()
  );
  m_pEngineUI->Initialize(
    m_pRenderer->GetDevice(),
    m_pRenderer->GetCommandQueueResource(),
    D3D12Engine::GraphicsCore::FrameCount,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    *(m_pRenderer->GetSrvAllocator())
  );

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

    if (Window::IsWindowMinimized()) {
      Sleep(10);

      continue;
    }

    RenderFrame();
  }

  AppDestroy();
}

void Kernel::RenderFrame() {
  if (
    m_pRenderer && m_pScene && m_pEngineUI
  ) {
    m_pEngineUI->UpdateLayout();
    m_pScene->UpdateScene(0.10F, 0.05F, m_pEngineUI->IsGizmoActive());
    
    m_pRenderer->BeginFrame();
    
    m_pScene->RenderScene(*m_pRenderer, m_pEngineUI->GetSelectedObject());
    m_pRenderer->PrepareUIContext();
    
    m_pEngineUI->BeginUI();
    m_pEngineUI->DrawUI();
    m_pEngineUI->EndUI();
    
    m_pRenderer->EndFrame();
  }
}

void Kernel::AppDestroy() {
  if (m_pEngineUI != nullptr) {
    m_pEngineUI.reset();
  }

  if (m_pScene != nullptr) {
    m_pScene.reset();
  }

  if (m_pAssetManager != nullptr) {
    m_pAssetManager.reset();
  }

  if (m_pRenderer != nullptr) {
    m_pRenderer->GetCommandQueue()->Flush();
    m_pRenderer.reset();
  }

  if (m_pWindow != nullptr) {
    m_pWindow.reset();
  }
}