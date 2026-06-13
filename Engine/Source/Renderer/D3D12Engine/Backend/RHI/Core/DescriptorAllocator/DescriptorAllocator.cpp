#include <Renderer/D3D12Engine/Backend/RHI/Core/DescriptorAllocator/DescriptorAllocator.hpp>

D3D12Engine::DescriptorAllocator::DescriptorAllocator(
  ID3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint32_t NumDescriptors
) : m_HeapType(HeapType), m_NumDescriptors(NumDescriptors), m_CrtDescriptor(0)
{
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.Type = m_HeapType;
  heapDesc.NumDescriptors = m_NumDescriptors;
  heapDesc.Flags = (m_HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ?
    D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  heapDesc.NodeMask = 0;

  pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap));

  m_SizeDescriptor = pDevice->GetDescriptorHandleIncrementSize(m_HeapType);
}

D3D12Engine::DescriptorAllocation D3D12Engine::DescriptorAllocator::Allocate() {
  uint32_t Index;

  if (!m_FreeIndicies.empty()) {
    Index = m_FreeIndicies.front();
    m_FreeIndicies.pop();
  } else {
    if (m_CrtDescriptor >= m_NumDescriptors) {
      throw std::runtime_error("Descriptor Heap is full");
    }
    Index = m_CrtDescriptor++;
  }

  DescriptorAllocation Allocation;
  Allocation.Index = Index;

  Allocation.CPU = m_Heap->GetCPUDescriptorHandleForHeapStart();
  Allocation.CPU.ptr += static_cast<SIZE_T>(Index) * m_SizeDescriptor;

  if (m_HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) {
    Allocation.GPU = m_Heap->GetGPUDescriptorHandleForHeapStart();
    Allocation.GPU.ptr += static_cast<UINT64>(Index) * m_SizeDescriptor;
  }

  return Allocation;
}

void D3D12Engine::DescriptorAllocator::Free(DescriptorAllocation& Allocation) {
  if (!Allocation.IsValid()) {
    return;
  }

  m_FreeIndicies.push(Allocation.Index);

  Allocation.CPU.ptr = 0;
  Allocation.GPU.ptr = 0;
  Allocation.Index = ~0U;
}