#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <d3d12.h>

namespace D3D12Engine {
  class CommandContext;
  class DescriptorAllocator;
  class CommandQueue;
  class Model;
  class Texture;
}

class AssetManager {
 public:
  AssetManager() = default;
  ~AssetManager() = default;

  void Initialize(
    ID3D12Device* pDevice,
    D3D12Engine::DescriptorAllocator* pSrvAllocator,
    D3D12Engine::CommandContext* pUploadContext,
    D3D12Engine::CommandQueue*   pCommandQueue
  );

  std::shared_ptr<D3D12Engine::Model>   LoadModel(const std::string& FilePath);
  std::shared_ptr<D3D12Engine::Texture> LoadTexture(const std::string& FilePath);

  void ExexuteUploads();
 private:
  std::unordered_map<std::string, std::shared_ptr<D3D12Engine::Model>>   m_Model;
  std::unordered_map<std::string, std::shared_ptr<D3D12Engine::Texture>> m_Texture;

  ID3D12Device* m_pDevice;
  D3D12Engine::DescriptorAllocator* m_pSrvAllocator;
  D3D12Engine::CommandContext* m_pUploadContext;
  D3D12Engine::CommandQueue*   m_pCommandQueue;
};