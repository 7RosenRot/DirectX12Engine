#include <Renderer/D3D12Engine/Backend/RHI/Core/CommandContextPool/CommandContextPool.hpp>
using namespace D3D12Engine;

CommandContextPool::CommandContextPool(
  ID3D12Device* pDevice,
  const SwapChain* pSwapChain,
  D3D12_COMMAND_LIST_TYPE CmdListType
) :
  m_pDevice(pDevice),
  m_pSwapChain(pSwapChain),
  m_CmdListType(CmdListType)
{}

CommandContext* CommandContextPool::Allocate(UINT64 completedFenceValue) {
  std::lock_guard<std::mutex> lock(m_Mutex);

  CommandContext* pContext = nullptr;

  if (!m_AvailebleContexts.empty() && m_AvailebleContexts.front()->GetFenceValue() <= completedFenceValue) {
    pContext = m_AvailebleContexts.front();
    
    m_AvailebleContexts.pop();
  }

  else {
    auto pNewContext = std::make_unique<CommandContext>(
      m_pDevice, m_CmdListType
    );

    m_CmdContextPool.push_back(std::move(pNewContext));

    pContext = m_CmdContextPool.back().get();
  }

  pContext->Reset();

  return pContext;
}

void CommandContextPool::Free(CommandContext* pContext) {
  std::lock_guard<std::mutex> lock(m_Mutex);

  m_AvailebleContexts.push(pContext);
}