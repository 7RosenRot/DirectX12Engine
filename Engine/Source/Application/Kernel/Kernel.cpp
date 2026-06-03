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
  Input::SetMouseLock(false);

  m_pRenderer = std::make_unique<D3D12Engine::DirectX12Graphics>(
    Window::GetHwnd(), m_WindowWidth, m_WindowHeight
  );
  
  m_pRendererInstance = m_pRenderer.get();
  
  m_pRenderer->OnInitialize();
  m_pRenderer->OnResize(m_WindowWidth, m_WindowHeight);
  m_pRenderer->ResizeViewport(m_WindowWidth, m_WindowHeight);

  m_pScene = std::make_unique<Scene>();
  m_pScene->GetActiveCamera().GetTransform().SetPosition(0.0F, 3.5F, -10.0F);

  m_pEngineUI = std::make_unique<EngineUI>(
    Window::GetHwnd(), m_pRenderer.get(), m_pScene.get()
  );

  // ↓ Load models ↓
    // ↓ bastard_gun_corpus ↓  
    auto bastard_gun_corpus_model = m_pRenderer->LoadModel("Engine/Assets/Models/bastard_gun/bastard_gun_corpus.obj");
    auto bastard_gun_corpus_texture = m_pRenderer->LoadTexture("Engine/Assets/Models/bastard_gun/bastard_gun_corpus.png");

    auto bastard_gun_corpus = std::make_shared<GameObject>(bastard_gun_corpus_model, bastard_gun_corpus_texture, "bastard_gun_corpus");
    bastard_gun_corpus->GetTransform().SetPosition(0.0F, 0.0F, 0.0F);

    m_pScene->AddGameObject("bastard_gun_corpus", bastard_gun_corpus);
    // ↑ bastard_gun_corpus ↑
    
    // ↓ bastard_gun_corob ↓ 
    auto bastard_gun_corob_model = m_pRenderer->LoadModel("Engine/Assets/Models/bastard_gun/bastard_gun_corob.obj");
    auto bastard_gun_corob_texture = m_pRenderer->LoadTexture("Engine/Assets/Models/bastard_gun/bastard_gun_corob.png");

    auto bastard_gun_corob = std::make_shared<GameObject>(bastard_gun_corob_model, bastard_gun_corob_texture, "bastard_gun_corob");
    bastard_gun_corob->GetTransform().SetPosition(0.0F, 0.0F, 0.0F);

    m_pScene->AddGameObject("bastard_gun_corob", bastard_gun_corob);
    // ↑ bastard_gun_corob ↑
  // ↑ Load models ↑
  
  m_AppRunning = true;
}

void Kernel::AppRun() {
  MSG msg{};
  ZeroMemory(&msg, sizeof(msg));

  static UINT sceneWidth = m_WindowWidth;
  static UINT sceneHeight = m_WindowHeight;

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

    if (
      m_pRenderer != nullptr &&
      m_pScene    != nullptr &&
      m_pEngineUI != nullptr
    ) {
      m_pEngineUI->UpdateLayout();
      
      m_pScene->UpdateScene(0.10F, 0.05F);
      m_pRenderer->BeginFrame();
      m_pScene->RenderScene(*m_pRenderer);

      m_pRenderer->BeginUI();
      
      m_pEngineUI->NewFrame();
      m_pEngineUI->Draw();
      
      ImGui::Render();
      m_pRenderer->RenderUI();

      m_pRenderer->EndFrame();
    }
  }

  AppDestroy();
}

void Kernel::AppDestroy() {
  if (m_pRenderer != nullptr) {
    m_pRenderer->DestroyUI();
    
    m_pEngineUI.reset();
    
    m_pRenderer->OnDestroy();
    m_pRenderer.reset();
    m_pRendererInstance = nullptr;
  }

  m_pWindow.reset();
}