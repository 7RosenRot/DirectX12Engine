#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

#include <include/Graphics/Scene/Model.hpp>

bool D3D12Engine::Model::LoadObj(const std::string& filepath, ID3D12Device* device, CommandContext& uploadContext) {
  std::ifstream file(filepath);
  
  if (!file.is_open()) {
    std::cerr << "Failed to open OBJ file: " << filepath << std::endl;
    return false;
  }
  
  std::vector<DirectX::XMFLOAT3> temp_positions;
  std::vector<DirectX::XMFLOAT2> temp_texcoords;
  std::vector<DirectX::XMFLOAT3> temp_normals;
  
  std::vector<Vertex> vertices;
  std::vector<UINT> indices;
  
  std::string line;
  UINT currentIndex = 0;
  
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string type;
    
    iss >> type;
    if (type == "v") {
      DirectX::XMFLOAT3 pos;
      iss >> pos.x >> pos.y >> pos.z;
      pos.z *= -1.0f; 
      temp_positions.push_back(pos);
    }
    
    else if (type == "vt") {
      DirectX::XMFLOAT2 uv;
      iss >> uv.x >> uv.y;
      uv.y = 1.0f - uv.y;
      temp_texcoords.push_back(uv);
    }
    
    else if (type == "vn") {
      DirectX::XMFLOAT3 normal;
      iss >> normal.x >> normal.y >> normal.z;
      normal.z *= -1.0f;
      temp_normals.push_back(normal);
    }
    
    else if (type == "f") {
      for (int i = 0; i < 3; ++i) {
        std::string vertexData;
        iss >> vertexData;
        
        std::replace(vertexData.begin(), vertexData.end(), '/', ' ');
        std::istringstream viss(vertexData);
        
        int vIdx = 0, vtIdx = 0, vnIdx = 0;
        viss >> vIdx >> vtIdx >> vnIdx;
        Vertex vertex{};
        // Индексы в OBJ начинаются с 1
        if (vIdx > 0) vertex.Position = temp_positions[vIdx - 1];
        if (vtIdx > 0) vertex.TexCoord = temp_texcoords[vtIdx - 1];
        if (vnIdx > 0) vertex.Normal = temp_normals[vnIdx - 1];
        vertices.push_back(vertex);
        indices.push_back(currentIndex++);
      }
    }
  }
  m_IndexCount = static_cast<UINT>(indices.size());
  
  const UINT vbByteSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));
  const UINT ibByteSize = static_cast<UINT>(indices.size() * sizeof(UINT));
  
  std::wstring wName(filepath.begin(), filepath.end());
  
  m_VertexBuffer.Create(device, wName + L"_VB", vbByteSize);
  m_IndexBuffer.Create(device, wName + L"_IB", ibByteSize);
  
  uploadContext.InitializeBuffer(m_VertexBuffer, vertices.data(), vbByteSize);
  uploadContext.InitializeBuffer(m_IndexBuffer, indices.data(), ibByteSize);
  
  m_VertexBufferView.BufferLocation = m_VertexBuffer.GetResource()->GetGPUVirtualAddress();
  m_VertexBufferView.StrideInBytes = sizeof(Vertex);
  m_VertexBufferView.SizeInBytes = vbByteSize;
  m_IndexBufferView.BufferLocation = m_IndexBuffer.GetResource()->GetGPUVirtualAddress();
  m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
  m_IndexBufferView.SizeInBytes = ibByteSize;
  
  return true;
}

void D3D12Engine::Model::Draw(CommandContext& context) {
  context.TransitionResource(m_VertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
  context.TransitionResource(m_IndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
  context.FlushResourceBarriers();
  
  context.SetVertexBuffer(0, m_VertexBufferView);
  context.SetIndexBuffer(m_IndexBufferView);
  context.DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
}