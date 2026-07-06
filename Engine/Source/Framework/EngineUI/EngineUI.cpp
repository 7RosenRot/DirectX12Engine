#include <Framework/EngineUI/EngineUI.hpp>
#include <Framework/Scene/Scene.hpp>
#include <Framework/AssetManager/AssetManager.hpp>
#include <Renderer/D3D12Engine/Backend/DirectX12Graphics/DirectX12Graphics.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>
#include <Renderer/D3D12Engine/Model/Model.hpp>
#include <Application/Input/Input.hpp>
#include <windows.h>
#include <commdlg.h>

static std::string OpenFileDialog(HWND hwnd, const char* Filter) {
  OPENFILENAMEA OFN;
  
  CHAR File[260] = {0};
  
  ZeroMemory(&OFN, sizeof(OPENFILENAMEA));

  OFN.lStructSize = sizeof(OPENFILENAMEA);
  OFN.hwndOwner = hwnd;
  OFN.lpstrFile = File;
  OFN.nMaxFile = sizeof(File);
  OFN.lpstrFilter = Filter;
  OFN.nFilterIndex = 1;
  OFN.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

  if (GetOpenFileNameA(&OFN) == TRUE) {
    return std::string(OFN.lpstrFile);
  }

  return std::string();
}

static std::string SaveFileDialog(HWND hwnd, const char* Filter) {
  OPENFILENAMEA OFN;
  CHAR File[260] = {0};
  ZeroMemory(&OFN, sizeof(OPENFILENAMEA));

  OFN.lStructSize = sizeof(OPENFILENAMEA);
  OFN.hwndOwner = hwnd;
  OFN.lpstrFile = File;
  OFN.nMaxFile = sizeof(File);
  OFN.lpstrFilter = Filter;
  OFN.nFilterIndex = 1;
  OFN.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
  OFN.lpstrDefExt = "rose";

  if (GetSaveFileNameA(&OFN) == TRUE) {
    return std::string(OFN.lpstrFile);
  }

  return std::string();
}

static bool RayTriangleIntersect(
  DirectX::FXMVECTOR rayOrigin, DirectX::FXMVECTOR rayDir,
  DirectX::FXMVECTOR V0, DirectX::GXMVECTOR V1, DirectX::CXMVECTOR V2,
  float& outIntersectionDistance
) {
  const float EPSILON = 1e-6f;
  DirectX::XMVECTOR edge1 = DirectX::XMVectorSubtract(V1, V0);
  DirectX::XMVECTOR edge2 = DirectX::XMVectorSubtract(V2, V0);
  DirectX::XMVECTOR h = DirectX::XMVector3Cross(rayDir, edge2);
  DirectX::XMVECTOR aVec = DirectX::XMVector3Dot(edge1, h);
  float a = DirectX::XMVectorGetX(aVec);
  
  if (a > -EPSILON && a < EPSILON) {
    return false;
  }
  
  float f = 1.0f / a;
  DirectX::XMVECTOR s = DirectX::XMVectorSubtract(rayOrigin, V0);
  DirectX::XMVECTOR uVec = DirectX::XMVectorMultiply(DirectX::XMVector3Dot(s, h), DirectX::XMVectorReplicate(f));
  float u = DirectX::XMVectorGetX(uVec);
  
  if (u < 0.0f || u > 1.0f) {
    return false;
  }
  
  DirectX::XMVECTOR q = DirectX::XMVector3Cross(s, edge1);
  DirectX::XMVECTOR vVec = DirectX::XMVectorMultiply(DirectX::XMVector3Dot(rayDir, q), DirectX::XMVectorReplicate(f));
  float v = DirectX::XMVectorGetX(vVec);
  
  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }
  
  DirectX::XMVECTOR tVec = DirectX::XMVectorMultiply(DirectX::XMVector3Dot(edge2, q), DirectX::XMVectorReplicate(f));
  float t = DirectX::XMVectorGetX(tVec);
  
  if (t > EPSILON) {
    outIntersectionDistance = t;
    return true;
  }
  
  return false;
}

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

EngineUI::EngineUI(
  HWND hwnd,
  D3D12Engine::DirectX12Graphics* pRenderer,
  Scene* pScene
) : 
  m_hwnd(hwnd),
  m_pRenderer(pRenderer),
  m_pScene(pScene)
{
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();
  
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  m_IniFilePath = "Engine/Assets/Config/imgui.ini";
  io.IniFilename = m_IniFilePath.c_str();

  io.Fonts->Clear();

  ImFont* pMainFont = io.Fonts->AddFontFromFileTTF("Engine/Assets/Fonts/Inter.ttf", 16.0f);

  if (pMainFont == nullptr) {
    io.Fonts->AddFontDefault(); 
  }

#pragma region ImGui StyleConfig
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
#pragma endregion
}

EngineUI::~EngineUI() {
  Shutdown();
}

DirectX::XMFLOAT3 EngineUI::GetTathetPoint() {
  DirectX::XMFLOAT3 targetPoint = { 0.0f, 0.0f, 0.0f };
  if (!m_pSelectedObject.empty()) {
    for (const auto& pObj : m_pSelectedObject) {
      if (pObj) {
        DirectX::XMFLOAT3 pos = pObj->GetTransform().GetPosition();
        targetPoint.x += pos.x;
        targetPoint.y += pos.y;
        targetPoint.z += pos.z;
      }
    }
    targetPoint.x /= m_pSelectedObject.size();
    targetPoint.y /= m_pSelectedObject.size();
    targetPoint.z /= m_pSelectedObject.size();
  } else if (m_pScene != nullptr) {
    targetPoint = m_pScene->GetActiveCamera().GetOrbitTarget();
  }

  return targetPoint;
}


void EngineUI::Initialize(
  ID3D12Device* pDevice,
  ID3D12CommandQueue* pCommandQueue,
  UINT FramesInFlight,
  DXGI_FORMAT RtvFormat,
  D3D12Engine::DescriptorAllocator& rSrvAllocator,
  AssetManager* pAssetManager
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

  // ↓ Preroload Images ↓
  if (pAssetManager) {
    m_pIconMove   = pAssetManager->LoadTexture("Engine/Assets/Images/Move.png");
    m_pIconRotate = pAssetManager->LoadTexture("Engine/Assets/Images/Rotate.png");
    m_pIconScale  = pAssetManager->LoadTexture("Engine/Assets/Images/Scale.png");
    
    pAssetManager->ExexuteUploads();
  }
  // ↑ Preroload Images ↑
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

  // ↓ If "Free Camera" - hide Cursor ↓
  if (Input::IsMouseLocked()) {
    ImGui::GetIO().MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
  }
  // ↑ If "Free Camera" - hide Cursor ↑

  ImGuizmo::BeginFrame();
}

void EngineUI::DrawUI() {
  ImGuiIO& io = ImGui::GetIO();
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
    m_CommandHistory.Undo();
  }
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
    m_CommandHistory.Redo();
  }

  if (!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
    if (!m_pSelectedObject.empty()) {
      m_CommandHistory.ExecuteCommand(std::make_unique<DeleteCommand>(
        m_pScene, m_pSelectedObject
      ));
      m_pSelectedObject.clear();
    }
  }

  DrawDockSpace();
  DrawProjectUI();
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

      ImGui::DockBuilderDockWindow("Browser",        dockBottomID);
      ImGui::DockBuilderDockWindow("Properties",     dockRightID);
      ImGui::DockBuilderDockWindow("Scene Viewport", dockMainID);

      ImGui::DockBuilderFinish(dockSpaceID);
    }
  }
}

void EngineUI::DrawProjectUI() {
  ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 2.5f);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.5f);

  if (ImGui::BeginMainMenuBar()) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.5f);

    ImGui::SetCursorPos(ImVec2(5.0f, 0.0f));
    
    if (ImGui::Button("Scene")) {
      ImGui::OpenPopup("SceneMenuPopup");
    }

    bool isSceneButtonHovered = ImGui::IsItemHovered(
      ImGuiHoveredFlags_AllowWhenBlockedByPopup
    );

    ImVec2 PopupPosition = ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGui::SetNextWindowPos(PopupPosition);

    if (ImGui::BeginPopup("SceneMenuPopup")) {
      
      ImVec2 BtnSize = ImVec2(75.0f, 0.0f);

      ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

      if (ImGui::Button("Open...", BtnSize)) {
        std::string FilePath = OpenFileDialog(m_hwnd, "Rose Scene (*.rose)\0*.rose\0");
        
        if (!FilePath.empty()) {
          m_pScene->LoadScene(FilePath);
          m_pSelectedObject.clear();
        }

        ImGui::CloseCurrentPopup();
      }

      if (ImGui::Button("Save As...", BtnSize)) {
        std::string FilePath = SaveFileDialog(m_hwnd, "Rose Scene (*.rose)\0*.rose\0");
        
        if (!FilePath.empty()) {
          m_pScene->SaveScene(FilePath);
        }

        ImGui::CloseCurrentPopup();
      }

      ImGui::PopStyleVar();

      bool isPopupHovered = ImGui::IsWindowHovered(
        ImGuiHoveredFlags_AllowWhenBlockedByPopup |
        ImGuiHoveredFlags_AllowWhenBlockedByActiveItem
      );
      
      if (!isPopupHovered && !isSceneButtonHovered) {
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }

    ImGui::EndMainMenuBar();
  }
  
  ImGui::PopStyleVar(2);
}

void EngineUI::DrawViewportUI() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Scene Viewport", nullptr, WindowFlags);
  ImGui::PopStyleVar();

  // ↓ Save Viewport Coordinates for ImGuizmo::SetRect ↓
  ImVec2 WindowPosition = ImGui::GetWindowPos();
  ImVec2 ContentMin     = ImGui::GetWindowContentRegionMin();
  ImVec2 ContentMax     = ImGui::GetWindowContentRegionMax();

  m_ViewportBoundsMin = { WindowPosition.x + ContentMin.x, WindowPosition.y + ContentMin.y };
  m_ViewportBoundsMax = { WindowPosition.x + ContentMax.x, WindowPosition.y + ContentMax.y };
  // ↑ Save Viewport Coordinates for ImGuizmo::SetRect ↑

  ImVec2 ViewportSize = ImGui::GetContentRegionAvail();

  UINT ViewportWidth  = static_cast<UINT>(ViewportSize.x);
  UINT ViewportHeight = static_cast<UINT>(ViewportSize.y);

  if (ViewportWidth > 0 && ViewportHeight > 0) {
    if (m_ViewportWidth != ViewportWidth || m_ViewportHeight != ViewportHeight) {
      m_ViewportWidth  = ViewportWidth;
      m_ViewportHeight = ViewportHeight;

      m_SizeChanged = true;
    }

    ImTextureID sceneTextureID = static_cast<ImTextureID>(
      m_pRenderer->GetSceneTextureSRV().ptr
    );
    ImGui::Image(sceneTextureID, ViewportSize);

    // ↓ Raycasting Selection ↓
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver()) {
      ImVec2 mousePos = ImGui::GetMousePos();
      float x = (2.0f * (mousePos.x - m_ViewportBoundsMin.x)) / ViewportWidth - 1.0f;
      float y = 1.0f - (2.0f * (mousePos.y - m_ViewportBoundsMin.y)) / ViewportHeight;

      DirectX::XMMATRIX projMatrix = m_pScene->GetActiveCamera().GetMatrixProjection();
      DirectX::XMMATRIX viewMatrix = m_pScene->GetActiveCamera().GetTransform().GetMatrixView();
      DirectX::XMMATRIX viewProjInv = DirectX::XMMatrixInverse(nullptr, viewMatrix * projMatrix);

      DirectX::XMVECTOR rayOrigin = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(x, y, 0.0f, 1.0f), viewProjInv);
      DirectX::XMVECTOR rayEnd = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(x, y, 1.0f, 1.0f), viewProjInv);
      DirectX::XMVECTOR rayDir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(rayEnd, rayOrigin));

      std::shared_ptr<GameObject> closestObj = nullptr;
      float minWorldHitDist = FLT_MAX;

      for (auto& [name, pObj] : m_pScene->GetGameObjects()) {
        auto pModel = pObj->GetModel();
        if (!pModel) continue;

        const auto& vertices = pModel->GetVertices();
        const auto& indices = pModel->GetIndices();
        if (vertices.empty() || indices.empty()) continue;

        DirectX::XMMATRIX modelMatrix = pObj->GetTransform().GetMatrixModel();
        DirectX::XMMATRIX invModelMatrix = DirectX::XMMatrixInverse(nullptr, modelMatrix);

        DirectX::XMVECTOR localRayOrigin = DirectX::XMVector3TransformCoord(rayOrigin, invModelMatrix);
        DirectX::XMVECTOR localRayEnd = DirectX::XMVector3TransformCoord(rayEnd, invModelMatrix);
        DirectX::XMVECTOR localRayDir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(localRayEnd, localRayOrigin));

        float closestLocalDist = FLT_MAX;
        bool hitFound = false;

        size_t indexCount = indices.size();
        for (size_t i = 0; i < indexCount; i += 3) {
          if (i + 2 >= indexCount) break;

          UINT idx0 = indices[i];
          UINT idx1 = indices[i + 1];
          UINT idx2 = indices[i + 2];

          if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size()) continue;

          DirectX::XMVECTOR V0 = DirectX::XMLoadFloat3(&vertices[idx0].Position);
          DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&vertices[idx1].Position);
          DirectX::XMVECTOR V2 = DirectX::XMLoadFloat3(&vertices[idx2].Position);

          float t = 0.0f;
          if (RayTriangleIntersect(localRayOrigin, localRayDir, V0, V1, V2, t)) {
            if (t < closestLocalDist) {
              closestLocalDist = t;
              hitFound = true;
            }
          }
        }

        if (hitFound) {
          DirectX::XMVECTOR localHitPos = DirectX::XMVectorMultiplyAdd(localRayDir, DirectX::XMVectorReplicate(closestLocalDist), localRayOrigin);
          DirectX::XMVECTOR worldHitPos = DirectX::XMVector3TransformCoord(localHitPos, modelMatrix);
          DirectX::XMVECTOR distVec = DirectX::XMVector3Length(DirectX::XMVectorSubtract(worldHitPos, rayOrigin));
          float worldDistance = DirectX::XMVectorGetX(distVec);

          if (worldDistance < minWorldHitDist) {
            minWorldHitDist = worldDistance;
            closestObj = pObj;
          }
        }
      }

      if (ImGui::GetIO().KeyCtrl) {
        if (closestObj) {
          auto it = std::find(m_pSelectedObject.begin(), m_pSelectedObject.end(), closestObj);
          if (it != m_pSelectedObject.end()) {
            m_pSelectedObject.erase(it);
          } else {
            m_pSelectedObject.push_back(closestObj);
          }
        }
      } else {
        m_pSelectedObject.clear();
        if (closestObj) {
          m_pSelectedObject.push_back(closestObj);
        }
      }
    }
    // ↑ Raycasting Selection ↑

    // ↓ ImGuizmo Toolbar (Move / Rotate / Scale) ↓
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,     0.0f );
    
    ImVec2 ToolbarPos = {
      m_ViewportBoundsMin.x + 5.0f,
      m_ViewportBoundsMin.y + 10.0f
    };
    ImGui::SetNextWindowPos(ToolbarPos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGuiWindowFlags ToolbarFlags =
      ImGuiWindowFlags_NoDecoration      |
      ImGuiWindowFlags_NoMove            |
      ImGuiWindowFlags_NoSavedSettings   |
      ImGuiWindowFlags_NoDocking         |
      ImGuiWindowFlags_NoNav             |
      ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::Begin("##GizmoToolbar", nullptr, ToolbarFlags);

    auto DrawToolButton = [&](
      const char* ID,
      ImTextureID TextureIcon,
      int         GizmoMode,
      int         PositionType
    ) {
      ImVec2 BtnSize = ImVec2(40.0f, 40.0f);

      ImVec2 Point_TopLeft     = ImGui::GetCursorScreenPos();
      ImVec2 Point_BottomRight = ImVec2(Point_TopLeft.x + BtnSize.x, Point_TopLeft.y + BtnSize.y);
      ImDrawList* pDrawList    = ImGui::GetWindowDrawList();

      ImGui::InvisibleButton(ID, BtnSize);

      bool IsHovered  = ImGui::IsItemHovered();
      bool IsSelected = (m_GizmoType == GizmoMode);

      if (ImGui::IsItemClicked()) {
        m_GizmoType = (m_GizmoType == GizmoMode) ? -1 : GizmoMode;
      }

      ImU32 bgColor;
      if (IsSelected)     { bgColor = IM_COL32(76, 127, 230, 200); }
      else if (IsHovered) { bgColor = IM_COL32(100, 150, 255, 100); }
      else                { bgColor = IM_COL32(40, 40, 40, 180); }

      ImDrawFlags Corners = ImDrawFlags_RoundCornersNone;
      float Rounding = 0.0f;

      if (PositionType == 0) {
        Corners = ImDrawFlags_RoundCornersTop;
        Rounding = 5.0f;
      } else if (PositionType == 2) {
        Corners = ImDrawFlags_RoundCornersBottom;
        Rounding = 5.0f;
      }

      pDrawList->AddRectFilled(Point_TopLeft, Point_BottomRight, bgColor, Rounding, Corners);

      float IconPadding = 5.0f;
      ImVec2 Image_TopLeft     = ImVec2(Point_TopLeft.x + IconPadding, Point_TopLeft.y + IconPadding);
      ImVec2 Image_BottomRight = ImVec2(Point_BottomRight.x - IconPadding, Point_BottomRight.y - IconPadding);

      pDrawList->AddImage(TextureIcon, Image_TopLeft, Image_BottomRight);
    };

    ImTextureID IconMove_Texture = static_cast<ImTextureID>(m_pIconMove->GetSrvHandle().ptr);
    DrawToolButton("##MoveBtn", IconMove_Texture, ImGuizmo::OPERATION::TRANSLATE, 0);
    
    ImTextureID IconRotate_Texture = static_cast<ImTextureID>(m_pIconRotate->GetSrvHandle().ptr);
    DrawToolButton("##RotateBtn", IconRotate_Texture, ImGuizmo::OPERATION::ROTATE, 1);
    
    ImTextureID IconScale_Texture = static_cast<ImTextureID>(m_pIconScale->GetSrvHandle().ptr);
    DrawToolButton("##ScaleBtn", IconScale_Texture,  ImGuizmo::OPERATION::SCALE, 2);

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      if (!ImGuizmo::IsOver() && !ImGui::IsAnyItemHovered()) {
        m_GizmoType = -1;
      }
    }

    ImGui::End();

    ImGui::PopStyleVar(3);
    // ↑ ImGuizmo Toolbar (Move / Rotate / Scale) ↑

    DrawGizmo();
  }

  ImGui::End();
}

void EngineUI::DrawBrowserUI() {
  ImGui::Begin("Browser", nullptr, WindowFlags);

  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

  if (
    ImGui::Button("Import", ImVec2(60.0f, 25.0f))
  ) {
    std::string FilePath = OpenFileDialog(m_hwnd, "Model files... (*.obj)\0*.obj\0");

    if (!FilePath.empty()) {
      m_pScene->AddGameObject(FilePath);
    }
  }

  ImGui::PopStyleVar();

  if (
    ImGui::IsWindowHovered() &&
    ImGui::IsMouseClicked(0) &&
    !ImGui::IsAnyItemHovered()
  ) {
    m_pSelectedObject.clear();
  }

  // ↓ Object Icon ↓
  float Padding = 16.0f;
  float Thumbnail = 64.0f;
  float Cell = Thumbnail + Padding;
  float PanelWidth = ImGui::GetContentRegionAvail().x;
  int ColumnCount = max(1, static_cast<int>(PanelWidth / Cell));
  // ↑ Object Icon ↑

  ImGui::Columns(ColumnCount, 0, false);

  ImGui::Spacing();

  auto& SceneObjects = m_pScene->GetGameObjects();

  for (auto& [ObjectName, pGameObject] : SceneObjects) {
    ImGui::PushID(pGameObject.get());

    bool isSelected = std::find(m_pSelectedObject.begin(), m_pSelectedObject.end(), pGameObject) != m_pSelectedObject.end();
    if (isSelected) {
      ImGui::PushStyleColor(
        ImGuiCol_Button,
        ImVec4(0.3f, 0.4f, 0.8f, 1.0f)
      );
    } else {
      ImGui::PushStyleColor(
        ImGuiCol_Button,
        ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
      );
    }

    if (
      ImGui::ImageButton(
        pGameObject->GetObjectName().c_str(),
        (ImTextureID)pGameObject->GetTexture()->GetSrvHandle().ptr,
        ImVec2(Thumbnail, Thumbnail)
      )
    ) {
      if (ImGui::GetIO().KeyCtrl) {
        auto it = std::find(m_pSelectedObject.begin(), m_pSelectedObject.end(), pGameObject);
        if (it != m_pSelectedObject.end()) {
          m_pSelectedObject.erase(it);
        } else {
          m_pSelectedObject.push_back(pGameObject);
        }
      } else {
        m_pSelectedObject.clear();
        m_pSelectedObject.push_back(pGameObject);
      }
    }
    
    ImGui::PopStyleColor();

    ImGui::TextWrapped("%s", pGameObject->GetObjectName().c_str());

    ImGui::NextColumn();
    ImGui::PopID();
  }

  ImGui::Columns(1);
  ImGui::End();
}

void EngineUI::DrawPropertiesUI() {
  ImGui::Begin("Properties", nullptr, WindowFlags);

  if (m_pSelectedObject.size() == 1) {
    auto& pSelected = m_pSelectedObject[0];
    char NameBuffer[256];
    strcpy_s(NameBuffer, pSelected->GetObjectName().c_str());

    if (ImGui::InputText("Name", NameBuffer, sizeof(NameBuffer))) {
      pSelected->GetObjectName() = NameBuffer;
    }

    ImGui::Separator();
    ImGui::Text("Transform");
    ImGui::Spacing();

    auto& rTransform = pSelected->GetTransform();

    // ↓ Position ↓
    DirectX::XMFLOAT3 Position = rTransform.GetPosition();
    float PositionArr[3] = { Position.x, Position.y, Position.z };

    if (ImGui::DragFloat3("Position", PositionArr, 0.05f)) {
      if (!m_PropertiesEditing) { m_PropertiesInitialTransform = rTransform; m_PropertiesEditing = true; }
      rTransform.SetPosition(PositionArr[0], PositionArr[1], PositionArr[2]);
    }
    // ↑ Position ↑

    // ↓ Rotation (градусы в UI, радианы внутри) ↓
    DirectX::XMFLOAT3 RotationRad = rTransform.GetRotation();
    float RotationDeg[3] = {
      DirectX::XMConvertToDegrees(RotationRad.x),
      DirectX::XMConvertToDegrees(RotationRad.y),
      DirectX::XMConvertToDegrees(RotationRad.z)
    };

    if (ImGui::DragFloat3("Rotation", RotationDeg, 0.5f)) {
      if (!m_PropertiesEditing) { m_PropertiesInitialTransform = rTransform; m_PropertiesEditing = true; }
      rTransform.SetRotation(
        RotationDeg[0],
        RotationDeg[1],
        RotationDeg[2]
      );
    }
    // ↑ Rotation ↑

    // ↓ Scale ↓
    DirectX::XMFLOAT3 Scale = rTransform.GetScale();
    float ScaleArr[3] = { Scale.x, Scale.y, Scale.z };

    if (ImGui::DragFloat3("Scale", ScaleArr, 0.01f, 0.001f, 100.0f)) {
      if (!m_PropertiesEditing) { m_PropertiesInitialTransform = rTransform; m_PropertiesEditing = true; }
      rTransform.SetScale(ScaleArr[0], ScaleArr[1], ScaleArr[2]);
    }
    // ↑ Scale ↑

    if (m_PropertiesEditing && !ImGui::IsAnyItemActive()) {
      m_CommandHistory.ExecuteCommand(std::make_unique<TransformCommand>(
        pSelected, m_PropertiesInitialTransform, rTransform
      ));
      m_PropertiesEditing = false;
    }

    ImGui::Separator();
    ImGui::Text("Material");

    if (ImGui::Button("Load Texture", ImVec2(-1, 0))) {
      std::string FilePath = OpenFileDialog(m_hwnd, "Image Files...\0*.png\0*.jpg\0");

      if (!FilePath.empty()) {
        pSelected->LoadTexture(FilePath);
      }
    }
  } else if (m_pSelectedObject.size() > 1) {
    ImGui::Text("Selected: %d objects", static_cast<int>(m_pSelectedObject.size()));
  } else {
    ImGui::Text("Select an object...");
  }

  ImGui::End();
}

void EngineUI::DrawGizmo() {
  if (m_pSelectedObject.empty() || m_GizmoType == -1) {
    m_GizmoActive = false;
    
    return;
  }

  ImGuizmo::SetOrthographic(false);
  ImGuizmo::SetDrawlist();

  // Передаём экранные координаты вьюпорта, сохранённые в DrawViewportUI
  float ViewportWidth  = m_ViewportBoundsMax.x - m_ViewportBoundsMin.x;
  float ViewportHeight = m_ViewportBoundsMax.y - m_ViewportBoundsMin.y;
  ImGuizmo::SetRect(m_ViewportBoundsMin.x, m_ViewportBoundsMin.y, ViewportWidth, ViewportHeight);

  if (ViewportHeight > 0.0f) {
    ImGuizmo::SetGizmoSizeClipSpace(0.1f * (500.0f / ViewportHeight));
  }

  // ↓ Матрицы камеры ↓
  // DirectXMath хранит матрицы row-major. ImGuizmo читает float[16] column-major.
  // Свойство: DX row-major и GLM column-major одних и тех же байт — это математически
  // транспонированные матрицы, что в точности совпадает с нужным преобразованием.
  // Дополнительный XMMatrixTranspose не нужен и ломает порядок элементов.
  DirectX::XMMATRIX ViewMatrix       = m_pScene->GetActiveCamera().GetTransform().GetMatrixView();
  DirectX::XMMATRIX ProjectionMatrix = m_pScene->GetActiveCamera().GetMatrixProjection();
  // ↑ Матрицы камеры ↑

  // Calculate common center (average position)
  DirectX::XMFLOAT3 avgPos = { 0.0f, 0.0f, 0.0f };
  for (const auto& pObj : m_pSelectedObject) {
    DirectX::XMFLOAT3 pos = pObj->GetTransform().GetPosition();
    avgPos.x += pos.x;
    avgPos.y += pos.y;
    avgPos.z += pos.z;
  }
  avgPos.x /= m_pSelectedObject.size();
  avgPos.y /= m_pSelectedObject.size();
  avgPos.z /= m_pSelectedObject.size();

  // Create group matrix at the average position with no rotation and identity scale
  DirectX::XMMATRIX GroupMatrix = DirectX::XMMatrixTranslation(avgPos.x, avgPos.y, avgPos.z);

  // If we were using the gizmo, keep the current group matrix
  if (m_GizmoWasUsing) {
    GroupMatrix = DirectX::XMLoadFloat4x4(&m_GizmoCurrentGroupMatrix);
  }

  // Загружаем в массивы float[16] без дополнительных преобразований
  DirectX::XMFLOAT4X4 ViewFloat, ProjectionFloat, GroupFloat;
  DirectX::XMStoreFloat4x4(&ViewFloat,       ViewMatrix);
  DirectX::XMStoreFloat4x4(&ProjectionFloat, ProjectionMatrix);
  DirectX::XMStoreFloat4x4(&GroupFloat,      GroupMatrix);

  ImGuizmo::AllowAxisFlip(false);
  ImGuizmo::Manipulate(
    reinterpret_cast<const float*>(&ViewFloat),
    reinterpret_cast<const float*>(&ProjectionFloat),
    static_cast<ImGuizmo::OPERATION>(m_GizmoType),
    ImGuizmo::WORLD,
    reinterpret_cast<float*>(&GroupFloat)
  );

  if (ImGuizmo::IsUsing()) {
    if (!m_GizmoWasUsing) {
      // Record initial transforms
      m_GizmoInitialTransforms.clear();
      for (auto& pObj : m_pSelectedObject) {
        m_GizmoInitialTransforms.push_back(pObj->GetTransform());
      }
      // Record initial and current group matrix
      DirectX::XMStoreFloat4x4(&m_GizmoInitialGroupMatrix, GroupMatrix);
      DirectX::XMStoreFloat4x4(&m_GizmoCurrentGroupMatrix, GroupMatrix);
      m_GizmoWasUsing = true;
    }
    m_GizmoActive = true;

    // Calculate delta matrix: Delta = InvInitialGroup * ResultGroup
    DirectX::XMMATRIX InitialGroupMatrix = DirectX::XMLoadFloat4x4(&m_GizmoInitialGroupMatrix);
    DirectX::XMMATRIX InvInitialGroup = DirectX::XMMatrixInverse(nullptr, InitialGroupMatrix);
    DirectX::XMMATRIX ResultGroupMatrix = DirectX::XMLoadFloat4x4(&GroupFloat);
    
    // Store the updated current group matrix
    DirectX::XMStoreFloat4x4(&m_GizmoCurrentGroupMatrix, ResultGroupMatrix);

    DirectX::XMMATRIX DeltaMatrix = InvInitialGroup * ResultGroupMatrix;

    // Apply delta matrix to all selected objects
    for (size_t i = 0; i < m_pSelectedObject.size(); ++i) {
      auto& pObj = m_pSelectedObject[i];
      auto& rTransform = pObj->GetTransform();
      
      DirectX::XMMATRIX InitialModelMatrix = m_GizmoInitialTransforms[i].GetMatrixModel();
      DirectX::XMMATRIX NewModelMatrix = InitialModelMatrix * DeltaMatrix;
      
      DirectX::XMVECTOR ScaleVector, RotationQuaternion, TranslationVector;
      DirectX::XMMatrixDecompose(&ScaleVector, &RotationQuaternion, &TranslationVector, NewModelMatrix);

      DirectX::XMFLOAT3 NewPosition, NewScale;
      DirectX::XMStoreFloat3(&NewPosition, TranslationVector);
      DirectX::XMStoreFloat3(&NewScale,    ScaleVector);

      DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(RotationQuaternion);
      DirectX::XMFLOAT4X4 RotFloat;
      DirectX::XMStoreFloat4x4(&RotFloat, RotationMatrix);

      float val = -RotFloat._32;
      if (val < -1.0f) val = -1.0f;
      else if (val > 1.0f) val = 1.0f;
      float Pitch = asinf(val);
      float Yaw   = atan2f(RotFloat._31, RotFloat._33);
      float Roll  = atan2f(RotFloat._12, RotFloat._22);

      rTransform.SetPosition(NewPosition.x, NewPosition.y, NewPosition.z);
      rTransform.SetRotation(
        DirectX::XMConvertToDegrees(Pitch),
        DirectX::XMConvertToDegrees(Yaw),
        DirectX::XMConvertToDegrees(Roll)
      );
      rTransform.SetScale(NewScale.x, NewScale.y, NewScale.z);
    }
  } else {
    if (m_GizmoWasUsing) {
      // Record new transforms and create multi-transform command
      std::vector<Transform> NewTransforms;
      for (auto& pObj : m_pSelectedObject) {
        NewTransforms.push_back(pObj->GetTransform());
      }
      
      m_CommandHistory.ExecuteCommand(std::make_unique<MultiTransformCommand>(
        m_pSelectedObject, m_GizmoInitialTransforms, NewTransforms
      ));
      m_GizmoWasUsing = false;
    }
    m_GizmoActive = false;
  }
}
