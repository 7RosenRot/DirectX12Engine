#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <Framework/Camera/Camera.hpp>
#include <Framework/GameObject/GameObject.hpp>

namespace D3D12Engine {
  class DirectX12Graphics;
}

class Scene {
 public:
  Scene() = default;
  ~Scene() = default;

  void AddGameObject(const std::string& ObjectName, std::shared_ptr<GameObject> pGameObject);
  void UpdateScene(float MovementSpeed, float MouseSensivity);
  void RenderScene(D3D12Engine::DirectX12Graphics& rRenderer);
  
  Camera& GetActiveCamera() {
    return m_ActiveCamera;
  }

  auto& GetGameObjects() {
    return m_GameObjects;
  }
 private:
  Camera m_ActiveCamera;
  std::unordered_map<std::string, std::shared_ptr<GameObject>> m_GameObjects;
};