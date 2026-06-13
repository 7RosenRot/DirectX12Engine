#include <Framework/Scene/Scene.hpp>
#include <Renderer/D3D12Engine/Backend/DirectX12Graphics/DirectX12Graphics.hpp>

void Scene::AddGameObject(
  const std::string& ObjectName,
  std::shared_ptr<GameObject> pGameObject
) {
  m_GameObjects[ObjectName] = pGameObject;
}

void Scene::UpdateScene(const float MovementSpeed, const float MouseSensivity) {
  m_ActiveCamera.InputProcessing(MovementSpeed, MouseSensivity);
}

void Scene::RenderScene(D3D12Engine::DirectX12Graphics& rRenderer) {
  DirectX::XMMATRIX View = m_ActiveCamera.GetTransform().GetMatrixView();
  DirectX::XMMATRIX Projection = m_ActiveCamera.GetMatrixProjection();

  for (auto& [ObjectName, pGameObject] : m_GameObjects) {
    pGameObject->UpdateModelMatrix(View, Projection);
    
    rRenderer.DrawFrame(
      *pGameObject->GetModel(),
      *pGameObject->GetTexture(),
       pGameObject->GetModelMatrix()
    );
  }
}