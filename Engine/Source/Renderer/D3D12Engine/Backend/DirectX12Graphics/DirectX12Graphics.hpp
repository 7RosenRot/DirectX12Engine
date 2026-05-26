#pragma once

#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <string>

// DirectX include path
#include <Framework/Camera/Camera.hpp>

#include <Renderer/IRenderer/IRenderer.hpp>

#include <Renderer/D3D12Engine/Model/Model.hpp>

#include <Renderer/D3D12Engine/Backend/RHI/Core/CommandContext/CommandContext.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/SwapChain/SwapChain.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/CommandQueue/CommandQueue.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/PipelineState/PipelineState.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/RootSignature/RootSignature.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/DepthBuffer/DepthBuffer.hpp>

namespace D3D12Engine {
  class DirectX12Graphics : public IRenderer {
   public:
    DirectX12Graphics(UINT WindowWidth, UINT WindowHeight);
    ~DirectX12Graphics() override;

    void OnInitialize(HWND hwnd, UINT WindowWidth, UINT WindowHeight) override;
    void OnRender() override;
    void OnResize(UINT WindowWidth, UINT WindowHeight) override;
    void OnUpdate() override;
    void OnDestroy() override;
  
   private:
    // ↓ Window properties ↓ 
      UINT m_WindowWidth;
      UINT m_WindowHeight;
    // ↑ Window properties ↑

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

    // ↓ Model ↓
      std::unique_ptr<Model> m_Model;
      std::unique_ptr<Model> m_FloorModel;

      DirectX::XMMATRIX m_DisplacementMatrix;
      DirectX::XMMATRIX m_FloorMatrix;
    // ↑ Model ↑

    // ↓ Camera ↓
      std::unique_ptr<Camera> m_Camera;
    // ↑ Camera ↑

    void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory1, _Outptr_opt_result_maybenull_ IDXGIAdapter1** ppAdapter1, bool requestHighPerfomanceAdapter);
    void LoadPipeline();
    void LoadAssets();
  };
}
