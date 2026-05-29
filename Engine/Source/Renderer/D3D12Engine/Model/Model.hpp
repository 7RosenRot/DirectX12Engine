#pragma once

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

#include <Renderer/D3D12Engine/Texture/Texture.hpp>

#include <Renderer/D3D12Engine/Backend/RHI/Resources/GpuBuffer/GpuBuffer.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Core/CommandContext/CommandContext.hpp>

namespace D3D12Engine {
  class Model {
   public:
    struct Vertex {
      DirectX::XMFLOAT3 Position;
      DirectX::XMFLOAT2 Texture;
      DirectX::XMFLOAT3 Normal;
    };

    Model() = default;
    ~Model() = default;

    bool LoadObj(
      const std::string& filepath, ID3D12Device* device, CommandContext& uploadContext
    );
    
    void Draw(CommandContext& context);

   private:
    GpuBuffer m_VertexBuffer;
    GpuBuffer m_IndexBuffer;
    
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView{};
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView{};
    
    UINT m_IndexCount{0};
  };
}