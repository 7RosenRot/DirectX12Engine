#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include <Framework/Camera/Camera.hpp>

namespace D3D12Engine {
  class DirectX12Graphics;
}

class AssetManager;
class GameObject;

class Scene {
 public:
  Scene() = default;
  ~Scene() = default;

  void Initialize(AssetManager* AssetManager);
  void AddGameObject(const std::string& FilePath);
  void UpdateScene(float MovementSpeed, float MouseSensivity, bool BlockCameraInput = false, const DirectX::XMFLOAT3& TargetPoint = {0.0f, 0.0f, 0.0f});
  void RenderScene(D3D12Engine::DirectX12Graphics& rRenderer, const std::vector<std::shared_ptr<GameObject>>& selectedObjects = {});
  
  void SaveScene(const std::string& FilePath);
  void LoadScene(const std::string& FilePath);
  
  Camera& GetActiveCamera() {
    return m_ActiveCamera;
  }

  auto& GetGameObjects() {
    return m_GameObjects;
  }

  void RemoveGameObject(const std::string& ObjectName) {
    m_GameObjects.erase(ObjectName);
  }
 private:
  AssetManager* m_pAssetManager = nullptr;
  Camera m_ActiveCamera;
  std::unordered_map<std::string, std::shared_ptr<GameObject>> m_GameObjects;
};