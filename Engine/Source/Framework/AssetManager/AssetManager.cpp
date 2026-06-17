#include <Framework/AssetManager/AssetManager.hpp>
#include <Framework/Logger/Logger.hpp>
#include <Renderer/D3D12Engine/Model/Model.hpp>
#include <Renderer/D3D12Engine/Texture/Texture.hpp>

#include <Renderer/D3D12Engine/Backend/RHI/Core/CommandContextPool/CommandContextPool.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/CommandQueue/CommandQueue.hpp>

AssetManager::~AssetManager() {
  if (m_pUploadContext && m_pContextPool) {
    m_pContextPool->Free(m_pUploadContext);
  }
}

void AssetManager::Initialize(
  ID3D12Device* pDevice,
  DescriptorAllocator* pSrvAllocator,
  CommandContextPool* pContextPool,
  CommandQueue*   pCommandQueue
) {
  m_pDevice = pDevice;
  
  m_pSrvAllocator = pSrvAllocator;
  
  m_pContextPool = pContextPool;
  
  m_pUploadContext = pContextPool->Allocate(
    pCommandQueue->GetCompletedFenceValue()
  );
  
  m_pCommandQueue = pCommandQueue;
}

std::shared_ptr<Model>
  AssetManager::LoadModel(const std::string& rFilePath) {
    if (m_Model.find(rFilePath) != m_Model.end()) {
      return m_Model[rFilePath];
    }

    auto Model = std::make_shared<D3D12Engine::Model>();

    Model->LoadModel(rFilePath, m_pDevice, *m_pUploadContext);

    m_Model[rFilePath] = Model;

    return Model;
  }

std::shared_ptr<Texture>
  AssetManager::LoadTexture(const std::string& rFilePath) {
    if (m_Texture.find(rFilePath) != m_Texture.end()) {
      return m_Texture[rFilePath];
    }

    auto Texture = std::make_shared<D3D12Engine::Texture>();

    bool flag = Texture->LoadTexture(rFilePath, m_pDevice, *m_pUploadContext, *m_pSrvAllocator);

    if (!flag) {
      return GetDefaultTexture();
    }

    m_Texture[rFilePath] = Texture;

    return Texture;
  }

std::shared_ptr<Texture>
  AssetManager::GetDefaultTexture() {
    if (m_pDefaultTexture != nullptr) {
      return m_pDefaultTexture;
    }

    m_pDefaultTexture = std::make_shared<Texture>();

    uint8_t GrayPixel[4] = { 128, 128, 128, 128 };

    m_pDefaultTexture->LoadFromMemory(
      GrayPixel, 1, 1, m_pDevice, *m_pUploadContext, *m_pSrvAllocator
    );

    return m_pDefaultTexture;
  }

void AssetManager::ExexuteUploads() {
  if (m_pUploadContext == nullptr) {
    return;
  }
  
  m_pUploadContext->Close();
  
  m_pCommandQueue->ExecuteCommandList(m_pUploadContext->GetCommandList());
  m_pCommandQueue->Flush();

  m_pContextPool->Free(m_pUploadContext);
  m_pUploadContext = m_pContextPool->Allocate(m_pCommandQueue->GetCompletedFenceValue());
}