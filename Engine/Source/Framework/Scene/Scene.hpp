#pragma once

#include <string>
#include <memory>
#include <unordered_map>

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
  void UpdateScene(float MovementSpeed, float MouseSensivity, bool BlockCameraInput = false);
  void RenderScene(D3D12Engine::DirectX12Graphics& rRenderer, std::shared_ptr<GameObject> pSelectedObject = nullptr);
  
  void SaveScene(const std::string& FilePath);
  void LoadScene(const std::string& FilePath);
  
  Camera& GetActiveCamera() {
    return m_ActiveCamera;
  }

  auto& GetGameObjects() {
    return m_GameObjects;
  }
 private:
  AssetManager* m_pAssetManager = nullptr;
  Camera m_ActiveCamera;
  std::unordered_map<std::string, std::shared_ptr<GameObject>> m_GameObjects;
};