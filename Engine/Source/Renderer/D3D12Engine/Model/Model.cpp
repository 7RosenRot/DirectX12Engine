#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

#include <Renderer/D3D12Engine/Model/Model.hpp>

bool D3D12Engine::Model::LoadModel(
  const std::string& rFilePath,
  ID3D12Device*      pDevice,
  CommandContext&    rCommandContext
) {
  std::ifstream ObjFile(rFilePath);
  
  if (!ObjFile.is_open()) {
    OutputDebugStringA(("File not found: " + rFilePath + "\n").c_str());
    
    return false;
  }
  
  std::vector<DirectX::XMFLOAT3> tmpPositionBuffer;
  std::vector<DirectX::XMFLOAT2> tmpTextureBuffer;
  std::vector<DirectX::XMFLOAT3> tmpNarmalBuffer;
  
  std::vector<Vertex> Vertices;
  std::vector<UINT>   Indices;
  
  std::string crtReadingLine;
  UINT crtIndex = 0;
  
  while (std::getline(ObjFile, crtReadingLine)) {
    std::istringstream Stream(crtReadingLine);
    std::string lineType;
    
    Stream >> lineType;
    // ↓ If line is Vertex type ↓
    if (lineType == "v") {
      DirectX::XMFLOAT3 Position;
      Stream >> Position.x >> Position.y >> Position.z;
      Position.z *= -1.0f; 
      
      tmpPositionBuffer.push_back(Position);
    }
    // ↑ If line is Vertex type ↑
    
    // ↓ If line is Texture type ↓
    else if (lineType == "vt") {
      DirectX::XMFLOAT2 Texture;
      Stream >> Texture.x >> Texture.y;
      Texture.y = 1.0f - Texture.y;

      tmpTextureBuffer.push_back(Texture);
    }
    // ↑ If line is Texture type ↑
    
    // ↓ If line is Normal type ↓
    else if (lineType == "vn") {
      DirectX::XMFLOAT3 Normal;
      Stream >> Normal.x >> Normal.y >> Normal.z;
      Normal.z *= -1.0f;
      
      tmpNarmalBuffer.push_back(Normal);
    }
    // ↑ If line is Normal type ↑
    
    // ↓ If line is Face type ↓
    else if (lineType == "f") {
      for (int i = 0; i < 3; ++i) {
        std::string VertexData;
        Stream >> VertexData;
        
        std::replace(VertexData.begin(), VertexData.end(), '/', ' ');
        std::istringstream VertexStream(VertexData);
        
        int VertexPositionIdx = 0, VertexTextureIdx = 0, VertexNormalIdx = 0;
        VertexStream >> VertexPositionIdx >> VertexTextureIdx >> VertexNormalIdx;
        
        Vertex VertexStruct{};
        
        if (VertexPositionIdx > 0) {
          VertexStruct.Position = tmpPositionBuffer[VertexPositionIdx - 1];
        }
        
        if (VertexTextureIdx > 0) {
          VertexStruct.Texture = tmpTextureBuffer[VertexTextureIdx - 1];
        }
        
        if (VertexNormalIdx > 0) {
          VertexStruct.Normal = tmpNarmalBuffer[VertexNormalIdx - 1];
        }
        
        Vertices.push_back(VertexStruct);
        Indices.push_back(crtIndex++);
      }
    }
    // ↑ If line is Face type ↑
  }
  
  // ↓ Set Up Enviromment ↓
  m_IndexCount = static_cast<UINT>(Indices.size());
  m_Vertices = Vertices;
  m_Indices  = Indices;
  
  const UINT VertexBufferByteSize = static_cast<UINT>(Vertices.size() * sizeof(Vertex));
  const UINT IndexBufferByteSize = static_cast<UINT>(Indices.size() * sizeof(UINT));
  
  std::wstring ObjDebugName(rFilePath.begin(), rFilePath.end());
  // ↑ Set Up Enviromment ↑

  // ↓ Vertex Buffer - Create, Initialize, Fill Structure ↓
  m_VertexBuffer.Create(pDevice, ObjDebugName + L"_VB", VertexBufferByteSize);
  
  rCommandContext.InitializeBuffer(m_VertexBuffer, Vertices.data(), VertexBufferByteSize);
  
  m_VertexBufferView.BufferLocation = m_VertexBuffer.GetResource()->GetGPUVirtualAddress();
  m_VertexBufferView.StrideInBytes = sizeof(Vertex);
  m_VertexBufferView.SizeInBytes = VertexBufferByteSize;
  // ↑ Vertex Buffer - Create, Initialize, Fill Structure ↑

  // ↓ Index Buffer - Create, Initialize, Fill Structure ↓
  m_IndexBuffer.Create(pDevice, ObjDebugName + L"_IB", IndexBufferByteSize);

  rCommandContext.InitializeBuffer(m_IndexBuffer, Indices.data(), IndexBufferByteSize);
  
  m_IndexBufferView.BufferLocation = m_IndexBuffer.GetResource()->GetGPUVirtualAddress();
  m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
  m_IndexBufferView.SizeInBytes = IndexBufferByteSize;
  // ↑ Index Buffer - Create, Initialize, Fill Structure ↑
  
  return true;
}

void D3D12Engine::Model::DrawModel(CommandContext& rCommandContext) {
  rCommandContext.TransitionResource(
    m_VertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
  );
  rCommandContext.TransitionResource(
    m_IndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER
  );
  
  rCommandContext.FlushResourceBarriers();
  
  rCommandContext.SetVertexBuffer(0, m_VertexBufferView);
  rCommandContext.SetIndexBuffer(m_IndexBufferView);
  rCommandContext.DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
}