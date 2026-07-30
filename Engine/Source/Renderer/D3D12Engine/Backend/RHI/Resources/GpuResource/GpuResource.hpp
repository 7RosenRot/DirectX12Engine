#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>

namespace D3D12Engine {
  class GpuResource {
   public:
    GpuResource() = default;
    
    virtual ~GpuResource() {
      Shutdown();
    }
    
    virtual void Shutdown() {
      if (m_pResource != nullptr) {
        m_pResource.Reset();
      }
    }
    
    // ↓ Get ptr ↓
    ID3D12Resource* operator->() { return m_pResource.Get(); }
    const ID3D12Resource* operator->() const { return m_pResource.Get(); }
    
    ID3D12Resource* GetResource() { return m_pResource.Get(); }
    const ID3D12Resource* GetResource() const { return m_pResource.Get(); }
    // ↑ Get ptr ↑
    
    // ↓ Set/Get Usage State ↓
    void SetUsageState(D3D12_RESOURCE_STATES usageState) { m_UsageState = usageState; }
    D3D12_RESOURCE_STATES GetUsageState() const { return m_UsageState; }
    // ↑ Set/Get Usage State ↑

   protected:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
    D3D12_RESOURCE_STATES m_UsageState = D3D12_RESOURCE_STATE_COMMON;
  };
}