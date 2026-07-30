#pragma once

#include <string>
#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/GpuResource/GpuResource.hpp>

namespace D3D12Engine {
  class ColorBuffer : public GpuResource {
   public:
    ColorBuffer() = default;
    ~ColorBuffer() = default;

    bool Initialize(
      ID3D12Device* pDevice,
      const std::wstring& SceneName,
      UINT SceneWidth,
      UINT SceneHeight,
      DescriptorAllocator& RtvAllocator,
      DescriptorAllocator& SrvAllocator,
      DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM
    );

    void Shutdown(
      DescriptorAllocator& RtvAllocator,
      DescriptorAllocator& SrvAllocator
    );

    // ↓ Getters ↓
    DXGI_FORMAT GetFormat()                       const { return m_Format; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCpuHandle() const { return m_RtvAllocation.CPU; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCpuHandle() const { return m_SrvAllocation.CPU; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGpuHandle() const { return m_SrvAllocation.GPU; }
    // ↑ Getters ↑

   private:
    DXGI_FORMAT m_Format;

    // ↓ Scene RTT ↓
    DescriptorAllocation m_RtvAllocation;
    // ↑ Scene RTT ↑

    // ↓ ImGui RTT ↓
    DescriptorAllocation m_SrvAllocation;
    // ↑ ImGui RTT ↑
  };
}