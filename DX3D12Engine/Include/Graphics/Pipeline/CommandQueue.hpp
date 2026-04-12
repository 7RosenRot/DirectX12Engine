#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace D3D12Engine {
  class CommandQueue {
   public:
    CommandQueue(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE cmdListType);
    ~CommandQueue();

    UINT64 ExecuteCommandList(ID3D12CommandList* pCmdList);
    UINT64 Signal();
    void WaitForPreviousFrame(UINT64 fenceValue);
    void Flush();

    ID3D12CommandQueue* GetResource() const;
    
   private:
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue;
    
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent;
    UINT64 m_fenceValue;
  };

  inline ID3D12CommandQueue* CommandQueue::GetResource() const {
    return m_cmdQueue.Get();
  }
}