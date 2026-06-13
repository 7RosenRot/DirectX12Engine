#pragma once

#include <dxgi1_6.h>
#include <wrl/client.h>

#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Core/GraphicsCore/GraphicsCore.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/BackBuffer/BackBuffer.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/DepthBuffer/DepthBuffer.hpp>

namespace D3D12Engine {
  class SwapChain {
   public:
    SwapChain() = default;
    ~SwapChain() = default;

    // ↓ SwapChain (SwapChain) interaction ↓
    void Initialize(
      ID3D12Device* pDevice,
      IDXGIFactory4* pFactory,
      ID3D12CommandQueue* pCmdQueue,
      HWND hwnd,
      UINT WindowWidth,
      UINT WindowHeight,
      DescriptorAllocator& RtvAllocator
    );

    void Present();
    
    void Resize(
      ID3D12Device* pDevice,
      DescriptorAllocator& RtvAllocator,
      UINT WindowWidth,
      UINT WindowHeight
    );
    // ↑ SwapChain (SwapChain) interaction ↑

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
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    BackBuffer m_renderTargetsResources[GraphicsCore::m_frameCount];
   
    DescriptorAllocation m_rtvAllocations[GraphicsCore::m_frameCount];
    UINT m_frameIndex = 0;
  };
}