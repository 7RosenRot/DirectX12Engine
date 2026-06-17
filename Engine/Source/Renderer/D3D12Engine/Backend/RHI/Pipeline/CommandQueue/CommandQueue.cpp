#include <exception>
#include <stdexcept>

#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/CommandQueue/CommandQueue.hpp>

D3D12Engine::CommandQueue::CommandQueue(
  ID3D12Device* pDevice,
  D3D12_COMMAND_LIST_TYPE cmdListType
) :
  m_fenceValue(1) 
{
  D3D12_COMMAND_QUEUE_DESC cmdQueueDescriptor{};
  cmdQueueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  cmdQueueDescriptor.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  cmdQueueDescriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  pDevice->CreateCommandQueue(&cmdQueueDescriptor, IID_PPV_ARGS(&m_cmdQueue));

  pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
  m_fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
  
  if (m_fenceEvent == nullptr) {
    throw std::runtime_error("Failed to create fence event!");
  }
}

D3D12Engine::CommandQueue::~CommandQueue() {
  Flush();

  CloseHandle(m_fenceEvent);
}

UINT64 D3D12Engine::CommandQueue::ExecuteCommandList(ID3D12CommandList* pCmdList) {
  ID3D12CommandList* pCmdLists[] = { pCmdList };
  m_cmdQueue->ExecuteCommandLists(_countof(pCmdLists), pCmdLists);
  
  return Signal();
}

UINT64 D3D12Engine::CommandQueue::Signal() {
  UINT64 fenceValue = m_fenceValue;
  m_cmdQueue->Signal(m_fence.Get(), fenceValue);
  m_fenceValue += 1;

  return fenceValue;
}

void D3D12Engine::CommandQueue::WaitForPreviousFrame(UINT64 fenceValue) {
  if (m_fence->GetCompletedValue() < fenceValue) {
    m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
    
    WaitForSingleObject(m_fenceEvent, INFINITE);
  }
}

void D3D12Engine::CommandQueue::Flush() {
  UINT64 fenceValue = Signal();

  WaitForPreviousFrame(fenceValue);
}