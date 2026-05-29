#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <DirectXMath.h>

#include <Framework/Transform/Transform.hpp>
#include <Renderer/D3D12Engine/Model/Model.hpp>
#include <Renderer/D3D12Engine/Texture/Texture.hpp>

class GameObject {
 public:
  GameObject(
    const std::string& ModelPath = "",
    const std::string& TexturePath = ""
  );
  ~GameObject() = default;

  // ↓ Model Pipeline ↓
  void Initialize(
    ID3D12Device* pDevice, D3D12Engine::CommandContext& rCommandContext
  );
  
  void Draw(
    D3D12Engine::CommandContext& rCommandContext
  );

  void UpdateModelMatrix(
    DirectX::FXMMATRIX ModelView, DirectX::FXMMATRIX ModelProjection
  );
  // ↑ Model Pipeline ↑
  
  // ↓ Setters ↓
  void SetModelPath(const std::string& ModelPath) {
    m_ModelPath = ModelPath;
  }

  void SetTexturePath(const std::string& TexturePath) {
    m_TexturePath = TexturePath;
  }
  // ↑ Setters ↑

  // ↓ Getters ↓
  Transform& GetTransform() {
    return m_Transform;
  }
  // ↑ Getters ↑

 private:
  Transform m_Transform;
  
  std::unique_ptr<D3D12Engine::Model> m_Model;
  std::string m_ModelPath;
  DirectX::XMMATRIX m_ModelMatrix;
  
  D3D12Engine::Texture m_Texture;
  std::string m_TexturePath;
};