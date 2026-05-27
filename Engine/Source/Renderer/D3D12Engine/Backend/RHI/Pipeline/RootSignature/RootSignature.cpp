#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/RootSignature/RootSignature.hpp>

void D3D12Engine::RootSignature::Reset(UINT numRootParams, UINT numSamplers) {
  m_RootParams = std::make_unique<RootParameter[]>(numRootParams);
  m_numRootParams = numRootParams;

  m_SamplerDesc = std::make_unique<CD3DX12_STATIC_SAMPLER_DESC[]>(numSamplers);
  m_numSamplers = numSamplers;
}

void D3D12Engine::RootParameter::InitAsConstantBuffer(UINT numRegister, D3D12_SHADER_VISIBILITY visibility) {
  m_RootParam.InitAsConstantBufferView(numRegister, 0, visibility);
}

void D3D12Engine::RootParameter::InitAsConstants(UINT shaderRegister, UINT num32BitValues, D3D12_SHADER_VISIBILITY visibility) {
  m_RootParam.InitAsConstants(num32BitValues, shaderRegister, 0, visibility);
}

void D3D12Engine::RootSignature::Finalize(ID3D12Device* pDevice, D3D12_ROOT_SIGNATURE_FLAGS flags) {
  D3D12_ROOT_PARAMETER* pRootParam = nullptr;
  if (m_numRootParams) {
    pRootParam = reinterpret_cast<D3D12_ROOT_PARAMETER*>(
      _malloca(sizeof(D3D12_ROOT_PARAMETER) * m_numRootParams)
    );

    for (UINT i = 0; i < m_numRootParams; i += 1) {
      pRootParam[i] = m_RootParams[i].m_RootParam;
    }
  }
  
  CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDescriptor{};
  rootSignatureDescriptor.Init(
    m_numRootParams, pRootParam,
    m_numSamplers, m_SamplerDesc.get(),
    flags
  );

  Microsoft::WRL::ComPtr<ID3DBlob> pSignature;
  Microsoft::WRL::ComPtr<ID3DBlob> pError;

  HRESULT hr = D3D12SerializeRootSignature(
    &rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError
  );
  
  pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

  if (pRootParam) _freea(pRootParam); 
}