#include <Framework/EngineUI/EngineUI.hpp>
#include <Framework/Scene/Scene.hpp>
#include <Renderer/IRenderer/IRenderer.hpp>

EngineUI::EngineUI(HWND hwnd, IRenderer* pRenderer, Scene* pScene)
 : m_hwnd(hwnd), m_pRenderer(pRenderer), m_pScene(pScene)
{
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();
  
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // ↓ Style Config ↓
  ImGuiStyle& Style = ImGui::GetStyle();
  Style.WindowRounding    = 8.0f;
  Style.PopupRounding     = 6.0f;
  Style.ChildRounding     = 6.0f;
  
  Style.FrameRounding     = 6.0f;
  Style.GrabRounding      = 6.0f;
  
  Style.ScrollbarRounding = 6.0f;
  Style.TabRounding       = 6.0f;

  Style.FramePadding      = ImVec2(6.0F, 4.0F); 
  Style.ItemSpacing       = ImVec2(8.0F, 6.0F);

  ImVec4* Colors = Style.Colors;
  Colors[ImGuiCol_WindowBg]           = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
  Colors[ImGuiCol_ChildBg]            = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
  Colors[ImGuiCol_PopupBg]            = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  Colors[ImGuiCol_Border]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  Colors[ImGuiCol_FrameBg]            = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
  Colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  Colors[ImGuiCol_FrameBgActive]      = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

  Colors[ImGuiCol_TitleBg]            = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
  Colors[ImGuiCol_TitleBgActive]      = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
  Colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

  Colors[ImGuiCol_Tab]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
  Colors[ImGuiCol_TabHovered]         = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
  Colors[ImGuiCol_TabActive]          = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
  Colors[ImGuiCol_TabUnfocused]       = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
  Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);

  Colors[ImGuiCol_DockingPreview]     = ImVec4(0.30f, 0.30f, 0.30f, 0.70f);
  Colors[ImGuiCol_Header]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  Colors[ImGuiCol_Button]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  Colors[ImGuiCol_ButtonHovered]      = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
  Colors[ImGuiCol_ButtonActive]       = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
  // ↑ Style Config ↑

  ImGui_ImplWin32_Init(m_hwnd);
  
  m_pRenderer->InitUI();
}

EngineUI::~EngineUI() {
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

void EngineUI::NewFrame() {
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
}

void EngineUI::UpdateLayout() {
  if (m_ViewportWidth > 1 && m_ViewportHeight > 1) {
    m_SizeChanged = false;
    
    m_pRenderer->ResizeViewport(
      m_ViewportWidth,
      m_ViewportHeight
    );

    float aspectRatio = static_cast<float>(m_ViewportWidth) / static_cast<float>(m_ViewportHeight);
    m_pScene->GetActiveCamera().SetLensProperties(
      0.25F * DirectX::XM_PI,
      aspectRatio,
      1.0F,
      1000.0F
    );
  }
}

void EngineUI::Draw() {
  DrawDockSpace();
  
  DrawViewportUI();
  
  DrawBrowserUI();
  
  DrawPropertiesUI();
}

void EngineUI::DrawDockSpace() {
  ImGuiID dockSpaceID = ImGui::GetID("MainDockSpace");
  ImGui::DockSpaceOverViewport(dockSpaceID, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

  static bool firstRun = true;
  if (firstRun) {
    firstRun = false;

    if (
      ImGui::DockBuilderGetNode(dockSpaceID) == nullptr ||
      ImGui::DockBuilderGetNode(dockSpaceID)->IsEmpty()
    ) {
      ImGui::DockBuilderRemoveNode(dockSpaceID);
      ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_DockSpace);
      ImGui::DockBuilderSetNodeSize(dockSpaceID, ImGui::GetMainViewport()->Size);

      ImGuiID dockMainID = dockSpaceID;

      ImGuiID dockRighID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Right, 0.3F, nullptr, &dockMainID);
      ImGuiID dockBottomID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Down, 0.3F, nullptr, &dockMainID);

      ImGui::DockBuilderDockWindow("Browser", dockBottomID);
      ImGui::DockBuilderDockWindow("Properties", dockRighID);
      ImGui::DockBuilderDockWindow("Scene Viewport", dockMainID);

      ImGui::DockBuilderFinish(dockSpaceID);
    }
  }
}

void EngineUI::DrawViewportUI() {
  ImGui::Begin("Scene Viewport", nullptr, m_WindowFlags);
      
  ImVec2 sceneSize = ImGui::GetContentRegionAvail();

  if (sceneSize.x > 1.0F && sceneSize.y > 1.0F) {
    if (
      m_ViewportWidth  != static_cast<UINT>(sceneSize.x) ||
      m_ViewportHeight != static_cast<UINT>(sceneSize.y)
    ) {
      m_SizeChanged = true;
      
      m_ViewportWidth  = static_cast<UINT>(sceneSize.x);
      m_ViewportHeight = static_cast<UINT>(sceneSize.y);
    }

    ImGui::Image((ImTextureID)(intptr_t)m_pRenderer->GetSceneTextureSRV().ptr, sceneSize);
  }
  
  ImGui::End();
}

void EngineUI::DrawBrowserUI() {
  ImGui::Begin("Browser", nullptr, m_WindowFlags);
      
  if (ImGui::Button("Load")) {
    // Open file manager
  }

  ImGui::End();
}

void EngineUI::DrawPropertiesUI() {
  ImGui::Begin("Properties", nullptr, m_WindowFlags);
      
  ImGui::Text("Textbox");
  
  ImGui::End();
}