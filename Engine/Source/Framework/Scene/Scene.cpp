#include <Framework/Scene/Scene.hpp>
#include <Renderer/IRenderer/IRenderer.hpp>

void Scene::AddGameObject(
  const std::string& ObjectName,
  std::shared_ptr<GameObject> pGameObject
) {
  m_GameObjects[ObjectName] = pGameObject;
}

void Scene::UpdateScene(const float MovementSpeed, const float MouseSensivity) {
  m_ActiveCamera.InputProcessing(MovementSpeed, MouseSensivity);
}

void Scene::RenderScene(IRenderer& rIRenderer) {
  DirectX::XMMATRIX View = m_ActiveCamera.GetTransform().GetMatrixView();
  DirectX::XMMATRIX Projection = m_ActiveCamera.GetMatrixProjection();

  for (auto& [ObjectName, pGameObject] : m_GameObjects) {
    pGameObject->UpdateModelMatrix(View, Projection);
    
    rIRenderer.DrawFrame(
      *pGameObject->GetModel(),
      *pGameObject->GetTexture(),
       pGameObject->GetModelMatrix()
    );
  }
}