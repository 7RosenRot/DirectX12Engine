#include <Framework/GameObject/GameObject.hpp>

GameObject::GameObject(
  const std::string& ModelPath,
  const std::string& TexturePath
) : 
  m_ModelPath(ModelPath), m_TexturePath(TexturePath)
{}

void GameObject::Initialize(
  ID3D12Device* pDevice, D3D12Engine::CommandContext& rCommandContext
) {
  m_Model = std::make_unique<D3D12Engine::Model>();

  m_Model->LoadObj(m_ModelPath, pDevice, rCommandContext);
  m_Texture.LoadTexture(m_TexturePath, pDevice, rCommandContext);
}

void GameObject::UpdateModelMatrix(DirectX::FXMMATRIX ModelView, DirectX::FXMMATRIX ModelProjection) {
  m_ModelMatrix = m_Transform.GetMatrixModel();

  m_ModelMatrix = DirectX::XMMatrixTranspose(m_ModelMatrix * ModelView * ModelProjection);
}

void GameObject::Draw(D3D12Engine::CommandContext& rCommandContext) {
  if (m_Model == nullptr) {
    return;
  }

  m_Texture.Bind(rCommandContext, 1);

  rCommandContext.GetCommandList()->SetGraphicsRoot32BitConstants(0, 16, &m_ModelMatrix, 0);
  m_Model->Draw(rCommandContext);
}