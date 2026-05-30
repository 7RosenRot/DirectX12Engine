#pragma once

#include <vector>

#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/RootSignature/RootSignature.hpp>

namespace D3D12Engine {
  class PSO {
   public:
    PSO(const wchar_t* Name) : m_Name(Name), m_pipelineState(nullptr) {}
    ~PSO() = default;
    
    ID3D12PipelineState* GetPipelineState() const { return m_pipelineState.Get(); }

   protected:
    const wchar_t* m_Name;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
  };

  class GraphicsPSO : public PSO {
   public:
    GraphicsPSO(const wchar_t* Name = L"Default Graphics PSO");
    ~GraphicsPSO() = default;

    void SetRootSignature(const RootSignature& rootSignature);
    void SetVertexShader(const void* pByteCode, size_t size);
    void SetPixelShader(const void* pByteCode, size_t size);
    void SetInputLayout(UINT count, const D3D12_INPUT_ELEMENT_DESC* desc);
    
    void SetCullMode(D3D12_CULL_MODE cullMode);
    void SetDepthTest(bool enable);
    void SetFrontCounterClockwise(bool isBufferCCW);
    
    void SetRenderTargetFormat(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat);
    
    void Finalize(ID3D12Device* pDevice);

   private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayoutDesc; // to rework
  };
}