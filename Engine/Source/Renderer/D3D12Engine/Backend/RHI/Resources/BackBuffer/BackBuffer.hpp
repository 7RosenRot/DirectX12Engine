#pragma once

#include <Renderer/D3D12Engine/Backend/RHI/Resources/GpuResource/GpuResource.hpp>

namespace D3D12Engine {
  class BackBuffer : public GpuResource {
   public:
    BackBuffer() : m_rtvHandle{0} {}

    void CreateFromSwapChain(ID3D12Device* device, const std::wstring& name, ID3D12Resource* baseResource, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_rtvHandle; }

   private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle;
  };
}