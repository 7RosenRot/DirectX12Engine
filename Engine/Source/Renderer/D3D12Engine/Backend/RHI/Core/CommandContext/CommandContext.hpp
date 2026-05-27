#pragma once

#include <d3d12.h>
#include <Renderer/D3D12Engine/Backend/RHI\Libraries\d3dx12.h>
#include <wrl/client.h>
#include <vector>

#include <Renderer/D3D12Engine/Backend/RHI/Resources/GpuBuffer/GpuBuffer.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/GpuResource/GpuResource.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/BackBuffer/BackBuffer.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/DepthBuffer/DepthBuffer.hpp>

namespace D3D12Engine {
  class CommandContext {
   public:
    CommandContext(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE cmdListType);
    ~CommandContext() = default;

    void Reset();
    void Close();

    void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES newState);    
    void FlushResourceBarriers();
    
    void ClearColor(BackBuffer& Target, const float* ClearColor);
    void ClearDepth(DepthBuffer& Target);

    void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount,
      UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation);

    // ↓ Extended to process 3D models ↓
    void InitializeBuffer(GpuBuffer& destBuffer, const void* data, size_t numBytes);

    // ↓ Getters ↓
    ID3D12GraphicsCommandList* GetCommandList() const {
      return m_cmdList.Get();
    }
    // ↑ Getters ↑
    
    // ↓ Setters ↓
    void SetPipelineState(ID3D12PipelineState* pipelineStateObject);
    void SetGraphicsRootSignature(ID3D12RootSignature* rootSignature);

    void SetRenderTargets(BackBuffer& rtv);
    void SetRenderTargets(BackBuffer& rtv, DepthBuffer& dsv);
    
    void SetViewports(UINT numViewports, const D3D12_VIEWPORT* pViewport);
    void SetScissorRects(UINT numRectangles, const D3D12_RECT* pRectangles);
    
    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);
    
    void SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView);
    void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& indexBufferView);
    void SetConstantBuffer(UINT rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS constantBufferView);
    // ↑ Setters ↑

   private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_pDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_cmdList;
    
    std::vector<D3D12_RESOURCE_BARRIER> m_ResourceBarrierBuffer;
    
    // ↓ Extended to process 3D models ↓
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_TempUploadBuffers;
  };

  inline void CommandContext::Reset() {
    m_cmdAllocator->Reset();

    m_cmdList->Reset(m_cmdAllocator.Get(), nullptr);

    m_ResourceBarrierBuffer.clear();
  }

  inline void CommandContext::Close() {
    m_cmdList->Close();
  }

  inline void CommandContext::DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount,
    UINT startIndexLocation,INT baseVertexLocation, UINT startInstanceLocation)
  {
    m_cmdList->DrawIndexedInstanced(
      indexCountPerInstance,
      instanceCount,
      startIndexLocation,
      baseVertexLocation,
      startInstanceLocation
    );
  }

  inline void CommandContext::SetPipelineState(ID3D12PipelineState* pipelineStateObject) {
    m_cmdList->SetPipelineState(pipelineStateObject);
  }
  
  inline void CommandContext::SetGraphicsRootSignature(ID3D12RootSignature* rootSignature) {
    m_cmdList->SetGraphicsRootSignature(rootSignature);
  }

  inline void CommandContext::SetRenderTargets(BackBuffer& rtv) {
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtv.GetRTV();

    m_cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
  }

  inline void CommandContext::SetRenderTargets(BackBuffer& rtv, DepthBuffer& dsv) {
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtv.GetRTV();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsv.GetDSV();

    m_cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
  }
  
  inline void CommandContext::SetViewports(UINT numViewports, const D3D12_VIEWPORT* pViewport) {
    m_cmdList->RSSetViewports(numViewports, pViewport);
  }
  
  inline void CommandContext::SetScissorRects(UINT numRectangles, const D3D12_RECT* pRectangles) {
    m_cmdList->RSSetScissorRects(numRectangles, pRectangles); 
  }
  
  inline void CommandContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology) {
    m_cmdList->IASetPrimitiveTopology(primitiveTopology);
  }
  
  inline void CommandContext::SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView) {
    m_cmdList->IASetVertexBuffers(Slot, 1, &vertexBufferView);
  }
  
  inline void CommandContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& indexBufferView) {
    m_cmdList->IASetIndexBuffer(&indexBufferView);
  }
  
  inline void CommandContext::SetConstantBuffer(UINT rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS constantBufferView) {
    m_cmdList->SetGraphicsRootConstantBufferView(rootParameterIndex, constantBufferView);
  }
}