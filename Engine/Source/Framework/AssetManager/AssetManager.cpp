#include <Framework/AssetManager/AssetManager.hpp>

#include <Renderer/D3D12Engine/Model/Model.hpp>
#include <Renderer/D3D12Engine/Texture/Texture.hpp>

#include <Renderer/D3D12Engine/Backend/RHI/Core/CommandContext/CommandContext.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/CommandQueue/CommandQueue.hpp>

void AssetManager::Initialize(
  ID3D12Device* pDevice,
  D3D12Engine::DescriptorAllocator* pSrvAllocator,
  D3D12Engine::CommandContext* pUploadContext,
  D3D12Engine::CommandQueue*   pCommandQueue
) {
  m_pDevice = pDevice;
  m_pSrvAllocator = pSrvAllocator;
  m_pUploadContext = pUploadContext;
  m_pCommandQueue = pCommandQueue;
}

std::shared_ptr<D3D12Engine::Model>
  AssetManager::LoadModel(const std::string& rFilePath) {
    if (m_Model.find(rFilePath) != m_Model.end()) {
      return m_Model[rFilePath];
    }

    auto Model = std::make_shared<D3D12Engine::Model>();

    Model->LoadModel(rFilePath, m_pDevice, *m_pUploadContext);

    m_Model[rFilePath] = Model;

    return Model;
  }

std::shared_ptr<D3D12Engine::Texture>
  AssetManager::LoadTexture(const std::string& rFilePath) {
    if (m_Texture.find(rFilePath) != m_Texture.end()) {
      return m_Texture[rFilePath];
    }

    auto Texture = std::make_shared<D3D12Engine::Texture>();

    Texture->LoadTexture(rFilePath, m_pDevice, *m_pUploadContext, *m_pSrvAllocator);

    m_Texture[rFilePath] = Texture;

    return Texture;
  }

void AssetManager::ExexuteUploads() {
  m_pUploadContext->Close();
  
  UINT64 fenceValue = m_pCommandQueue->ExecuteCommandList(m_pUploadContext->GetCommandList());
  m_pCommandQueue->WaitForPreviousFrame(fenceValue);
}