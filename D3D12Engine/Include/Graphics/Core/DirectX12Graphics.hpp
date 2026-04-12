#pragma once

#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <Include/Window/Window.hpp>

// DirectX include path
#include <Include/Graphics/Core/InterfaceDirectX12.hpp>
#include <Include/Graphics/Core/CommandContext.hpp>
#include <Include/Graphics/Pipeline/CommandQueue.hpp>
#include <Include/Graphics/Pipeline/Display.hpp>
#include <Include/Graphics/Pipeline/RootSignature.hpp>
#include <Include/Graphics/Pipeline/PipelineState.hpp>
#include <Include/Graphics/Resources/DepthBuffer.hpp>
#include <Include/Graphics/Scene/Model.hpp>

namespace D3D12Engine {
  class DirectX12Graphics : public InterfaceDirectX12 {
   public:
    DirectX12Graphics(
      UINT WindowWidth, UINT WindowHeight, UINT AspectWidth, UINT AspectHeight, std::wstring WindowName
    );
    virtual ~DirectX12Graphics();

    void OnInitialize() override;
    void OnRender() override;
    void OnResize(UINT WindowWidth, UINT WindowHeight) override;
    void OnUpdate() override;
    void OnDestroy() override;
  
   private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;

    std::unique_ptr<CommandQueue> m_cmdQueue;
    std::unique_ptr<CommandContext> m_cmdContext;
    std::unique_ptr<Display> m_display;
    
    DepthBuffer m_depthBuffer;
    
    RootSignature m_rootSignature;
    GraphicsPSO m_pipelineState{L"Main PipelineStateObject"};

    CD3DX12_VIEWPORT m_viewPort{};
    CD3DX12_RECT m_scissorRect{};

    std::unique_ptr<Model> m_Model;
    DirectX::XMMATRIX m_DisplacementMatrix;

    void LoadPipeline();
    void LoadAssets();
  };
}
