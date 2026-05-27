#include <Renderer/D3D12Engine/Backend/RHI/Resources/BackBuffer/BackBuffer.hpp>

namespace D3D12Engine {
  void BackBuffer::CreateFromSwapChain(ID3D12Device* device, const std::wstring& name,
    ID3D12Resource* baseResource, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
  {    
    m_pResource.Attach(baseResource);
    m_rtvHandle = rtvHandle;

    device->CreateRenderTargetView(m_pResource.Get(), nullptr, m_rtvHandle);

    m_pResource->SetName(name.c_str());
    m_UsageState = D3D12_RESOURCE_STATE_PRESENT;
  }
}