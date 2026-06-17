#pragma once

#include <vector>

#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/RootSignature/RootSignature.hpp>

namespace D3D12Engine {
  class PSO {
   public:
    PSO(const wchar_t* Name) : m_Name(Name), m_pipelineState(nullptr) {}
    ~PSO() = default;
    
    virtual void Shutdown() {
      if (m_pipelineState) {
        m_pipelineState.Reset();
      }
    }

    ID3D12PipelineState* GetPipelineState() const { return m_pipelineState.Get(); }

   protected:
    const wchar_t* m_Name;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
  };

  class GraphicsPSO : public PSO {
   public:
    GraphicsPSO(const wchar_t* Name = L"Default Graphics PSO");
    ~GraphicsPSO() = default;

    void SetRootSignature(
      const RootSignature& rootSignature
    );
    
    void SetVertexShader(
      const void* pByteCode, size_t size
    );
    
    void SetPixelShader(
      const void* pByteCode, size_t size
    );
    
    void SetInputLayout(
      UINT count, const D3D12_INPUT_ELEMENT_DESC* desc
    );
    
    void SetCullMode(
      D3D12_CULL_MODE cullMode
    );
    
    void SetDepthTest(
      bool enable
    );
    
    void SetDepthFunc(
      D3D12_COMPARISON_FUNC depthFunc
    );
    
    void SetStencilTest(
      bool                  isEnable,
      D3D12_COMPARISON_FUNC Func        = D3D12_COMPARISON_FUNC_ALWAYS,
      D3D12_STENCIL_OP      FailOp      = D3D12_STENCIL_OP_KEEP,
      D3D12_STENCIL_OP      DepthFailOp = D3D12_STENCIL_OP_KEEP,
      D3D12_STENCIL_OP      PassOp      = D3D12_STENCIL_OP_KEEP,
      UINT8                 ReadMask    = 0xFF,
      UINT8                 WriteMask   = 0xFF
    );
    
    void SetFrontCounterClockwise(
      bool isBufferCCW
    );
    
    void SetRenderTargetFormat(
      DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat
    );
    
    void Finalize(
      ID3D12Device* pDevice
    );

   private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayoutDesc;
  };
}