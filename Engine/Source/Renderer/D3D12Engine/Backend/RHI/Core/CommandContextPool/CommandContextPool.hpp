#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <limits>
#include <d3d12.h>

#include <Renderer/D3D12Engine/Backend/RHI/Core/CommandContext/CommandContext.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/SwapChain/SwapChain.hpp>

namespace D3D12Engine {
  class CommandContextPool {
   public:
    CommandContextPool(
      ID3D12Device* pDevice,
      const SwapChain* pSwapChain,
      D3D12_COMMAND_LIST_TYPE CmdListType
    );
    ~CommandContextPool() = default;

    CommandContext* Allocate(UINT64 completedFenceValue = (std::numeric_limits<UINT64>::max)());
    void Free(CommandContext* pContext);
   
   private:
    ID3D12Device* m_pDevice;
    const SwapChain* m_pSwapChain;
    D3D12_COMMAND_LIST_TYPE m_CmdListType;

    std::mutex m_Mutex;
    std::vector<std::unique_ptr<CommandContext>> m_CmdContextPool;
    std::queue<CommandContext*> m_AvailebleContexts;
  };
}