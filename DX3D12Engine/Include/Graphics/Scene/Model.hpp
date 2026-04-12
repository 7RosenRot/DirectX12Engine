#pragma once

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

#include <include/Graphics/Resources/GpuBuffer.hpp>
#include <include/Graphics/Core/CommandContext.hpp>

namespace D3D12Engine {
  class Model {
   public:
    struct Vertex {
      DirectX::XMFLOAT3 Position;
      DirectX::XMFLOAT2 TexCoord;
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