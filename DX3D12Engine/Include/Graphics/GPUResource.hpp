#pragma once

#include <d3d12.h>
#include "d3dx12.h"
#include <wrl/client.h>

namespace D3D12Engine {
  class GPUResource {
   public:
    GPUResource() : m_UsageState(D3D12_RESOURCE_STATE_COMMON) {}
    
    virtual ~GPUResource() { Destroy(); }
    
    virtual void Destroy() { m_Resource.Reset(); }
    
    ID3D12Resource* GetResource() const { return m_Resource.Get(); }
    
    D3D12_RESOURCE_STATES GetUsageState() const { return m_UsageState; }
    void SetUsageState(D3D12_RESOURCE_STATES state) { m_UsageState = state; }

    void GPUResource::CreateFromSwapChain(Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource) {
      Destroy();
      
      m_Resource = swapChainResource;
      m_UsageState = D3D12_RESOURCE_STATE_PRESENT;
    }
  
   protected:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
    D3D12_RESOURCE_STATES m_UsageState;
  };
}