#include <Include/Graphics/GraphicsContext.hpp>

D3D12Engine::GraphicsContext::GraphicsContext(ID3D12GraphicsCommandList* cmdList) : m_CommandList(cmdList) {}

void D3D12Engine::GraphicsContext::TransitionResource(GPUResource& resource, D3D12_RESOURCE_STATES newState) {
  D3D12_RESOURCE_STATES oldState = resource.GetUsageState();
  
  if (oldState != newState) {
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      resource.GetResource(), oldState, newState
    );
    m_ResourceBarrierBuffer.push_back(barrier);
    
    resource.SetUsageState(newState);
  }
}

void D3D12Engine::GraphicsContext::FlushResourceBarriers() {
  if (!m_ResourceBarrierBuffer.empty()) {
    m_CommandList->ResourceBarrier((UINT)m_ResourceBarrierBuffer.size(), m_ResourceBarrierBuffer.data());
  
    m_ResourceBarrierBuffer.clear();
  }
}

void D3D12Engine::GraphicsContext::ClearColor(GPUResource& target, const float* bgColor) {
  TransitionResource(target, D3D12_RESOURCE_STATE_RENDER_TARGET);
  
  FlushResourceBarriers();
}