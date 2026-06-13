#pragma once

#include <string>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <imgui_internal.h>

#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>

namespace D3D12Engine {
  class DirectX12Graphics;
}
class Scene;

class EngineUI {
 public:
  EngineUI(HWND hwnd, D3D12Engine::DirectX12Graphics* pRenderer, Scene* pScene);
  ~EngineUI();

  void Initialize(
    ID3D12Device* pDevice,
    ID3D12CommandQueue* pCommandQueue,
    UINT FramesInFlight,
    DXGI_FORMAT RtvFormat,
    D3D12Engine::DescriptorAllocator& rSrvAllocator
  );
  void Shutdown();

  void UpdateLayout();
  
  void BeginUI();
  void DrawUI();
  void EndUI();

 private:
  HWND m_hwnd;
  D3D12Engine::DirectX12Graphics* m_pRenderer;
  Scene* m_pScene;
  
  UINT m_ViewportWidth  = 1;
  UINT m_ViewportHeight = 1;
  bool m_SizeChanged    = false;

  D3D12Engine::DescriptorAllocation m_FontAllocation;

  void DrawDockSpace();
  void DrawViewportUI();
  void DrawBrowserUI();
  void DrawPropertiesUI();
};