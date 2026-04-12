#pragma once

#include <string>

#include <Include/Graphics/Resources/GpuResource.hpp>

namespace D3D12Engine {
  class DepthBuffer : public GpuResource {
   public:
    DepthBuffer(float clearDepth = 1.0F, UINT8 clearStencil = 0)
    : m_Format(DXGI_FORMAT_UNKNOWN), m_ClearDepth(clearDepth), m_ClearStencil(clearStencil) {}
    
    ~DepthBuffer() = default;

    void Create(ID3D12Device* device, const std::wstring& name, UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT);

    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_DSVHandle; }
    DXGI_FORMAT GetFormat() const { return m_Format; }

    float GetClearDepth() const { return m_ClearDepth; }
    UINT8 GetClearStencil() const { return m_ClearStencil; }

   private:
    DXGI_FORMAT m_Format;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_DSVHandle{};

    float m_ClearDepth;
    UINT8 m_ClearStencil;
  };
}