#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <DirectXMath.h>

#include <Framework/Transform/Transform.hpp>
#include <Renderer/D3D12Engine/Model/Model.hpp>

class GameObject {
 public:
  GameObject(const std::string& ObjName, const std::string& ObjPath);
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

  // ↓ Getters ↓
  Transform& GetTransform() {
    return m_Transform;
  }

  const std::string& GetObjName() const {
    return m_ObjName;
  }
  // ↑ Getters ↑

 private:
  Transform m_Transform;
  
  const std::string m_ObjName;
  const std::string m_ObjPath;
  DirectX::XMFLOAT3 m_Position;
  
  std::unique_ptr<D3D12Engine::Model> m_Model;
  DirectX::XMMATRIX m_ModelMatrix;
};