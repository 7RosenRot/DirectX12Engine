#pragma once

#include <Include/Graphics/GPUResource.hpp>

namespace D3D12Engine {    
  class GPUBuffer : public GPUResource {
   public:
    virtual ~GPUBuffer() {}
    
    void Create(ID3D12Device* device, const std::wstring& name, UINT sizeInBytes, const void* initialData = nullptr);
    
    void CreateUploadable(ID3D12Device* device, const std::wstring& name, UINT sizeInBytes);
    
    void UpdateUploadData(const void* data, UINT sizeInBytes);

   protected:
    UINT m_BufferSize{ 0 };
  };
  
  class GpuConstantBuffer : public GPUBuffer {
   public:
    void CreateUploadable(ID3D12Device* device, const std::wstring& name, UINT sizeInBytes);
    
    D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_Resource->GetGPUVirtualAddress(); }
  };
}