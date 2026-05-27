#pragma once

#include <d3d12.h>
#include <Renderer/D3D12Engine/Backend/RHI/Libraries/d3dx12.h>
#include <wrl/client.h>

namespace D3D12Engine {
  class RootParameter {
    friend class RootSignature;

   public:
    RootParameter() = default;
    ~RootParameter() = default;
    
    void InitAsConstantBuffer(
      UINT numRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
    );  
    
    void InitAsConstants(
      UINT shaderRegister, UINT num32BitValues, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
    );

   protected:
    CD3DX12_ROOT_PARAMETER m_RootParam;
  };

  class RootSignature {
   public:
    RootSignature() = default;
    ~RootSignature() = default;

    void Reset(UINT numParams, UINT numSamplers = 0);
    void Finalize(ID3D12Device* pDevice, D3D12_ROOT_SIGNATURE_FLAGS flags);

    ID3D12RootSignature* Get() const { return m_RootSignature.Get(); }

    RootParameter& operator[](size_t index) { return m_RootParams.get()[index]; }
    const RootParameter& operator[](size_t index) const { return m_RootParams.get()[index]; }

   protected:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
    
    std::unique_ptr<CD3DX12_STATIC_SAMPLER_DESC[]> m_SamplerDesc;
    UINT m_numSamplers = 0;

    std::unique_ptr<RootParameter[]> m_RootParams;
    UINT m_numRootParams = 0;
  };
}