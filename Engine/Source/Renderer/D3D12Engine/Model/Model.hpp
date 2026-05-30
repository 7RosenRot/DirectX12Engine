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
    Model() = default;
    ~Model() = default;

    bool LoadModel(
      const std::string& rFilePath,
      ID3D12Device*      pDevice,
      CommandContext&    rCommandContext
    );
    
    void DrawModel(
      CommandContext&    rCommandContext
    );

   private:
    struct Vertex {
      DirectX::XMFLOAT3 Position;
      DirectX::XMFLOAT2 Texture;
      DirectX::XMFLOAT3 Normal;
    };

    GpuBuffer m_VertexBuffer;
    GpuBuffer m_IndexBuffer;
    
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView{};
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView{};
    
    UINT m_IndexCount{0};
  };
}