#pragma once

#include <wrl/client.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <stb_image.h>
#include <string>
#include <iostream>
#include <stdexcept>

#include <Renderer/D3D12Engine/Backend/RHI/Core/CommandContext/CommandContext.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>

namespace D3D12Engine {
  class Texture {
   public:
    Texture() = default;
    ~Texture() = default;

    bool LoadTexture(
      const std::string& FilePath,
      ID3D12Device* pDevice,
      CommandContext& rCommandContext,
      DescriptorAllocator& rSrvAllocator
    );

    void Bind(
      CommandContext& rCommandContext,
      UINT RootParameters
    );

    ID3D12Resource* GetResource() {
      return m_Texture.Get();
    }
   private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_Texture;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadHeap;
    DescriptorAllocation m_SrvAllocation;
  };
}