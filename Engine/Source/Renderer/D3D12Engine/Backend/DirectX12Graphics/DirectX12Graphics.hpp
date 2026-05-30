#pragma once

#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <unordered_map>

// DirectX include path
#include <Renderer/D3D12Engine/Backend/RHI/Libraries/d3dx12.h>

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

namespace D3D12Engine {
  class DirectX12Graphics : public IRenderer {
   public:
    DirectX12Graphics(HWND hwnd, UINT WindowWidth, UINT WindowHeight);
    ~DirectX12Graphics() override;

    void OnInitialize() override;
    void OnRender() override;
    void OnResize(UINT WindowWidth, UINT WindowHeight) override;
    void OnUpdate() override;
    void OnDestroy() override;
  
   private:
    // ↓ Window properties ↓
      HWND m_hwnd;
      unsigned int m_WindowWidth;
      unsigned int m_WindowHeight;
    // ↑ Window properties ↑

    bool m_useWarpAdapter = false;

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

    // ↓ GameObject ↓
      std::unordered_map<std::string, std::unique_ptr<GameObject>> m_GameObjects;
    // ↑ GameObject ↑

    // ↓ Camera ↓
      std::unique_ptr<Camera> m_Camera;
    // ↑ Camera ↑

    void GetHardwareAdapter(
      _In_ IDXGIFactory1* pFactory1,
      _Outptr_opt_result_maybenull_ IDXGIAdapter1** ppAdapter1,
      bool requestHighPerformanceAdapter
    );
    
    void LoadPipeline();
    void LoadAssets();
  };
}
