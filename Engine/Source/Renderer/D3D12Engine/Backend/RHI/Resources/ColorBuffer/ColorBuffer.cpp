#include <Renderer/D3D12Engine/Backend/RHI/Resources/ColorBuffer/ColorBuffer.hpp>

bool D3D12Engine::ColorBuffer::Initialize(
  ID3D12Device* pDevice,
  const std::wstring& SceneName,
  UINT SceneWidth,
  UINT SceneHeight,
  DescriptorAllocator& RtvAllocator,
  DescriptorAllocator& SrvAllocator,
  DXGI_FORMAT Format
) {
  // ↓ RTV Heap ↓
  if (!m_RtvAllocation.IsValid()) {
    m_RtvAllocation = RtvAllocator.Allocate();
  }
  // ↑ RTV Heap ↑

  // ↓ SRV Heap ↓
  if (!m_SrvAllocation.IsValid()) {
    m_SrvAllocation = SrvAllocator.Allocate();
  }
  // ↑ SRV Heap ↑

  D3D12_RESOURCE_DESC TextureDesc = {};
  TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  TextureDesc.Width = SceneWidth;
  TextureDesc.Height = SceneHeight;
  TextureDesc.DepthOrArraySize = 1;
  TextureDesc.MipLevels = 1;
  TextureDesc.Format = Format;
  TextureDesc.SampleDesc.Count = 1;
  TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

  D3D12_CLEAR_VALUE ClearValue = {};
  ClearValue.Format = Format;
  ClearValue.Color[0] = 0.1F;
  ClearValue.Color[1] = 0.1F;
  ClearValue.Color[2] = 0.1F;
  ClearValue.Color[3] = 1.0F;

  CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_DEFAULT);
  pDevice->CreateCommittedResource(
    &HeapProperties,
    D3D12_HEAP_FLAG_NONE,
    
    &TextureDesc,
    D3D12_RESOURCE_STATE_COMMON,
    &ClearValue,
    
    IID_PPV_ARGS(&m_pResource)
  );

  D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
  RtvDesc.Format = Format;
  RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
  
  pDevice->CreateRenderTargetView(m_pResource.Get(), &RtvDesc, m_RtvAllocation.CPU);

  D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
  SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  SrvDesc.Format = Format;
  SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  SrvDesc.Texture2D.MipLevels = 1;

  pDevice->CreateShaderResourceView(m_pResource.Get(), &SrvDesc, m_SrvAllocation.CPU);

  if (m_pResource != nullptr) {
    m_pResource->SetName(SceneName.c_str());
  }

  m_Format = Format;
  
  SetUsageState(D3D12_RESOURCE_STATE_COMMON);

  return true;
}

void D3D12Engine::ColorBuffer::Shutdown(
  DescriptorAllocator& RtvAllocator,
  DescriptorAllocator& SrvAllocator
) {
  if (m_RtvAllocation.IsValid()) {
    RtvAllocator.Free(m_RtvAllocation);
  }

  if (m_SrvAllocation.IsValid()) {
    SrvAllocator.Free(m_SrvAllocation);
  }

  GpuResource::Shutdown();
}