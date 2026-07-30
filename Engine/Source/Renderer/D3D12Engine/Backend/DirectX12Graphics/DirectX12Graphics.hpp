#pragma once

#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <unordered_map>

#include <d3dx12.h>

#include <Framework/Camera/Camera.hpp>
#include <Framework/GameObject/GameObject.hpp>

#include <Renderer/D3D12Engine/Model/Model.hpp>

#include <Renderer/D3D12Engine/Backend/RHI/Core/CommandContext/CommandContext.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Core/CommandContextPool/CommandContextPool.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/SwapChain/SwapChain.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/CommandQueue/CommandQueue.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/PipelineState/PipelineState.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/RootSignature/RootSignature.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/DepthBuffer/DepthBuffer.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/ColorBuffer/ColorBuffer.hpp>

namespace D3D12Engine {
  class DirectX12Graphics {
   public:
    DirectX12Graphics(HWND hwnd, UINT WindowWidth, UINT WindowHeight);
    ~DirectX12Graphics();

    // ↓ Pipeline Stages ↓
      void OnInitialize();
      void OnResize(UINT WindowWidth, UINT WindowHeight);
      void OnDestroy();
    // ↑ Pipeline Stages ↑

    // ↓ RTT Interface ↓
      void ResizeViewport(UINT ViewportWidth, UINT ViewportHeight);

      D3D12_GPU_DESCRIPTOR_HANDLE GetSceneTextureSRV() const {
        return m_SceneTexture.GetSRVGpuHandle();
      }
    // ↑ RTT Interface ↑

    // ↓ Scene Rendering ↓
      void BeginFrame();

      void DrawNormal(
        D3D12Engine::Model& rModel,
        D3D12Engine::Texture& rTexture,
        const DirectX::XMMATRIX& rViewProjectionMatrix,
        bool isSelected
      );

      void DrawOutline(
        D3D12Engine::Model& rModel,
        const DirectX::XMMATRIX& rViewProjectionMatrix
      );

      void DrawStencil(
        D3D12Engine::Model& rModel,
        const DirectX::XMMATRIX& rViewProjectionMatrix
      );

      void EndFrame();
    // ↑ Scene Rendering ↑

    void PrepareUIContext();

    // ↓ Get Resource ↓
      ID3D12Device* GetDevice() const {
        return m_device.Get();
      }
      
      CommandQueue* GetCommandQueue() const {
        return m_cmdQueue.get();
      }
      
      ID3D12CommandQueue* GetCommandQueueResource() const {
        return m_cmdQueue->GetResource();
      }
      
      CommandContext* GetCommandContext() const {
        return m_pFrameContext;
      }

      CommandContextPool* GetContextPool() const {
        return m_cmdContextPool.get();
      }
      
      ID3D12GraphicsCommandList* GetCommandList() const {
        return m_pFrameContext ? m_pFrameContext->GetCommandList() : nullptr;
      }
      
      DescriptorAllocator* GetSrvAllocator() const {
        return m_SrvAllocator.get(); 
      }
    // ↑ Get Resource ↑
  
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
      std::unique_ptr<DescriptorAllocator> m_SrvAllocator;
      std::unique_ptr<DescriptorAllocator> m_RtvAllocator;
      std::unique_ptr<DescriptorAllocator> m_DsvAllocator;
    // ↑ ImGui RTT ↑

    // ↓ ImGui RTT ↓
      ColorBuffer m_SceneTexture;
    // ↑ ImGui RTT ↑

    // ↓ Pipeline modules ↓ 
      Microsoft::WRL::ComPtr<ID3D12Device> m_device;
      Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;

      std::unique_ptr<CommandQueue> m_cmdQueue;
      std::unique_ptr<SwapChain> m_SwapChain;
      
      std::unique_ptr<CommandContextPool> m_cmdContextPool;
      CommandContext* m_pFrameContext = nullptr;

      DepthBuffer m_DepthBuffer;

      RootSignature m_rootSignature;
      GraphicsPSO m_pipelineState{L"Main PipelineStateObject"};
      GraphicsPSO m_outlinePipelineState{L"Outline PipelineStateObject"};
      GraphicsPSO m_stencilWritePipelineState{L"Stencil Write PipelineStateObject"};
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
