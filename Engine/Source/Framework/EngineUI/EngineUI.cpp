#include <Framework/EngineUI/EngineUI.hpp>
#include <Framework/Scene/Scene.hpp>
#include <Renderer/D3D12Engine/Backend/DirectX12Graphics/DirectX12Graphics.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>

ImGuiWindowFlags WindowFlags = {
  ImGuiWindowFlags_NoMove     |
  ImGuiWindowFlags_NoCollapse |
  ImGuiWindowFlags_NoNav      |
  ImGuiWindowFlags_NoResize
};

ImGuiDockNodeFlags dockFlags = {
  ImGuiDockNodeFlags_AutoHideTabBar      |
  ImGuiDockNodeFlags_PassthruCentralNode |
  ImGuiDockNodeFlags_NoWindowMenuButton  |
  ImGuiDockNodeFlags_NoCloseButton       |
  ImGuiDockNodeFlags_NoDockingOverMe     |
  ImGuiDockNodeFlags_NoDockingSplit      |
  ImGuiDockNodeFlags_NoTabBar
};

EngineUI::EngineUI(HWND hwnd, D3D12Engine::DirectX12Graphics* pRenderer, Scene* pScene)
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
}

EngineUI::~EngineUI() {
  Shutdown();
}

void EngineUI::Initialize(
  ID3D12Device* pDevice,
  ID3D12CommandQueue* pCommandQueue,
  UINT FramesInFlight,
  DXGI_FORMAT RtvFormat,
  D3D12Engine::DescriptorAllocator& rSrvAllocator
) {
  ImGui_ImplWin32_Init(m_hwnd);

  m_FontAllocation = rSrvAllocator.Allocate();

  ImGui_ImplDX12_InitInfo InitInfo = {};
    InitInfo.Device            = pDevice;
    InitInfo.CommandQueue      = pCommandQueue;
    InitInfo.NumFramesInFlight = FramesInFlight;
    InitInfo.RTVFormat         = RtvFormat;
    InitInfo.SrvDescriptorHeap = rSrvAllocator.GetHeap();
    InitInfo.LegacySingleSrvCpuDescriptor = m_FontAllocation.CPU;
    InitInfo.LegacySingleSrvGpuDescriptor = m_FontAllocation.GPU;
  ImGui_ImplDX12_Init(&InitInfo);
}

void EngineUI::Shutdown() {
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

void EngineUI::BeginUI() {
  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
}

void EngineUI::DrawUI() {
  DrawDockSpace();
  DrawViewportUI();
  DrawBrowserUI();
  DrawPropertiesUI();
}

void EngineUI::EndUI() {
  ImGui::Render();
  
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pRenderer->GetCommandList());
}

void EngineUI::UpdateLayout() {
  if (m_ViewportWidth > 0 && m_ViewportHeight > 0 && m_SizeChanged) {
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

    m_SizeChanged = false;
  }
}

void EngineUI::DrawDockSpace() {
  ImGuiID dockSpaceID = ImGui::GetID("MainDockSpace");
  ImGui::DockSpaceOverViewport(dockSpaceID, ImGui::GetMainViewport(), dockFlags);

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

      ImGuiID dockRightID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Right, 0.3F, nullptr, &dockMainID);
      ImGuiID dockBottomID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Down, 0.3F, nullptr, &dockMainID);

      ImGui::DockBuilderDockWindow("Browser", dockBottomID);
      ImGui::DockBuilderDockWindow("Properties", dockRightID);
      ImGui::DockBuilderDockWindow("Scene Viewport", dockMainID);

      ImGui::DockBuilderFinish(dockSpaceID);
    }
  }
}

void EngineUI::DrawViewportUI() {
  ImGui::Begin("Scene Viewport", nullptr, WindowFlags);
      
  ImVec2 ViewportSize = ImGui::GetContentRegionAvail();

  UINT ViewportWidth  = static_cast<UINT>(ViewportSize.x);
  UINT ViewportHeight = static_cast<UINT>(ViewportSize.y);

  if (ViewportWidth > 0 && ViewportHeight > 0) {
    if (m_ViewportWidth != ViewportWidth || m_ViewportHeight != ViewportHeight) {
      m_ViewportWidth  = ViewportWidth;
      m_ViewportHeight = ViewportHeight;

      m_SizeChanged = true;
    }

    ImTextureID sceneTextureID = 
      static_cast<ImTextureID>(
        m_pRenderer->GetSceneTextureSRV().ptr
      );
    ImGui::Image(sceneTextureID, ViewportSize);
  }
  
  ImGui::End();
}

void EngineUI::DrawBrowserUI() {
  ImGui::Begin("Browser", nullptr, WindowFlags);
      
  if (ImGui::Button("Load")) {
    // Open file manager
  }

  ImGui::End();
}

void EngineUI::DrawPropertiesUI() {
  ImGui::Begin("Properties", nullptr, WindowFlags);
      
  ImGui::Text("Textbox");
  
  ImGui::End();
}