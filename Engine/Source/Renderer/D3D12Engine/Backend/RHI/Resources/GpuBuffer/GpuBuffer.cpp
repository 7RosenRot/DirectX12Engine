#include <exception>
#include <stdexcept>

#include <Renderer/D3D12Engine/Backend/RHI/Resources/GpuBuffer/GpuBuffer.hpp>

void D3D12Engine::GpuBuffer::Create(
  ID3D12Device* pDevice,
  const std::wstring& name,
  UINT sizeInBytes,
  const void* initialData
) {
  Shutdown();
  
  m_BufferSize = sizeInBytes;
  
  pDevice->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize),
    D3D12_RESOURCE_STATE_COMMON,
    nullptr,
    IID_PPV_ARGS(&m_pResource)
  );
  
  m_pResource->SetName(name.c_str());
  m_UsageState = D3D12_RESOURCE_STATE_COMMON;
  
  if (initialData) {
    //  copying by CommandContext
  }
}

void D3D12Engine::GpuBuffer::CreateUploadable(
  ID3D12Device* device,
  const std::wstring& name,
  UINT sizeInBytes
) {
  Shutdown();
  m_BufferSize = sizeInBytes;

  device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&m_pResource)
  );
  
  m_pResource->SetName(name.c_str());
  m_UsageState = D3D12_RESOURCE_STATE_GENERIC_READ;
}

void D3D12Engine::GpuBuffer::UpdateUploadData(const void* data, UINT sizeInBytes) {
  if (m_UsageState != D3D12_RESOURCE_STATE_GENERIC_READ) {
    throw std::runtime_error("Cannot update non-uploadable buffer!");
  }
  
  UINT8* pCBVDataBegin;
  CD3DX12_RANGE readRange(0, 0);
  
  m_pResource->Map(0, &readRange, reinterpret_cast<void**>(&pCBVDataBegin));
  memcpy(pCBVDataBegin, data, sizeInBytes);
  m_pResource->Unmap(0, nullptr);
}

void D3D12Engine::GpuConstantBuffer::CreateUploadable(ID3D12Device* device, const std::wstring& name, UINT sizeInBytes) {
  UINT alignment = 256;
  UINT alignedSize = (sizeInBytes + alignment - 1) & ~(alignment - 1);
  
  GpuBuffer::CreateUploadable(device, name, alignedSize);
}