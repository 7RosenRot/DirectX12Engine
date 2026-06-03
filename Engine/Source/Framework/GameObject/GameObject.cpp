#include <Framework/GameObject/GameObject.hpp>

GameObject::GameObject(
  std::shared_ptr<D3D12Engine::Model> pModel,
  std::shared_ptr<D3D12Engine::Texture> pTexture,
  const std::string& ObjectName
) : 
  m_Model(pModel), m_Texture(pTexture), m_ObjectName(ObjectName)
{}

void GameObject::UpdateModelMatrix(DirectX::FXMMATRIX ModelView, DirectX::FXMMATRIX ModelProjection) {
  m_ModelMatrix = m_Transform.GetMatrixModel();

  m_ModelMatrix = DirectX::XMMatrixTranspose(m_ModelMatrix * ModelView * ModelProjection);
}