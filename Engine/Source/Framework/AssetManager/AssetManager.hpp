#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <d3d12.h>

namespace D3D12Engine {
  class CommandContextPool;
  class CommandContext;
  class DescriptorAllocator;
  class CommandQueue;
  class Model;
  class Texture;
}

using namespace D3D12Engine;

class AssetManager {
 public:
  AssetManager() = default;
  ~AssetManager();

  void Initialize(
    ID3D12Device*        pDevice,
    DescriptorAllocator* pSrvAllocator,
    CommandContextPool*  pContextPool,
    CommandQueue*        pCommandQueue
  );

  std::shared_ptr<Model>   LoadModel(const std::string& FilePath);
  std::shared_ptr<Texture> LoadTexture(const std::string& FilePath);

  std::shared_ptr<Texture> GetDefaultTexture();

  void ExexuteUploads();
 private:
  std::unordered_map<std::string, std::shared_ptr<Model>>   m_Model;
  std::unordered_map<std::string, std::shared_ptr<Texture>> m_Texture;
  std::shared_ptr<Texture> m_pDefaultTexture = nullptr;

  ID3D12Device*        m_pDevice        = nullptr;
  DescriptorAllocator* m_pSrvAllocator  = nullptr;
  CommandContextPool*  m_pContextPool   = nullptr;
  CommandContext*      m_pUploadContext = nullptr;
  CommandQueue*        m_pCommandQueue  = nullptr;
};