#include <stdexcept>

#include <Renderer/D3D12Engine/Backend/RHI/Resources/DepthBuffer/DepthBuffer.hpp>

void D3D12Engine::DepthBuffer::Create(
  ID3D12Device* device,
  const std::wstring& name,
  UINT width,
  UINT height,
  DescriptorAllocator& DsvAllocator,
  DXGI_FORMAT format
) {
  m_Format = format;

  m_DsvAllocation = DsvAllocator.Allocate();

  D3D12_CLEAR_VALUE clearValue = { format, 1.0f, 0 };
  
  CD3DX12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
    format, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
  );

  CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

  device->CreateCommittedResource(
    &heapProps,
    D3D12_HEAP_FLAG_NONE,
    &depthDesc,
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    &clearValue,
    IID_PPV_ARGS(&m_pResource)
  );

  m_pResource->SetName(name.c_str());
  m_UsageState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

  D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
  dsvDesc.Format = format;
  dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

  device->CreateDepthStencilView(m_pResource.Get(), &dsvDesc, m_DsvAllocation.CPU);
}

void D3D12Engine::DepthBuffer::Shutdown(DescriptorAllocator& DsvAllocator) {
  if (m_DsvAllocation.IsValid()) {
    DsvAllocator.Free(m_DsvAllocation);
  }

  GpuResource::Shutdown();
}