#include <Include/Graphics/Pipeline/PipelineState.hpp>

D3D12Engine::GraphicsPSO::GraphicsPSO(const wchar_t* Name) : PSO(Name) {
  ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
  
  m_psoDesc.NodeMask = 1;
  m_psoDesc.SampleMask = UINT_MAX;
  m_psoDesc.SampleDesc.Count = 1;
  m_psoDesc.InputLayout.NumElements = 0;
  
  m_psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  
  m_psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  m_psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  m_psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
}

void D3D12Engine::GraphicsPSO::SetRootSignature(const RootSignature& rootSignature) {
  m_psoDesc.pRootSignature = rootSignature.Get();
}

void D3D12Engine::GraphicsPSO::SetVertexShader(const void* pByteCode, size_t size) {
  m_psoDesc.VS = CD3DX12_SHADER_BYTECODE(pByteCode, size);
}

void D3D12Engine::GraphicsPSO::SetPixelShader(const void* pByteCode, size_t size) {
  m_psoDesc.PS = CD3DX12_SHADER_BYTECODE(pByteCode, size);
}

void D3D12Engine::GraphicsPSO::SetInputLayout(UINT count, const D3D12_INPUT_ELEMENT_DESC* desc) {
  m_inputLayoutDesc.assign(desc, desc + count);
  
  m_psoDesc.InputLayout.pInputElementDescs = m_inputLayoutDesc.data();
  m_psoDesc.InputLayout.NumElements = count;
}

void D3D12Engine::GraphicsPSO::SetCullMode(D3D12_CULL_MODE cullMode) {
  m_psoDesc.RasterizerState.CullMode = cullMode;
}

void D3D12Engine::GraphicsPSO::SetFrontCounterClockwise(bool isBufferCCW) {
  m_psoDesc.RasterizerState.FrontCounterClockwise = isBufferCCW;
}

void D3D12Engine::GraphicsPSO::SetDepthTest(bool enable) {
  m_psoDesc.DepthStencilState.DepthEnable = enable;
  m_psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
  m_psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
}

void D3D12Engine::GraphicsPSO::SetRenderTargetFormat(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat) {
  m_psoDesc.NumRenderTargets = 1;
  m_psoDesc.RTVFormats[0] = rtvFormat;
  m_psoDesc.DSVFormat = dsvFormat;
}

void D3D12Engine::GraphicsPSO::Finalize(ID3D12Device* pDevice) {
  m_psoDesc.InputLayout.pInputElementDescs = m_inputLayoutDesc.data();
  m_psoDesc.InputLayout.NumElements = static_cast<UINT>(m_inputLayoutDesc.size());
  
  pDevice->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pipelineState));
  m_pipelineState->SetName(m_Name);
}