#pragma once

#include <dxgi1_6.h>
#include <wrl/client.h>

#include <Include/Graphics/Core/GraphicsCore.hpp>
#include <Include/Graphics/Resources/BackBuffer.hpp>

namespace D3D12Engine {
  class Display {
   public:
    Display() = default;
    ~Display() = default;

    // ↓ SwapChain (Display) interaction ↓
    void Initialize(ID3D12Device* pDevice, IDXGIFactory4* pFactory, ID3D12CommandQueue* pCmdQueue,
      HWND hwnd, UINT WindowWidth, UINT WindowHeight);

    void Present();
    
    void Resize(ID3D12Device* pDevice, UINT WindowWidth, UINT WindowHeight);
    // ↑ SwapChain (Display) interaction ↑

    // ↓ Getters ↓
    UINT GetCurrentFrameIndex() const {
      return m_frameIndex;
    }
    
    BackBuffer& GetCurrentBackBufferIndex() {
      return m_renderTargetsResources[m_frameIndex];
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const {
      return m_renderTargetsResources[m_frameIndex].GetRTV();
    }
    
    DXGI_FORMAT GetDXGIFormat() const {
      return GraphicsCore::BackBufferFormat;
    }
    // ↑ Getters ↑

   private:
    UINT m_frameIndex{0};
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    
    BackBuffer m_renderTargetsResources[GraphicsCore::m_frameCount];
    
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
    UINT m_rtvDescriptorSize{0};
  };
}