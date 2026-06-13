#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <queue>
#include <stdexcept>

namespace D3D12Engine {
  struct DescriptorAllocation {
    D3D12_CPU_DESCRIPTOR_HANDLE CPU = {0};
    D3D12_GPU_DESCRIPTOR_HANDLE GPU = {0};
    uint32_t Index = ~0U;

    bool IsValid() const {
      return CPU.ptr != 0;
    }
  };

  class DescriptorAllocator {
   public:
    DescriptorAllocator(
      ID3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint32_t numDescriptors
    );
    ~DescriptorAllocator() = default;

    DescriptorAllocation Allocate();
    void Free(DescriptorAllocation& Allocation);
    
    ID3D12DescriptorHeap* GetHeap() const {
      return m_Heap.Get();
    }
   private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
    D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
    uint32_t m_SizeDescriptor;
    uint32_t m_NumDescriptors;
    uint32_t m_CrtDescriptor;

    std::queue<uint32_t> m_FreeIndicies;
  };
}