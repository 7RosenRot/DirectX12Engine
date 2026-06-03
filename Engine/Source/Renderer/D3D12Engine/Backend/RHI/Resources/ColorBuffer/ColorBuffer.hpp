#pragma once

#include <string>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/GpuResource/GpuResource.hpp>

namespace D3D12Engine {
  class ColorBuffer : public GpuResource {
   public:
    ColorBuffer() = default;
    ~ColorBuffer() = default;

    void CreateScene(
      ID3D12Device* pDevice,
      const std::wstring& SceneName,
      UINT SceneWidth,
      UINT SceneHeight,
      DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM
    );

    // ↓ Getters ↓
    DXGI_FORMAT GetFormat() {
      return m_Format;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const {
      return m_RtvHandle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const {
      return m_SrvHandle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGpuHandle() const {
      return m_SrvHeap->GetGPUDescriptorHandleForHeapStart();
    }

    ID3D12DescriptorHeap* GetSRVHeap() const {
      return m_SrvHeap.Get();
    }
    // ↑ Getters ↑

   private:
    DXGI_FORMAT m_Format;

    // ↓ Scene RTT ↓
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_RtvHandle{};
    // ↑ Scene RTT ↑

    // ↓ ImGui RTT ↓
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_SrvHandle{};
    // ↑ ImGui RTT ↑
  };
}