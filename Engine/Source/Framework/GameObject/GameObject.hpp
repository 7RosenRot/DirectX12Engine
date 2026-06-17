#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <DirectXMath.h>

#include <Framework/Transform/Transform.hpp>

class AssetManager;

namespace D3D12Engine {
  class Model;
  class Texture;
}

class GameObject {
 public:
  GameObject(
    AssetManager* pAssetManager,
    const std::string& ObjectName = "default"
  );
  ~GameObject() = default;

  // ↓ Model Pipeline ↓
  void UpdateModelMatrix(
    DirectX::FXMMATRIX ModelView, DirectX::FXMMATRIX ModelProjection
  );

  void LoadModel(const std::string& FilePath);
  void LoadTexture(const std::string& FilePath);
  // ↑ Model Pipeline ↑
  
  // ↓ Object Name ↓
  void SetObjectName(const std::string& ObjectName) { m_ObjectName = ObjectName; }
  std::string& GetObjectName() { return m_ObjectName; }
  // ↑ Object Name ↑

  // ↓ Getters ↓
  Transform& GetTransform() { return m_Transform; }

  std::shared_ptr<D3D12Engine::Model>& GetModel() { return m_Model; }
  std::shared_ptr<D3D12Engine::Texture>& GetTexture() { return m_Texture; }
  DirectX::XMMATRIX& GetModelMatrix() { return m_ModelMatrix; }
  // ↑ Getters ↑

  // ↓ Paths ↓
  void SetModelPath(const std::string& rFilePath)   { m_ModelPath = rFilePath; }
  void SetTexturePath(const std::string& rFilePath) { m_TexturePath = rFilePath; }
  
  std::string GetModelPath()   const { return m_ModelPath; }
  std::string GetTexturePath() const { return m_TexturePath; }
  // ↑ Paths ↑

 private:
  AssetManager* m_pAssetManager = nullptr;

  Transform   m_Transform;

  std::string m_ObjectName;
  std::string m_ModelPath;
  std::string m_TexturePath;

  std::shared_ptr<D3D12Engine::Model>   m_Model;
  std::shared_ptr<D3D12Engine::Texture> m_Texture;
  DirectX::XMMATRIX m_ModelMatrix;
};