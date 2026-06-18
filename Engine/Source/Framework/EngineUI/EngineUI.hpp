#pragma once

#include <string>
#include <memory>

#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <imgui_internal.h>

#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>
#include <Framework/History/CommandHistory.hpp>

namespace D3D12Engine {
  class DirectX12Graphics;
}
class GameObject;
class Scene;
class AssetManager;

class EngineUI {
 public:
  EngineUI(
    HWND hwnd,
    D3D12Engine::DirectX12Graphics* pRenderer,
    Scene* pScene
  );
  ~EngineUI();

  void Initialize(
    ID3D12Device* pDevice,
    ID3D12CommandQueue* pCommandQueue,
    UINT FramesInFlight,
    DXGI_FORMAT RtvFormat,
    D3D12Engine::DescriptorAllocator& rSrvAllocator,
    AssetManager* pAssetManager
  );
  void Shutdown();

  void UpdateLayout();
  
  void BeginUI();
  void DrawUI();
  void EndUI();

  // ↓ Gizmo Query ↓
  bool IsGizmoActive() const {
    return m_GizmoActive;
  }
  // ↑ Gizmo Query ↑

  // ↓ Selection Query ↓
  std::shared_ptr<GameObject> GetSelectedObject() const {
    return m_SelectedObject;
  }
  // ↑ Selection Query ↑

 private:
  HWND m_hwnd;
  std::shared_ptr<GameObject>     m_SelectedObject = nullptr;
  D3D12Engine::DirectX12Graphics* m_pRenderer = nullptr;
  Scene*                          m_pScene = nullptr;
  AssetManager*                   m_pAssetManager = nullptr;
  
  UINT m_ViewportWidth  = 1;
  UINT m_ViewportHeight = 1;
  bool m_SizeChanged    = false;

  D3D12Engine::DescriptorAllocation m_FontAllocation;

  // ↓ Gizmo State ↓
  int    m_GizmoType         = -1;
  bool   m_GizmoActive       = false;
  ImVec2 m_ViewportBoundsMin = { 0.0f, 0.0f };
  ImVec2 m_ViewportBoundsMax = { 0.0f, 0.0f };
  // ↑ Gizmo State ↑

  // ↓ Undo/Redo State ↓
  CommandHistory m_CommandHistory;
  Transform m_GizmoInitialTransform;
  bool m_GizmoWasUsing = false;
  
  Transform m_PropertiesInitialTransform;
  bool m_PropertiesEditing = false;
  // ↑ Undo/Redo State ↑

  std::shared_ptr<D3D12Engine::Texture> m_pIconMove   = nullptr;
  std::shared_ptr<D3D12Engine::Texture> m_pIconRotate = nullptr;
  std::shared_ptr<D3D12Engine::Texture> m_pIconScale  = nullptr;

  void DrawDockSpace();
  void DrawProjectUI();
  void DrawViewportUI();
  void DrawBrowserUI();
  void DrawPropertiesUI();
  void DrawGizmo();
};