#include <Framework/GameObject/GameObject.hpp>

GameObject::GameObject(const std::string& ObjName, const std::string& ObjPath) : 
  m_ObjName(ObjName), m_ObjPath(ObjPath)
{}

void GameObject::Initialize(
  ID3D12Device* pDevice, D3D12Engine::CommandContext& rCommandContext
) {
  m_Model = std::make_unique<D3D12Engine::Model>();

  m_Model->LoadObj(m_ObjPath, pDevice, rCommandContext);
}

void GameObject::UpdateModelMatrix(DirectX::FXMMATRIX ModelView, DirectX::FXMMATRIX ModelProjection) {
  m_ModelMatrix = m_Transform.GetMatrixModel();

  m_ModelMatrix = DirectX::XMMatrixTranspose(m_ModelMatrix * ModelView * ModelProjection);
}

void GameObject::Draw(D3D12Engine::CommandContext& rCommandContext) {
  if (m_Model == nullptr) {
    return;
  }

  rCommandContext.GetCommandList()->SetGraphicsRoot32BitConstants(0, 16, &m_ModelMatrix, 0);
  m_Model->Draw(rCommandContext);
}