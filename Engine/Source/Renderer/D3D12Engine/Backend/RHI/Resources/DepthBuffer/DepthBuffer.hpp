#pragma once

#include <string>

#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/GpuResource/GpuResource.hpp>

namespace D3D12Engine {
  class DepthBuffer : public GpuResource {
   public:
    DepthBuffer(float clearDepth = 1.0F, UINT8 clearStencil = 0);
    
    ~DepthBuffer() = default;

    void Create(
      ID3D12Device* device,
      const std::wstring& name,
      UINT width,
      UINT height,
      DescriptorAllocator& DsvAllocator,
      DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT
    );

    void Shutdown(DescriptorAllocator& DsvAllocator);

    DXGI_FORMAT GetFormat() const { return m_Format; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_DsvAllocation.CPU; }

    float GetClearDepth() const { return m_ClearDepth; }
    UINT8 GetClearStencil() const { return m_ClearStencil; }

   private:
    DXGI_FORMAT m_Format;
    DescriptorAllocation m_DsvAllocation;

    float m_ClearDepth;
    UINT8 m_ClearStencil;
  };
}