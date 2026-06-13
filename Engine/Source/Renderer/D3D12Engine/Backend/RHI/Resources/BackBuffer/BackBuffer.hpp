#pragma once

#include <Renderer/D3D12Engine/Backend/RHI/Resources/GpuResource/GpuResource.hpp>

namespace D3D12Engine {
  class BackBuffer : public GpuResource {
   public:
    BackBuffer() = default;
    ~BackBuffer() = default;

    void CreateFromSwapChain(
      ID3D12Device* device,
      const std::wstring& name,
      ID3D12Resource* baseResource,
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle
    );

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_RtvAllocation; }

   private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_RtvAllocation = {0};
  };
}