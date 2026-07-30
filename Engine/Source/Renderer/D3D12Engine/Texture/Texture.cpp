#define STB_IMAGE_IMPLEMENTATION
#include <Renderer/D3D12Engine/Texture/Texture.hpp>

bool D3D12Engine::Texture::LoadTexture(
  const std::string& FilePath,
  ID3D12Device* pDevice,
  CommandContext& rUploadContext,
  DescriptorAllocator& rSrvAllocator
) {
  if (FilePath.empty()) {
    UINT32 purplePixel = 0xFFFF00FF;
    LoadFromMemory(&purplePixel, 1, 1, pDevice, rUploadContext, rSrvAllocator);
    return false;
  }

  int ImageWidth    = 1;
  int ImageHeight   = 1;
  int ImageChannels = 1;

  UCHAR* pImageData = stbi_load(FilePath.c_str(), &ImageWidth, &ImageHeight, &ImageChannels, 4);

  if (pImageData == nullptr) {
    OutputDebugStringA(("File not found: " + FilePath + "\n").c_str());
    
    UINT32 purplePixel = 0xFFFF00FF;
    LoadFromMemory(&purplePixel, 1, 1, pDevice, rUploadContext, rSrvAllocator);
    return false;
  }

  LoadFromMemory(
    pImageData,
    ImageWidth,
    ImageHeight,
    pDevice,
    rUploadContext,
    rSrvAllocator
  );

  stbi_image_free(pImageData);

  return true;
}

void D3D12Engine::Texture::LoadFromMemory(
  const void* pData,
  UINT Width,
  UINT Height,
  ID3D12Device* pDevice,
  D3D12Engine::CommandContext& rUploadContext,
  DescriptorAllocator& rSrvAllocator
) {
  D3D12_RESOURCE_DESC textureDescriptor = {};
  textureDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  textureDescriptor.Alignment = 0;
  textureDescriptor.Width = Width;
  textureDescriptor.Height = Height;
  textureDescriptor.DepthOrArraySize = 1;
  textureDescriptor.MipLevels = 1;
  textureDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  textureDescriptor.SampleDesc.Count = 1;
  textureDescriptor.SampleDesc.Quality = 0;
  textureDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  textureDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

  CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
  pDevice->CreateCommittedResource(
    &defaultHeapProperties,
    D3D12_HEAP_FLAG_NONE,
    &textureDescriptor,
    D3D12_RESOURCE_STATE_COPY_DEST,
    nullptr,
    IID_PPV_ARGS(&m_Texture)
  );

  UINT64 uploadBufferSize = 0;
  pDevice->GetCopyableFootprints(&textureDescriptor, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

  CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
  CD3DX12_RESOURCE_DESC bufferDescriptor = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
  
  pDevice->CreateCommittedResource(
    &uploadHeapProperties,
    D3D12_HEAP_FLAG_NONE,
    &bufferDescriptor,
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&m_UploadHeap)
  );

  D3D12_SUBRESOURCE_DATA textureData = {};
  textureData.pData = pData;
  textureData.RowPitch = Width * 4;
  textureData.SlicePitch = textureData.RowPitch * Height;

  UpdateSubresources(rUploadContext.GetCommandList(), m_Texture.Get(), m_UploadHeap.Get(), 0, 0, 1, &textureData);

  CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
    m_Texture.Get(),
    D3D12_RESOURCE_STATE_COPY_DEST,
    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
  );
  rUploadContext.GetCommandList()->ResourceBarrier(1, &barrier);

  m_SrvAllocation = rSrvAllocator.Allocate();

  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format = textureDescriptor.Format;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  
  pDevice->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SrvAllocation.CPU);
}

void D3D12Engine::Texture::Bind(
  CommandContext& rCommandContext,
  UINT RootParameters
) {
  rCommandContext.GetCommandList()->SetGraphicsRootDescriptorTable(
    RootParameters,
    m_SrvAllocation.GPU
  );
}