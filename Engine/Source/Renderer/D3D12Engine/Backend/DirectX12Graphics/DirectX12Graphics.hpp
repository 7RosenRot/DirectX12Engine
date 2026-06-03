#pragma once

#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <unordered_map>

#include <d3dx12.h>
#include <imgui_impl_dx12.h>

#include <Framework/Camera/Camera.hpp>
#include <Framework/GameObject/GameObject.hpp>

#include <Renderer/IRenderer/IRenderer.hpp>

#include <Renderer/D3D12Engine/Model/Model.hpp>

#include <Renderer/D3D12Engine/Backend/RHI/Core/CommandContext/CommandContext.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/SwapChain/SwapChain.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/CommandQueue/CommandQueue.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/PipelineState/PipelineState.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/RootSignature/RootSignature.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/DepthBuffer/DepthBuffer.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/ColorBuffer/ColorBuffer.hpp>

namespace D3D12Engine {
  class DirectX12Graphics : public IRenderer {
   public:
    DirectX12Graphics(HWND hwnd, UINT WindowWidth, UINT WindowHeight);
    ~DirectX12Graphics() override;

    // ↓ Pipeline Stages ↓
      void OnInitialize() override;
      void OnResize(UINT WindowWidth, UINT WindowHeight) override;
      void OnDestroy() override;
    // ↑ Pipeline Stages ↑

    // ↓ ImGui RTT Interface ↓
    void InitUI() override;
    void BeginUI() override;
    void RenderUI() override;
    void DestroyUI() override;

    void ResizeViewport(UINT ViewportWidth, UINT ViewportHeight) override;

    D3D12_GPU_DESCRIPTOR_HANDLE GetSceneTextureSRV() override;
    // ↑ ImGui RTT Interface ↑

    // ↓ Scene Rendering ↓
      void BeginFrame() override;

      void DrawFrame(
        D3D12Engine::Model& rModel,
        D3D12Engine::Texture& rTexture,
        const DirectX::XMMATRIX& rViewProjectionMatrix
      ) override;

      void EndFrame() override;
    // ↑ Scene Rendering ↑

    // ↓ Uploading Assets ↓
      std::shared_ptr<D3D12Engine::Model> LoadModel(const std::string FilePath) override;
      std::shared_ptr<D3D12Engine::Texture> LoadTexture(const std::string FilePath) override;
    // ↑ Uploading Assets ↑
  
   private:
    // ↓ Window properties & Adapter request ↓
      HWND m_hwnd = nullptr;
      
      UINT m_WindowWidth = 0;
      UINT m_WindowHeight = 0;
      
      UINT m_ViewportWidth = 0;
      UINT m_ViewportHeight = 0;

      bool m_useWarpAdapter = false;
    // ↑ Window properties & Adapter request ↑

    // ↓ ImGui RTT ↓
      ColorBuffer m_SceneTexture;
      Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_UiSrvHeap;
    // ↑ ImGui RTT ↑

    // ↓ Pipeline modules ↓ 
      Microsoft::WRL::ComPtr<ID3D12Device> m_device;
      Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;

      std::unique_ptr<CommandQueue> m_cmdQueue;
      std::unique_ptr<CommandContext> m_cmdContext;
      std::unique_ptr<SwapChain> m_display;

      DepthBuffer m_depthBuffer;

      RootSignature m_rootSignature;
      GraphicsPSO m_pipelineState{L"Main PipelineStateObject"};

      CD3DX12_VIEWPORT m_viewPort{};
      CD3DX12_RECT m_scissorRect{};
    // ↑ Pipeline modules ↑

    void GetHardwareAdapter(
      _In_ IDXGIFactory1* pFactory1,
      _Outptr_opt_result_maybenull_ IDXGIAdapter1** ppAdapter1,
      bool requestHighPerformanceAdapter
    );
    
    void LoadPipeline();
    void LoadAssets();
  };
}
