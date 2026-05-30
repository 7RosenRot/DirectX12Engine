#include <Framework/GameObject/GameObject.hpp>

GameObject::GameObject(
  const std::string& ModelPath,
  const std::string& TexturePath,
  const std::string& ObjectName
) : 
  m_ModelPath(ModelPath), m_TexturePath(TexturePath), m_ObjectName(ObjectName)
{}

void GameObject::InitContext(
  ID3D12Device* pDevice, D3D12Engine::CommandContext& rCommandContext
) {
  m_Model = std::make_shared<D3D12Engine::Model>();
  m_Texture = std::make_shared<D3D12Engine::Texture>();

  m_Model->LoadModel(m_ModelPath, pDevice, rCommandContext);
  m_Texture->LoadTexture(m_TexturePath, pDevice, rCommandContext);
}

void GameObject::UpdateModelMatrix(DirectX::FXMMATRIX ModelView, DirectX::FXMMATRIX ModelProjection) {
  m_ModelMatrix = m_Transform.GetMatrixModel();

  m_ModelMatrix = DirectX::XMMatrixTranspose(m_ModelMatrix * ModelView * ModelProjection);
}

void GameObject::Draw(D3D12Engine::CommandContext& rCommandContext) {
  if (m_Model == nullptr) {
    return;
  }

  m_Texture->Bind(rCommandContext, 1);

  rCommandContext.GetCommandList()->SetGraphicsRoot32BitConstants(0, 16, &m_ModelMatrix, 0);
  m_Model->DrawModel(rCommandContext);
}