#include <Include/Graphics/Core/CommandContext.hpp>
#include <Include/Graphics/Resources/BackBuffer.hpp>
#include <Include/Graphics/Resources/DepthBuffer.hpp>

D3D12Engine::CommandContext::CommandContext(
  ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE cmdListType) : m_pDevice(pDevice)
{
  m_pDevice->CreateCommandAllocator(cmdListType, IID_PPV_ARGS(&m_cmdAllocator));

  m_pDevice->CreateCommandList(0, cmdListType, m_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&m_cmdList));

  m_cmdList->Close();
}

void D3D12Engine::CommandContext::TransitionResource(
  GpuResource& Resource, D3D12_RESOURCE_STATES newState)
{
  D3D12_RESOURCE_STATES oldState = Resource.GetUsageState();
  
  if (oldState != newState) {
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      Resource.GetResource(), oldState, newState
    );
    m_ResourceBarrierBuffer.push_back(barrier);
    
    Resource.SetUsageState(newState);
  }
}

void D3D12Engine::CommandContext::FlushResourceBarriers() {
  if (!m_ResourceBarrierBuffer.empty()) {
    m_cmdList->ResourceBarrier(static_cast<UINT>(m_ResourceBarrierBuffer.size()), m_ResourceBarrierBuffer.data());
  
    m_ResourceBarrierBuffer.clear();
  }
}

void D3D12Engine::CommandContext::ClearColor(BackBuffer& Target, const float* ClearColor) {
  TransitionResource(Target, D3D12_RESOURCE_STATE_RENDER_TARGET);
  FlushResourceBarriers();

  m_cmdList->ClearRenderTargetView(Target.GetRTV(), ClearColor, 0, nullptr);
}

void D3D12Engine::CommandContext::ClearDepth(DepthBuffer& Target) {
  TransitionResource(Target, D3D12_RESOURCE_STATE_DEPTH_WRITE);
  FlushResourceBarriers();

  m_cmdList->ClearDepthStencilView(
    Target.GetDSV(),
    D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
    Target.GetClearDepth(), Target.GetClearStencil(),
    0, nullptr
  );
}

void D3D12Engine::CommandContext::InitializeBuffer(
  GpuBuffer& destBuffer, const void* data, size_t numBytes)
{
  Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
  
  m_pDevice->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(numBytes),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&uploadBuffer)
  );

  UINT8* pVertexDataBegin;
  CD3DX12_RANGE readRange(0, 0);
  uploadBuffer->Map(
    0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)
  );
  memcpy(pVertexDataBegin, data, numBytes);
  uploadBuffer->Unmap(0, nullptr);

  // ↓ PipelineState & RootSignature ↓
  TransitionResource(destBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
  FlushResourceBarriers();
  // ↑ PipelineState & RootSignature ↑

  m_cmdList->CopyBufferRegion(destBuffer.GetResource(), 0, uploadBuffer.Get(), 0, numBytes);

  // ↓ PipelineState & RootSignature ↓
  TransitionResource(destBuffer, D3D12_RESOURCE_STATE_COMMON);
  FlushResourceBarriers();
  // ↑ PipelineState & RootSignature ↑

  m_TempUploadBuffers.push_back(uploadBuffer);
}