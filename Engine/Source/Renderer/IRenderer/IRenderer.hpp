#pragma once

#include <windows.h>
#include <memory>
#include <string>
#include <DirectXMath.h>

namespace D3D12Engine {
  class Model;
  class Texture;
}

class IRenderer {
 public:
  IRenderer() = default;
  virtual ~IRenderer() = default;
  
  // ↓ Pipeline Stages ↓
  virtual void OnInitialize() = 0;
  virtual void OnResize(UINT WindowWidth, UINT WindowHeight) = 0;
  virtual void OnDestroy() = 0;
  // ↑ Pipeline Stages ↑

  // ↓ ImGui RTT Interface ↓
  virtual void InitUI() = 0;
  virtual void BeginUI() = 0;
  virtual void RenderUI() = 0;
  virtual void DestroyUI() = 0;

  virtual void ResizeViewport(UINT ViewportWidth, UINT ViewportHeight) = 0;
  
  virtual D3D12_GPU_DESCRIPTOR_HANDLE GetSceneTextureSRV() = 0;
  // ↑ ImGui RTT Interface ↑
  
  // ↓ Scene Rendering ↓
  virtual void BeginFrame() = 0;

  virtual void DrawFrame(
    D3D12Engine::Model& rModel,
    D3D12Engine::Texture& rTexture,
    const DirectX::XMMATRIX& rViewProjectionMatrix
  ) = 0;
  
  virtual void EndFrame() = 0;
  // ↑ Scene Rendering ↑

  // ↓ Uploading Assets ↓
  virtual std::shared_ptr<D3D12Engine::Model> LoadModel(const std::string FilePath) = 0;
  virtual std::shared_ptr<D3D12Engine::Texture> LoadTexture(const std::string FilePath) = 0;
  // ↑ Uploading Assets ↑
};