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
    std::shared_ptr<D3D12Engine::Model> pModel,
    std::shared_ptr<D3D12Engine::Texture> pTexture,
    const std::string& ObjectName  = "default"
  );
  ~GameObject() = default;

  // ↓ Model Pipeline ↓
  void UpdateModelMatrix(
    DirectX::FXMMATRIX ModelView, DirectX::FXMMATRIX ModelProjection
  );
  // ↑ Model Pipeline ↑
  
  // ↓ Setters ↓
  void SetObjectName(const std::string& ObjectName) {
    m_ObjectName = ObjectName;
  }
  // ↑ Setters ↑

  // ↓ Getters ↓
  Transform& GetTransform() {
    return m_Transform;
  }

  std::shared_ptr<D3D12Engine::Model>& GetModel() {
    return m_Model;
  }

  DirectX::XMMATRIX& GetModelMatrix() {
    return m_ModelMatrix;
  }
  
  std::shared_ptr<D3D12Engine::Texture>& GetTexture() {
    return m_Texture;
  }
  // ↑ Getters ↑

 private:
  Transform m_Transform;

  std::string m_ObjectName;
  
  std::shared_ptr<D3D12Engine::Model> m_Model;
  DirectX::XMMATRIX m_ModelMatrix;
  
  std::shared_ptr<D3D12Engine::Texture> m_Texture;
};