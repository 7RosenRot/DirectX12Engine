#include <Renderer/D3D12Engine/Backend/RHI/Resources/ColorBuffer/ColorBuffer.hpp>

void D3D12Engine::ColorBuffer::CreateScene(
  ID3D12Device* pDevice,
  const std::wstring& SceneName,
  UINT SceneWidth,
  UINT SceneHeight,
  DXGI_FORMAT Format
) {
  m_Format = Format;

  // ↓ RTV Heap ↓
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors = 1;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

  pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap));
  m_RtvHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
  // ↑ RTV Heap ↑

  // ↓ SRV Heap ↓
  D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
  srvHeapDesc.NumDescriptors = 1;
  srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

  pDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvHeap));
  m_SrvHandle = m_SrvHeap->GetCPUDescriptorHandleForHeapStart();
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
    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
    &ClearValue,
    
    IID_PPV_ARGS(&m_pResource)
  );

  m_pResource->SetName(SceneName.c_str());
  m_UsageState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

  pDevice->CreateRenderTargetView(m_pResource.Get(), nullptr, m_RtvHandle);

  D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
  SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  SrvDesc.Format = Format;
  SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  SrvDesc.Texture2D.MipLevels = 1;

  pDevice->CreateShaderResourceView(m_pResource.Get(), &SrvDesc, m_SrvHandle);
}