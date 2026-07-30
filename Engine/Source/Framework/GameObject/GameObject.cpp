#include <Framework/GameObject/GameObject.hpp>

#include <Framework/AssetManager/AssetManager.hpp>
#include <Framework/Logger/Logger.hpp>
#include <Renderer/D3D12Engine/Model/Model.hpp>
#include <Renderer/D3D12Engine/Texture/Texture.hpp>

GameObject::GameObject(
  AssetManager* pAssetManager,
  const std::string& ObjectName
) :
  m_pAssetManager(pAssetManager),
  m_ObjectName(ObjectName)
{
  if (m_pAssetManager != nullptr) {
    m_Texture = m_pAssetManager->GetDefaultTexture();

    m_pAssetManager->ExexuteUploads();
  }
}

void GameObject::UpdateModelMatrix(DirectX::FXMMATRIX ModelView, DirectX::FXMMATRIX ModelProjection) {
  m_ModelMatrix = m_Transform.GetMatrixModel();

  m_ModelMatrix = DirectX::XMMatrixTranspose(m_ModelMatrix * ModelView * ModelProjection);
}

void GameObject::LoadModel(const std::string& rFilePath) {
  if (m_pAssetManager && !rFilePath.empty()) {
    m_Model = m_pAssetManager->LoadModel(rFilePath);
    m_ModelPath = rFilePath;

    m_pAssetManager->ExexuteUploads();
  }
}

void GameObject::LoadTexture(const std::string& rFilePath) {
  if (m_pAssetManager && !rFilePath.empty()) {
    m_Texture = m_pAssetManager->LoadTexture(rFilePath);
    m_TexturePath = rFilePath;

    m_pAssetManager->ExexuteUploads();
  }
}