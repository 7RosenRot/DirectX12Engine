#pragma once

#include <Renderer/D3D12Engine/Backend/RHI/Resources/GpuResource/GpuResource.hpp>

namespace D3D12Engine {    
  class GpuBuffer : public GpuResource {
   public:
    virtual ~GpuBuffer() {}
    
    void Create(ID3D12Device* pDevice, const std::wstring& name,
      UINT sizeInBytes, const void* initialData = nullptr);
    
    void CreateUploadable(ID3D12Device* device, const std::wstring& name, UINT sizeInBytes);
    
    void UpdateUploadData(const void* data, UINT sizeInBytes);

   protected:
    UINT m_BufferSize{0};
  };
  
  class GpuConstantBuffer : public GpuBuffer {
   public:
    void CreateUploadable(ID3D12Device* device, const std::wstring& name, UINT sizeInBytes);
    
    D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const {
      return m_pResource->GetGPUVirtualAddress();
    }
  };
}